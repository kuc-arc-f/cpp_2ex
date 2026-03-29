#include "httplib.h"
#include <iostream>
#include <nlohmann/json.hpp> // JSONライブラリ
#include <vector>
#include <string>
#include <mutex>
#include <sstream>

#include "pg_search.hpp"
#include "http_client.hpp"

using json = nlohmann::json;

const std::string API_URL_EMBED = "http://localhost:8080/embedding";
const std::string API_URL_CHAT = "http://localhost:8090/v1/chat/completions";

// ─────────────────────────────────────────
// データ構造
// ─────────────────────────────────────────
struct Todo {
    int         id;
    std::string title;
    bool        done;
};

// インメモリストレージ
static std::vector<Todo> g_todos;
static int               g_next_id = 1;
static std::mutex        g_mutex;

// ─────────────────────────────────────────
// ヘルパー：Todo → JSON 文字列
// ─────────────────────────────────────────
std::string todo_to_json(const Todo& t) {
    std::ostringstream oss;
    oss << "{"
        << "\"id\":"    << t.id           << ","
        << "\"title\":\"" << t.title      << "\","
        << "\"done\":"  << (t.done ? "true" : "false")
        << "}";
    return oss.str();
}

std::string todos_to_json(const std::vector<Todo>& todos) {
    std::ostringstream oss;
    oss << "[";
    for (size_t i = 0; i < todos.size(); ++i) {
        if (i > 0) oss << ",";
        oss << todo_to_json(todos[i]);
    }
    oss << "]";
    return oss.str();
}

// ─────────────────────────────────────────
// ヘルパー：JSON から値を取り出す（簡易版）
// ─────────────────────────────────────────
std::string extract_string(const std::string& json, const std::string& key) {
    // "key":"value" を探す
    std::string pattern = "\"" + key + "\":\"";
    auto pos = json.find(pattern);
    if (pos == std::string::npos) return "";
    pos += pattern.size();
    auto end = json.find('"', pos);
    if (end == std::string::npos) return "";
    return json.substr(pos, end - pos);
}

bool extract_bool(const std::string& json, const std::string& key, bool def = false) {
    std::string pattern = "\"" + key + "\":";
    auto pos = json.find(pattern);
    if (pos == std::string::npos) return def;
    pos += pattern.size();
    return json.substr(pos, 4) == "true";
}

// ─────────────────────────────────────────
// CORS ヘッダー（ブラウザからのアクセス用）
// ─────────────────────────────────────────
void set_cors(httplib::Response& res) {
    res.set_header("Access-Control-Allow-Origin",  "*");
    res.set_header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
    res.set_header("Access-Control-Allow-Headers", "Content-Type");
}
struct SearchResponse {
    std::string ret;
    std::string text;
};   
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(SearchResponse, ret, text)

struct QueryReq {
    std::string input;
};   
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(QueryReq, input)
struct ChatQuery {
    std::string role;
    std::string content;
};
// これ一行で、QueryReq <=> json の変換が魔法のように可能になります
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ChatQuery, role, content)
struct ChatRequest {
    std::string model;
    std::vector<ChatQuery> messages;
    double temperature;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ChatRequest, model, messages, temperature)

std::string extractContent(const std::string& jsonStr)
{
    try {
        auto j = nlohmann::json::parse(jsonStr);
        return j["choices"][0]["message"]["content"].get<std::string>();
    } catch (const std::exception& e) {
        std::cerr << "[ERROR] JSON parse: " << e.what() << "\n";
        return "";
    }
}

// ─────────────────────────────────────────
// main
// ─────────────────────────────────────────
int main() {
    httplib::Server svr;

    // ── OPTIONS（プリフライト） ──────────────
    svr.Options(".*", [](const httplib::Request&, httplib::Response& res) {
        set_cors(res);
        res.status = 204;
    });

    /**
    *
    * @param
    *
    * @return
    */    
    svr.Post("/search", [](const httplib::Request& req, httplib::Response& res) {
        std::lock_guard<std::mutex> lk(g_mutex);
        // 1. Content-Typeの確認
        if (req.get_header_value("Content-Type") != "application/json") {
            res.status = 400;
            res.set_content("Expected application/json", "text/plain");
            return;
        }        
        try{
            // 2. JSONデコード (req.body をパース)
            json j = json::parse(req.body);

            // 3. データの取り出し (例: {"name": "Gopher", "id": 123})
            std::string input = j.at("input").get<std::string>();
            std::cout << "input=" << input << "\n";

            HttpClient client(30 /*timeout*/, true /*verify_ssl*/);
            struct QueryReq req_data;
            req_data.input = input;
            json j_req = req_data;
            //std::cout << j_req.dump() << std::endl;
            std::string json_str = j_req.dump();
            std::cout << "json_str : " << json_str << "\n";  
            auto resp = client.post_json("http://localhost:8080/embedding", json_str);
            if (!resp.error.empty()) {
                std::cerr << "[ERROR] " << resp.error << "\n";
                return;
            }
            std::cout << "Status : " << resp.status_code << "\n";
            std:string query = input;
            if (resp.is_ok()) {
                std::string str = resp.body;
                json j = json::parse(str);
                auto embedding = j[0]["embedding"];
                auto vec = embedding[0];
                int vlength = sizeof(vec) / sizeof(vec[0]);
                std::cout << "vec.len=" << vec.size() << std::endl;

                double v1 = vec[0];
                double v2 = vec[1];        
                std::cout << v1 << ", " << v2 << std::endl;

                PgSearch app("");
                std::string out_str = "日本語で、回答して欲しい。 \n要約して欲しい。\n\n";
                std::string resp_str = app.pg_search(vec);
                if(resp_str.empty()){
                    out_str.append("user query: ");
                    out_str.append(query);
                    out_str.append(" \n");
                }else{
                    out_str.append("context:");
                    out_str.append(resp_str);
                    out_str.append("\n user query: ");
                    out_str.append(query);
                    out_str.append(" \n");
                }
                std::cout << out_str  << std::endl;

                // send-qwe3.5
                ChatQuery req2;
                req2.role = "user";
                req2.content = out_str;
                json j2 = req2;
                std::string json_str2 = j2.dump();
                //std::cout << "json_str2:" << json_str2 << std::endl;
                std::vector<ChatQuery> chat_messages;
                chat_messages.push_back(req2);

                std::string target_msg = "[";
                target_msg.append(json_str2);
                target_msg.append("]");
                ChatRequest req3;
                req3.model = "local-model";
                req3.messages = chat_messages;
                req3.temperature = 0.7;
                json j3 = req3; // 構造体を代入するだけ！
                std::string json_str3 = j3.dump();
                //std::cout << "json_str3:" << json_str3 << std::endl;
                std::string requestBody = json_str3;
                std::cout << "requestBody:" << requestBody  << std::endl;
                auto resp2 = client.post_json(API_URL_CHAT, requestBody);
                if (!resp2.error.empty()) {
                    std::cerr << "[ERROR] " << resp2.error << "\n";
                    return;
                }
                std::string reply = extractContent(resp2.body);
                std::cout << "Assistant: " << reply << "\n\n"; 
                SearchResponse resp_d;
                resp_d.ret = "OK";
                resp_d.text = reply;
                json j4 = resp_d;
                std::string json_str4 = j4.dump();

                res.status = 201;
                res.set_content(json_str4, "application/json");
            }            
       } catch (const std::exception& e) {
            // キーが存在しない場合など
            res.status = 500;
            res.set_content("Internal Server Error", "text/plain");
        }        
    });

    // ── 起動 ────────────────────────────────
    int port_no = 8000;
    std::cout << "TODO Server running on http://localhost:8000\n";
    std::cout << "Endpoints:\n"
              << "  GET    /todos\n"
              << "  POST   /todos        body: {\"title\":\"...\"}\n"
              << "  PUT    /todos/:id    body: {\"title\":\"...\",\"done\":true}\n"
              << "  DELETE /todos/:id\n";

    svr.listen("0.0.0.0", port_no);
    return 0;
}
