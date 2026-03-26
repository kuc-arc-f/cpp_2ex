#include <iostream>
#include <string>
#include <vector>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <sqlite3.h>

// ─────────────────────────────────────────────
//  カラー定数
// ─────────────────────────────────────────────
namespace Color {
    const std::string RESET   = "\033[0m";
    const std::string BOLD    = "\033[1m";
    const std::string RED     = "\033[31m";
    const std::string GREEN   = "\033[32m";
    const std::string YELLOW  = "\033[33m";
    const std::string BLUE    = "\033[34m";
    const std::string CYAN    = "\033[36m";
    const std::string GRAY    = "\033[90m";
}

// ─────────────────────────────────────────────
//  データ構造
// ─────────────────────────────────────────────
struct Todo {
    int         id;
    std::string title;
    bool        done;
    std::string priority;   // high / medium / low
    std::string created_at;
};

// ─────────────────────────────────────────────
//  DB クラス
// ─────────────────────────────────────────────
class Database {
public:
    explicit Database(const std::string& path) : db_(nullptr) {
        if (sqlite3_open(path.c_str(), &db_) != SQLITE_OK) {
            std::cerr << "DB オープン失敗: " << sqlite3_errmsg(db_) << "\n";
            std::exit(1);
        }
        exec("PRAGMA journal_mode=WAL;");
        createTable();
    }
    ~Database() { if (db_) sqlite3_close(db_); }

    // ── CRUD ──────────────────────────────────
    bool addTodo(const std::string& title, const std::string& priority) {
        const char* sql =
            "INSERT INTO todos (title, done, priority) VALUES (?, 0, ?);";
        sqlite3_stmt* stmt = prepare(sql);
        sqlite3_bind_text(stmt, 1, title.c_str(),    -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, priority.c_str(), -1, SQLITE_TRANSIENT);
        bool ok = sqlite3_step(stmt) == SQLITE_DONE;
        sqlite3_finalize(stmt);
        return ok;
    }

    bool removeTodo(int id) {
        const char* sql = "DELETE FROM todos WHERE id = ?;";
        sqlite3_stmt* stmt = prepare(sql);
        sqlite3_bind_int(stmt, 1, id);
        bool ok = sqlite3_step(stmt) == SQLITE_DONE;
        sqlite3_finalize(stmt);
        return ok && sqlite3_changes(db_) > 0;
    }

    bool toggleTodo(int id) {
        const char* sql = "UPDATE todos SET done = NOT done WHERE id = ?;";
        sqlite3_stmt* stmt = prepare(sql);
        sqlite3_bind_int(stmt, 1, id);
        bool ok = sqlite3_step(stmt) == SQLITE_DONE;
        sqlite3_finalize(stmt);
        return ok && sqlite3_changes(db_) > 0;
    }

    bool editTitle(int id, const std::string& newTitle) {
        const char* sql = "UPDATE todos SET title = ? WHERE id = ?;";
        sqlite3_stmt* stmt = prepare(sql);
        sqlite3_bind_text(stmt, 1, newTitle.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(stmt, 2, id);
        bool ok = sqlite3_step(stmt) == SQLITE_DONE;
        sqlite3_finalize(stmt);
        return ok && sqlite3_changes(db_) > 0;
    }

    bool clearDone() {
        exec("DELETE FROM todos WHERE done = 1;");
        return sqlite3_changes(db_) > 0;
    }

    std::vector<Todo> getTodos(const std::string& filter = "all") {
        std::string sql =
            "SELECT id, title, done, priority, created_at FROM todos";
        if      (filter == "active") sql += " WHERE done = 0";
        else if (filter == "done")   sql += " WHERE done = 1";
        sql += " ORDER BY CASE priority WHEN 'high' THEN 1"
               "  WHEN 'medium' THEN 2 ELSE 3 END, id;";

        sqlite3_stmt* stmt = prepare(sql.c_str());
        std::vector<Todo> todos;
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            Todo t;
            t.id         = sqlite3_column_int (stmt, 0);
            t.title      = col_text(stmt, 1);
            t.done       = sqlite3_column_int (stmt, 2) != 0;
            t.priority   = col_text(stmt, 3);
            t.created_at = col_text(stmt, 4);
            todos.push_back(t);
        }
        sqlite3_finalize(stmt);
        return todos;
    }

    std::vector<Todo> search(const std::string& keyword) {
        const char* sql =
            "SELECT id, title, done, priority, created_at FROM todos"
            " WHERE title LIKE ? ORDER BY id;";
        sqlite3_stmt* stmt = prepare(sql);
        std::string pat = "%" + keyword + "%";
        sqlite3_bind_text(stmt, 1, pat.c_str(), -1, SQLITE_TRANSIENT);
        std::vector<Todo> todos;
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            Todo t;
            t.id         = sqlite3_column_int (stmt, 0);
            t.title      = col_text(stmt, 1);
            t.done       = sqlite3_column_int (stmt, 2) != 0;
            t.priority   = col_text(stmt, 3);
            t.created_at = col_text(stmt, 4);
            todos.push_back(t);
        }
        sqlite3_finalize(stmt);
        return todos;
    }

private:
    sqlite3* db_;

    void createTable() {
        exec(R"(
            CREATE TABLE IF NOT EXISTS todos (
                id         INTEGER PRIMARY KEY AUTOINCREMENT,
                title      TEXT    NOT NULL,
                done       INTEGER NOT NULL DEFAULT 0,
                priority   TEXT    NOT NULL DEFAULT 'medium',
                created_at TEXT    NOT NULL DEFAULT (datetime('now','localtime'))
            );
        )");
    }

    void exec(const std::string& sql) {
        char* err = nullptr;
        if (sqlite3_exec(db_, sql.c_str(), nullptr, nullptr, &err) != SQLITE_OK) {
            std::cerr << "SQL エラー: " << err << "\n";
            sqlite3_free(err);
        }
    }

    sqlite3_stmt* prepare(const char* sql) {
        sqlite3_stmt* stmt = nullptr;
        if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) {
            std::cerr << "Prepare 失敗: " << sqlite3_errmsg(db_) << "\n";
            std::exit(1);
        }
        return stmt;
    }

    static std::string col_text(sqlite3_stmt* s, int col) {
        const unsigned char* t = sqlite3_column_text(s, col);
        return t ? reinterpret_cast<const char*>(t) : "";
    }
};

// ─────────────────────────────────────────────
//  表示ヘルパー
// ─────────────────────────────────────────────
static std::string priorityLabel(const std::string& p) {
    if (p == "high")   return Color::RED    + "[高]" + Color::RESET;
    if (p == "low")    return Color::GRAY   + "[低]" + Color::RESET;
    return                    Color::YELLOW + "[中]" + Color::RESET;
}

static void printBanner() {
    std::cout << "\n"
        << Color::CYAN << Color::BOLD
        << "  ╔══════════════════════════════╗\n"
        << "  ║       📝  TODO  CLI          ║\n"
        << "  ╚══════════════════════════════╝"
        << Color::RESET << "\n\n";
}

static void printHelp() {
    std::cout << Color::BOLD << "コマンド一覧:\n" << Color::RESET
        << "  " << Color::GREEN << "list   [all|active|done]" << Color::RESET << "  タスク一覧\n"
        << "  " << Color::GREEN << "add    <タイトル> [high|medium|low]" << Color::RESET << "  追加\n"
        << "  " << Color::GREEN << "done   <id>"  << Color::RESET << "                完了トグル\n"
        << "  " << Color::GREEN << "edit   <id> <新タイトル>" << Color::RESET << "    タイトル編集\n"
        << "  " << Color::GREEN << "remove <id>"  << Color::RESET << "                削除\n"
        << "  " << Color::GREEN << "search <キーワード>" << Color::RESET << "         検索\n"
        << "  " << Color::GREEN << "clear"        << Color::RESET << "                完了済みを削除\n"
        << "  " << Color::GREEN << "help"         << Color::RESET << "                このヘルプ\n"
        << "  " << Color::GREEN << "exit / quit"  << Color::RESET << "                終了\n\n";
}

static void printTodos(const std::vector<Todo>& todos) {
    if (todos.empty()) {
        std::cout << Color::GRAY << "  (タスクなし)\n" << Color::RESET;
        return;
    }
    std::cout << Color::BOLD
        << "  " << std::left
        << std::setw(5)  << "ID"
        << std::setw(6)  << "状態"
        << std::setw(6)  << "優先"
        << "タイトル\n" << Color::RESET;
    std::cout << "  " << std::string(55, '-') << "\n";

    for (const auto& t : todos) {
        std::string status = t.done
            ? Color::GREEN + " ✓ " + Color::RESET
            : Color::YELLOW + " ○ " + Color::RESET;
        std::string title = t.done
            ? Color::GRAY + t.title + Color::RESET
            : t.title;

        std::cout << "  "
            << Color::BOLD << std::setw(5) << t.id << Color::RESET
            << status << "  "
            << priorityLabel(t.priority) << "  "
            << title << "\n";
    }
    std::cout << "\n";
}

// ─────────────────────────────────────────────
//  引数パーサー
// ─────────────────────────────────────────────
static std::vector<std::string> split(const std::string& s) {
    std::vector<std::string> tokens;
    std::istringstream iss(s);
    std::string tok;
    while (iss >> tok) tokens.push_back(tok);
    return tokens;
}

static std::string joinFrom(const std::vector<std::string>& v, size_t from, size_t len = std::string::npos) {
    std::string result;
    size_t end = (len == std::string::npos) ? v.size() : std::min(from + len, v.size());
    for (size_t i = from; i < end; ++i) {
        if (!result.empty()) result += ' ';
        result += v[i];
    }
    return result;
}

// ─────────────────────────────────────────────
//  メインループ
// ─────────────────────────────────────────────
int main(int argc, char* argv[]) {
    std::string dbPath = "todo.db";
    if (argc >= 2) dbPath = argv[1];

    Database db(dbPath);
    printBanner();
    std::cout << Color::GRAY << "  DB: " << dbPath << "\n\n" << Color::RESET;
    printHelp();

    std::string line;
    while (true) {
        std::cout << Color::CYAN << "todo> " << Color::RESET;
        if (!std::getline(std::cin, line)) break;

        auto args = split(line);
        if (args.empty()) continue;

        const std::string& cmd = args[0];

        // ── list ──────────────────────────────
        if (cmd == "list" || cmd == "ls") {
            std::string filter = (args.size() >= 2) ? args[1] : "all";
            auto todos = db.getTodos(filter);
            int total = (int)todos.size();
            int done  = (int)std::count_if(todos.begin(), todos.end(),
                              [](const Todo& t){ return t.done; });
            std::cout << "\n  フィルター: " << Color::BOLD << filter << Color::RESET
                      << "  |  全 " << total << " 件 / 完了 " << done << " 件\n\n";
            printTodos(todos);
        }

        // ── add ───────────────────────────────
        else if (cmd == "add") {
            if (args.size() < 2) {
                std::cout << Color::RED << "  使い方: add <タイトル> [high|medium|low]\n" << Color::RESET;
                continue;
            }
            // 最後の引数が priority かどうか判定
            std::string priority = "medium";
            size_t titleEnd = args.size();
            if (args.back() == "high" || args.back() == "medium" || args.back() == "low") {
                priority = args.back();
                titleEnd = args.size() - 1;
            }
            std::string title = joinFrom(args, 1, titleEnd - 1);
            if (title.empty()) {
                std::cout << Color::RED << "  タイトルが空です。\n" << Color::RESET;
                continue;
            }
            if (db.addTodo(title, priority)) {
                std::cout << Color::GREEN << "  ✓ 追加しました: " << Color::RESET
                          << title << " " << priorityLabel(priority) << "\n\n";
            }
        }

        // ── done ──────────────────────────────
        else if (cmd == "done" || cmd == "toggle") {
            if (args.size() < 2) {
                std::cout << Color::RED << "  使い方: done <id>\n" << Color::RESET;
                continue;
            }
            int id = std::stoi(args[1]);
            if (db.toggleTodo(id))
                std::cout << Color::GREEN << "  ✓ 状態を切り替えました (id=" << id << ")\n\n" << Color::RESET;
            else
                std::cout << Color::RED << "  id=" << id << " が見つかりません。\n\n" << Color::RESET;
        }

        // ── edit ──────────────────────────────
        else if (cmd == "edit") {
            if (args.size() < 3) {
                std::cout << Color::RED << "  使い方: edit <id> <新タイトル>\n" << Color::RESET;
                continue;
            }
            int id = std::stoi(args[1]);
            std::string newTitle = joinFrom(args, 2);
            if (db.editTitle(id, newTitle))
                std::cout << Color::GREEN << "  ✓ 更新しました (id=" << id << ")\n\n" << Color::RESET;
            else
                std::cout << Color::RED << "  id=" << id << " が見つかりません。\n\n" << Color::RESET;
        }

        // ── remove ────────────────────────────
        else if (cmd == "remove" || cmd == "rm" || cmd == "delete") {
            if (args.size() < 2) {
                std::cout << Color::RED << "  使い方: remove <id>\n" << Color::RESET;
                continue;
            }
            int id = std::stoi(args[1]);
            if (db.removeTodo(id))
                std::cout << Color::GREEN << "  ✓ 削除しました (id=" << id << ")\n\n" << Color::RESET;
            else
                std::cout << Color::RED << "  id=" << id << " が見つかりません。\n\n" << Color::RESET;
        }

        // ── search ────────────────────────────
        else if (cmd == "search" || cmd == "find") {
            if (args.size() < 2) {
                std::cout << Color::RED << "  使い方: search <キーワード>\n" << Color::RESET;
                continue;
            }
            std::string kw = joinFrom(args, 1);
            auto todos = db.search(kw);
            std::cout << "\n  検索: \"" << kw << "\"  " << todos.size() << " 件\n\n";
            printTodos(todos);
        }

        // ── clear ─────────────────────────────
        else if (cmd == "clear") {
            if (db.clearDone())
                std::cout << Color::GREEN << "  ✓ 完了済みタスクを削除しました。\n\n" << Color::RESET;
            else
                std::cout << Color::GRAY << "  完了済みタスクはありません。\n\n" << Color::RESET;
        }

        // ── help ──────────────────────────────
        else if (cmd == "help" || cmd == "h" || cmd == "?") {
            printHelp();
        }

        // ── exit ──────────────────────────────
        else if (cmd == "exit" || cmd == "quit" || cmd == "q") {
            std::cout << Color::CYAN << "  バイバイ！👋\n\n" << Color::RESET;
            break;
        }

        // ── 不明コマンド ──────────────────────
        else {
            std::cout << Color::RED << "  不明なコマンド: " << cmd
                      << "  (help で一覧表示)\n\n" << Color::RESET;
        }
    }

    return 0;
}
