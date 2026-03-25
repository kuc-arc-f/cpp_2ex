#include <iostream>
#include <string>
#include <vector>
#include "duckdb.hpp"

using namespace duckdb;
using namespace std;

std::string DB_PATH = "./todos.db";

// ============================================================
// ユーティリティ: エラーチェック付き実行
// ============================================================
static void exec(Connection& con, const string& sql) {
    auto result = con.Query(sql);
    if (result->HasError()) {
        cerr << "[ERROR] " << result->GetError() << "\n";
        cerr << "  SQL: " << sql << "\n";
    }
}

// ============================================================
// CREATE TABLE
// ============================================================
void create_table(Connection& con) {
    cout << "\n=== CREATE TABLE ===\n";

    exec(con, "DROP TABLE IF EXISTS employees");

    exec(con, R"(
        CREATE TABLE employees (
            id      INTEGER PRIMARY KEY,
            name    VARCHAR NOT NULL,
            dept    VARCHAR,
            salary  DOUBLE,
            hired   DATE
        )
    )");

    cout << "テーブル employees を作成しました。\n";
}

// ============================================================
// INSERT (Create)
// ============================================================
void insert_records(Connection& con) {
    cout << "\n=== INSERT (Create) ===\n";

    // --- 1件ずつ INSERT ---
    exec(con, "INSERT INTO employees VALUES (1, '田中 太郎', '開発',  750000.0, '2020-04-01')");
    exec(con, "INSERT INTO employees VALUES (2, '鈴木 花子', '営業',  680000.0, '2021-07-15')");
    exec(con, "INSERT INTO employees VALUES (3, '佐藤 次郎', '開発',  820000.0, '2019-10-01')");
    exec(con, "INSERT INTO employees VALUES (4, '山田 三郎', '人事',  620000.0, '2022-01-10')");
    exec(con, "INSERT INTO employees VALUES (5, '伊藤 美咲', '営業',  710000.0, '2023-03-20')");

    // --- Prepared Statement による INSERT ---
    auto prep = con.Prepare(
        "INSERT INTO employees VALUES (?, ?, ?, ?, ?)");

    prep->Execute(6, "中村 健一", "開発",  790000.0, "2018-06-01");
    prep->Execute(7, "小林 由美", "人事",  650000.0, "2024-02-28");

    cout << "7件のレコードを挿入しました。\n";
}



// ============================================================
// UPDATE
// ============================================================
void update_records(Connection& con) {
    cout << "\n=== UPDATE ===\n";

    // --- 1件更新 ---
    exec(con, "UPDATE employees SET salary = 850000.0 WHERE id = 1");
    cout << "id=1 の給与を 850000 に更新しました。\n";

    // --- 条件一括更新 ---
    exec(con, "UPDATE employees SET salary = salary * 1.05 WHERE dept = '営業'");
    cout << "営業部門の給与を 5% 昇給しました。\n";

    // --- Prepared Statement による更新 ---
    auto prep = con.Prepare(
        "UPDATE employees SET dept = ?, salary = ? WHERE id = ?");
    prep->Execute("マーケティング", 720000.0, 4);
    cout << "id=4 の部署を マーケティング に変更しました。\n";

    // 更新確認
    auto result = con.Query(
        "SELECT id, name, dept, salary FROM employees WHERE id IN (1, 4, 2, 5) ORDER BY id");
    cout << "\n--- 更新後確認 ---\n";
    for (auto& row : *result) {
        cout << "  id=" << row.GetValue<int32_t>(0)
             << "  " << row.GetValue<string>(1)
             << "  [" << row.GetValue<string>(2) << "]"
             << "  ¥" << row.GetValue<double>(3) << "\n";
    }
}

// ============================================================
// DELETE
// ============================================================
void delete_records(Connection& con) {
    cout << "\n=== DELETE ===\n";

    // --- 1件削除 ---
    exec(con, "DELETE FROM employees WHERE id = 7");
    cout << "id=7 を削除しました。\n";

    // --- 条件削除 ---
    exec(con, "DELETE FROM employees WHERE salary < 660000");
    cout << "給与 < 660000 のレコードを削除しました。\n";

    // 残件確認
    auto result = con.Query("SELECT COUNT(*) FROM employees");
    cout << "残レコード数: "
         << result->GetValue<int64_t>(0, 0) << " 件\n";
}

// ============================================================
// TRANSACTION デモ
// ============================================================
void transaction_demo(Connection& con) {
    cout << "\n=== TRANSACTION ===\n";

    // --- コミット例 ---
    exec(con, "BEGIN");
    exec(con, "INSERT INTO employees VALUES (10, 'テスト 一郎', '開発', 700000, '2025-01-01')");
    exec(con, "UPDATE employees SET salary = salary + 10000 WHERE id = 10");
    exec(con, "COMMIT");
    cout << "トランザクション COMMIT 完了。\n";

    // --- ロールバック例 ---
    exec(con, "BEGIN");
    exec(con, "INSERT INTO employees VALUES (11, 'ロール バック', '開発', 500000, '2025-06-01')");
    exec(con, "ROLLBACK");
    cout << "トランザクション ROLLBACK 完了。\n";

    // id=11 は存在しないはず
    auto r = con.Query("SELECT COUNT(*) FROM employees WHERE id = 11");
    cout << "id=11 の件数 (0 のはず): " << r->GetValue<int64_t>(0, 0) << "\n";
}

/**
*
* @param
*
* @return
*/
void file_db_demo() {
    DuckDB db(DB_PATH);
    Connection con(db);
    
   //select
    auto result = con.Query("SELECT id, title FROM todos ORDER BY id DESC");
    cout << "ファイルDBから再読み込み:\n";
    for (auto& row : *result) {
        cout << "  " << row.GetValue<int32_t>(0)
              << " => " << row.GetValue<string>(1) << "\n";
    }
}

// ============================================================
// main
// ============================================================
int main() {
    cout << "========================================\n";
    cout << "  DuckDB C++ CRUD サンプル\n";
    cout << "========================================\n";

    file_db_demo();

    cout << "\n全処理が完了しました。\n";
    return 0;
}
