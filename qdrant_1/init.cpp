#include "qdrant_client.hpp"
#include <random>
#include <iomanip>

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

    // 既存コレクションを削除して再作成
    client.deleteCollection(COLLECTION);
    client.createCollection(COLLECTION, VECTOR_DIM, "Cosine");
    std::cout << "\n";

    return 0;
}
