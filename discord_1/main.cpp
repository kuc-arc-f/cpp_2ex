
#include <iostream>
#include <string>
#include <map>
#include <stdexcept>
#include <curl/curl.h>
#include <nlohmann/json.hpp> // JSONライブラリ

#include "http_client.hpp"

// JSON用エイリアス
using json = nlohmann::json;

const std::string WEBHOOK_URL = "https://discord.com/api/webhooks/123/456";

struct QueryReq {
    std::string content;
    std::string username;
};
// これ一行で、QueryReq <=> json の変換が魔法のように可能になります
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(QueryReq, content, username)

// ─────────────────────────────────────────────
// main : 動作確認サンプル
// ─────────────────────────────────────────────
int main(int argc, char* argv[])
{
    if(argc < 2) {
        std::cerr << "[ERROR] argment none" << "\n";
        return 0;
    }
    int arg_count = argc;
    std::string input = argv[1];

    HttpClient client(30 /*timeout*/, true /*verify_ssl*/);

    try{
        QueryReq req;
        req.content = input;
        req.username = "Webhook-user-4";

        json j = req; // 構造体を代入するだけ！
        std::string json_str = j.dump();
        std::cout << "#Start discord post" << std::endl;
        std::cout << json_str << std::endl;
        auto resp = client.post_json(WEBHOOK_URL, json_str);
        print_response("POST-JSON:", resp);
    } catch (const std::exception& e) {
        std::cout << "Error , main" << std::endl;
        return 0;
    }    

    return 0;
}
