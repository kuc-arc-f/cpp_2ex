#include "qdrant_client.hpp"
#include <random>
#include <iomanip>
#include <uuid/uuid.h>

// ランダムなベクトルを生成するヘルパー
std::vector<float> randomVector(size_t dim, unsigned seed = 42) {
    std::mt19937 rng(seed);
    std::uniform_real_distribution<float> dist(-1.0f, 1.0f);
    std::vector<float> v(dim);
    for (auto& x : v) x = dist(rng);
    return v;
}

// ベクトルの先頭要素を表示するヘルパー
void printVector(const std::vector<float>& v, size_t max_show = 5) {
    std::cout << "[";
    for (size_t i = 0; i < std::min(v.size(), max_show); ++i) {
        std::cout << std::fixed << std::setprecision(3) << v[i];
        if (i + 1 < std::min(v.size(), max_show)) std::cout << ", ";
    }
    if (v.size() > max_show) std::cout << ", ...";
    std::cout << "]";
}
const std::string COLLECTION = "sample_collection";
const size_t      VECTOR_DIM = 128;  // ベクトル次元数

int main() {
    // =========================================================
    // 1. クライアント初期化
    // =========================================================
    // ローカルの Qdrant に接続 (Docker: docker run -p 6333:6333 qdrant/qdrant)
    QdrantClient client("localhost", 6333);

    std::cout << "=== Qdrant C++ クライアント デモ ===\n\n";

    // =========================================================
    // 2. コレクション管理
    // =========================================================
    std::cout << "--- コレクション一覧 ---\n";
    auto collections = client.listCollections();
    if (collections.empty()) {
        std::cout << "  (コレクションなし)\n";
    } else {
        for (auto& c : collections) std::cout << "  - " << c << "\n";
    }

    // =========================================================
    // 3. ベクトル登録 (Upsert)
    // =========================================================
    std::cout << "--- ベクトル登録 ---\n";

    std::vector<QdrantClient::Point> points;

    // ドキュメントサンプル
    struct Doc {
        uint64_t id;
        std::string title;
        std::string category;
        float price;
        unsigned seed;
    };

    std::vector<Doc> docs = {
        {1,  "りんごの育て方",     "農業",   980.0f,  1},
        {2,  "みかんの収穫時期",   "農業",   750.0f,  2},
        {3,  "C++プログラミング",  "IT",    2800.0f,  3},
        {4,  "Rustで始めるOS開発", "IT",    3200.0f,  4},
        {5,  "機械学習入門",       "AI",    1980.0f,  5},
        {6,  "深層学習の数学",     "AI",    2400.0f,  6},
        {7,  "料理の基本",         "料理",   580.0f,  7},
        {8,  "フランス料理入門",   "料理",  1200.0f,  8},
        {9,  "ベクトルDB徹底解説", "AI",    3000.0f,  9},
        {10, "分散システム設計",   "IT",    2600.0f, 10},
    };

    for (const auto& d : docs) {
        QdrantClient::Point p;
        p.id     = d.id;
        p.vector = randomVector(VECTOR_DIM, d.seed);
        p.payload = {
            {"title",    d.title},
            {"category", d.category},
            {"price",    d.price}
        };
        points.push_back(p);
    }

    client.upsertPoints(COLLECTION, points);
    std::cout << "\n";

    // =========================================================
    // 4. ベクトル検索 (類似検索)
    // =========================================================
    std::cout << "--- ベクトル類似検索 (Top 3) ---\n";
    // seed=5 (機械学習入門) に近いベクトルで検索
    auto query_vec = randomVector(VECTOR_DIM, 5);
    std::cout << "クエリベクトル: ";
    printVector(query_vec);
    std::cout << "\n\n";

    auto results = client.search(COLLECTION, query_vec, 3);
    for (const auto& r : results) {
        std::cout << "  ID: " << r.id
                  << "  スコア: " << std::fixed << std::setprecision(4) << r.score
                  << "  タイトル: " << r.payload["title"].get<std::string>()
                  << "  カテゴリ: " << r.payload["category"].get<std::string>()
                  << "\n";
    }
    std::cout << "\n";

    // =========================================================
    // 7. ID 指定でポイント取得
    // =========================================================
    std::cout << "--- ID=5 のポイントを取得 ---\n";
    auto pt = client.getPoint(COLLECTION, 5);
    if (pt.has_value()) {
        std::cout << "  ID: " << pt->id
                  << "  タイトル: " << pt->payload["title"].get<std::string>()
                  << "  価格: " << pt->payload["price"].get<float>() << "円\n";
    }
    std::cout << "\n";

    // 削除確認 (再検索)
    auto after_delete = client.search(COLLECTION, query_vec, 10);
    std::cout << "削除後の件数: " << after_delete.size() << " 件\n\n";

    std::cout << "=== デモ完了 ===\n";
    return 0;
}
