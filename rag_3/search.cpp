
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

const std::string COLLECTION = "document-4";
const size_t      VECTOR_DIM = 1024;  // ベクトル次元

using namespace std;

using json = nlohmann::json;

// ─────────────────────────────────────────────
// レスポンス構造体
// ─────────────────────────────────────────────
struct HttpResponse {
    long        status_code = 0;
    std::string body;
    std::string error;

    bool is_ok() const { return status_code >= 200 && status_code < 300; }
};

// ─────────────────────────────────────────────
// libcurl 書き込みコールバック
// ─────────────────────────────────────────────
static size_t write_callback(char* ptr, size_t size, size_t nmemb, void* userdata)
{
    size_t total = size * nmemb;
    std::string* body = static_cast<std::string*>(userdata);
    body->append(ptr, total);
    return total;
}

// ─────────────────────────────────────────────
// HttpClient クラス
// ─────────────────────────────────────────────
class HttpClient {
public:
    using Headers = std::map<std::string, std::string>;

    // タイムアウト(秒)・SSL検証はコンストラクタで設定可能
    explicit HttpClient(long timeout_sec = 30, bool verify_ssl = true)
        : timeout_sec_(timeout_sec), verify_ssl_(verify_ssl)
    {
        curl_global_init(CURL_GLOBAL_DEFAULT);
    }

    ~HttpClient()
    {
        curl_global_cleanup();
    }

    // ── GET ──────────────────────────────────
    HttpResponse get(const std::string& url,
                     const Headers& headers = {}) const
    {
        return perform(url, "GET", "", headers);
    }

    // ── POST ─────────────────────────────────
    HttpResponse post(const std::string& url,
                      const std::string& body,
                      const Headers& headers = {}) const
    {
        return perform(url, "POST", body, headers);
    }

    // ── POST (JSON 簡易ラッパー) ──────────────
    HttpResponse post_json(const std::string& url,
                           const std::string& json_body,
                           Headers headers = {}) const
    {
        headers["Content-Type"] = "application/json";
        return post(url, json_body, headers);
    }

private:
    long timeout_sec_;
    bool verify_ssl_;

    HttpResponse perform(const std::string& url,
                         const std::string& method,
                         const std::string& body,
                         const Headers& headers) const
    {
        HttpResponse response;
        CURL* curl = curl_easy_init();
        if (!curl) {
            response.error = "curl_easy_init() failed";
            return response;
        }

        // ── ヘッダー設定 ──────────────────────
        struct curl_slist* header_list = nullptr;
        for (const auto& [key, val] : headers) {
            std::string header_str = key + ": " + val;
            header_list = curl_slist_append(header_list, header_str.c_str());
        }

        // ── 基本オプション ────────────────────
        curl_easy_setopt(curl, CURLOPT_URL,            url.c_str());
        curl_easy_setopt(curl, CURLOPT_TIMEOUT,        timeout_sec_);
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10L);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);   // リダイレクト追従
        curl_easy_setopt(curl, CURLOPT_MAXREDIRS,      5L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, verify_ssl_ ? 1L : 0L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, verify_ssl_ ? 2L : 0L);
        curl_easy_setopt(curl, CURLOPT_USERAGENT,      "HttpClient/1.0");

        // ── レスポンスボディ受信 ───────────────
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA,     &response.body);

        // ── メソッド別設定 ────────────────────
        if (method == "POST") {
            curl_easy_setopt(curl, CURLOPT_POST,           1L);
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS,     body.c_str());
            curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE,
                             static_cast<long>(body.size()));
        } else {
            // GET (デフォルト)
            curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
        }

        if (header_list) {
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header_list);
        }

        // ── 実行 ──────────────────────────────
        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            response.error = curl_easy_strerror(res);
        } else {
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response.status_code);
        }

        // ── クリーンアップ ────────────────────
        curl_slist_free_all(header_list);
        curl_easy_cleanup(curl);

        return response;
    }
};

// ─────────────────────────────────────────────
// ユーティリティ：レスポンス表示
// ─────────────────────────────────────────────
static void print_response(const std::string& label, const HttpResponse& resp)
{
    std::cout << "\n===== " << label << " =====\n";
    if (!resp.error.empty()) {
        std::cerr << "[ERROR] " << resp.error << "\n";
        return;
    }
    std::cout << "Status : " << resp.status_code << "\n";
    std::cout << "Body   :\n" << resp.body << "\n";
}

struct QueryReq {
    std::string input;
};   
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(QueryReq, input)

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
    //std::cout << j_req.dump() << std::endl;

    std::string json_str = j_req.dump();
    //std::cout << "json_str : " << json_str << "\n";
    auto resp = client.post_json("http://localhost:8080/embedding", json_str);
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
            //std::cout << "  ID: " << r.id
            std::cout << "  score: " << std::fixed << std::setprecision(4) << r.score
                      << "\n";
            //<< "  content: " << r.payload["content"].get<std::string>()
            std::string content = r.payload["content"].get<std::string>();
            matches = content;
        }
        std::cout << "\n";
        std::string out_str = "日本語で、回答して欲しい。 \n要約して欲しい。\n\n";
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
      }
    } catch (const std::exception& e) {
        std::cout << "Error , main" << std::endl;
        return 0;
    }        
    return 0;
}


