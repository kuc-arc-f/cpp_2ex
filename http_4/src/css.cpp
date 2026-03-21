#include "httplib.h"
#include <fstream>
#include <sstream>
#include <iostream>

// ファイルを文字列として読み込むユーティリティ
std::string readFile(const std::string& path) {
    std::ifstream ifs(path);
    if (!ifs.is_open()) {
        return "";
    }
    std::ostringstream oss;
    oss << ifs.rdbuf();
    return oss.str();
}

int main() {
    httplib::Server svr;

    // ─── SSR HTMLページ (CSSを<link>タグで参照) ───
    svr.Get("/", [](const httplib::Request&, httplib::Response& res) {
        std::string html = R"(<!DOCTYPE html>
<html lang="ja">
<head>
    <meta charset="UTF-8">
    <title>cpp-httplib SSR</title>
    <!-- CSSはサーバーから /style.css として配信 -->
    <link rel="stylesheet" href="/style.css">
</head>
<body>
    <h1>Hello from cpp-httplib SSR!</h1>
    <p>このページはサーバーサイドでレンダリングされています。</p>
</body>
</html>)";
        res.set_content(html, "text/html; charset=utf-8");
    });

    // ─── CSSファイルの配信 ───
    svr.Get("/style.css", [](const httplib::Request&, httplib::Response& res) {
        std::string css = readFile("static/style.css");
        if (css.empty()) {
            res.status = 404;
            res.set_content("CSS not found", "text/plain");
            return;
        }
        res.set_content(css, "text/css; charset=utf-8");
    });

    // ─── 汎用的な静的ファイル配信 (応用版) ───
    // 複数CSSや画像にも対応したい場合はこちらを使う
    svr.Get(R"(/static/(.+))", [](const httplib::Request& req, httplib::Response& res) {
        std::string filename = "static/" + req.matches[1].str();
        std::string content = readFile(filename);
        if (content.empty()) {
            res.status = 404;
            res.set_content("File not found", "text/plain");
            return;
        }

        std::string mime = "application/octet-stream";
        //if (filename.ends_with(".css"))  mime = "text/css; charset=utf-8";
        //if (filename.ends_with(".js"))   mime = "application/javascript";
        //if (filename.ends_with(".html")) mime = "text/html; charset=utf-8";

        res.set_content(content, mime);
    });

    //std::cout << "Server running at http://localhost:8080" << std::endl;
    std::cout << "Server running at http://localhost:8000" << std::endl;
    svr.listen("0.0.0.0", 8000);

    return 0;
}