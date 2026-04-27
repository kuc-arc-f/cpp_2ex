#include <chrono>
#include <curl/curl.h>
#include <iostream>
#include <map>
#include <string>
#include <stdexcept>
#include <nlohmann/json.hpp>

#include "http_client.hpp"

// JSON用エイリアス
using json = nlohmann::json;

struct QueryReq {
    std::string title;
};
// これ一行で、QueryReq <=> json の変換が魔法のように可能になります
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(QueryReq, title)

std::string URL_TEST = "http://localhost:8000/todos";

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

int main()
{
    std::cout << "#start:" << std::endl;
    try{
        // 開始時刻
        auto start = std::chrono::high_resolution_clock::now();

        HttpClient client(30 /*timeout*/, true /*verify_ssl*/);

        QueryReq req;
        req.title = "test-1";

        json j = req; // 構造体を代入するだけ！
        std::string json_str = j.dump();
        int i;
        for(i = 0; i < 10; i++) {
            //std::wcout << L"Post " << i+1 << std::endl;
             auto resp = client.post_json("http://localhost:8000/todos", json_str);
            if (!resp.error.empty()) {
                std::cerr << L"[ERROR] \n";
                return 1;
            }
            //std::cout << "Status : " << resp.status_code << "\n";
        }        
        // 終了時刻
        auto end = std::chrono::high_resolution_clock::now();
        // 差分（ミリ秒）
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        std::cout << "time: " << duration.count() << " ms" << std::endl;        

    } catch (const std::exception& e) {
        std::cerr << "\n[ERROR] " << e.what() << std::endl;
        //std::wcout << L"Error , main" << std::endl;
        return 0;
    }   
}
