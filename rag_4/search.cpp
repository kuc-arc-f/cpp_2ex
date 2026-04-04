
#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>
#include <map>
#include <stdexcept>
#include <vector>
#include <curl/curl.h>
#include <nlohmann/json.hpp> // JSONライブラリ
#include <random>

#include "qdrant_client.hpp"
#include "http_client.hpp"

const std::string COLLECTION = "document-4";
const size_t      VECTOR_DIM = 1024;  // ベクトル次元

using namespace std;

using json = nlohmann::json;

const std::string API_URL_EMBED = "http://localhost:8080/embedding";
const std::string API_URL_CHAT = "http://localhost:8090/v1/chat/completions";

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
// ─────────────────────────────────────────────
// main : 動作確認サンプル
// ─────────────────────────────────────────────
int main(int argc, char* argv[])
{
    //std::cout << "arg_count=" << argc << "\n";
    //std::cout << "argv[0]=" << argv[0] << "\n";
    if(argc < 2) {
        std::cerr << "[ERROR] argment none" << "\n";
        return 0;
    }
    int arg_count = argc;
    std::string query = argv[1];
    std::cout << "query=" << query << "\n";

    HttpClient client(30 /*timeout*/, true /*verify_ssl*/);
    std::string api_url = "";
    struct QueryReq req_data;
    req_data.input = query;

    json j_req = req_data;

    std::string json_str = j_req.dump();
    //std::cout << "json_str : " << json_str << "\n";
    auto resp = client.post_json(API_URL_EMBED, json_str);
    if (!resp.error.empty()) {
        std::cerr << "[ERROR] " << resp.error << "\n";
        return 0;
    }
    std::cout << "Status : " << resp.status_code << "\n";
    // =========================================================
    // 1. クライアント初期化
    // =========================================================
    // ローカルの Qdrant に接続 (Docker: docker run -p 6333:6333 qdrant/qdrant)
    QdrantClient qdrant_client("localhost", 6333);
    std::cout << "=== Qdrant C++ クライアント===\n\n";

    try{
      if (resp.is_ok()) {
        std::string str = resp.body;
        json j = json::parse(str);
        auto embedding = j[0]["embedding"];
        auto vec = embedding[0];
        int vlength = sizeof(vec) / sizeof(vec[0]);
        std::cout << "vlen=" << vec.size() << std::endl;

        double v1 = vec[0];
        double v2 = vec[1]; 
        // 件数確認
        auto after_delete = qdrant_client.search(COLLECTION, vec, 10000);
        std::cout << "登録件数: " << after_delete.size() << " 件\n\n";               

        // =========================================================
        // 4. ベクトル検索 (類似検索)
        // =========================================================
        std::cout << "--- ベクトル類似検索 (Top N rec) ---\n";
        std::cout << "クエリベクトル: ";
        std::cout << "\n\n";        
        auto results = qdrant_client.search(COLLECTION, vec, 1);
        std::string matches = "";
        for (const auto& r : results) {
            std::cout << "  score: " << std::fixed << std::setprecision(4) << r.score
                      << "\n";
            std::cout << "r.score=" << r.score << "\n";
            std::string content = r.payload["content"].get<std::string>();
            if(r.score > 0.4) {
                matches = content;
            }
        }
        std::cout << "\n";

        std::string out_str = "要約して欲しい。\n";
        std::string resp_str = matches;
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

        //std::cout << "requestBody:" << requestBody  << std::endl;
        auto resp2 = client.post_json(API_URL_CHAT, requestBody);
        if (!resp2.error.empty()) {
            std::cerr << "[ERROR] " << resp2.error << "\n";
            return 0;
        }
        std::string reply = extractContent(resp2.body);
        std::cout << "Assistant: " << reply << "\n\n"; 
      }
    } catch (const std::exception& e) {
        std::cout << "Error , main" << std::endl;
        return 0;
    }        
    return 0;
}


