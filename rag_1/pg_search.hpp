#include <iostream>
#include <vector>
#include <pqxx/pqxx> // libpqxx が必要
#include <sstream>
#include <string>

using namespace std;

class PgSearch {
private:
    std::string name = "";

public:
    explicit PgSearch(std::string str){}

    std::string pg_search(std::vector<float> embedding) {
      std::string ret = "";
      try {
          pqxx::connection conn("host=localhost user=root dbname=mydb password=admin");
          if (conn.is_open()) {
              cout << "Connected to database successfully." << endl;
          }

          pqxx::work txn(conn);

          vector<float> query_vector = embedding;

          stringstream ss;
          ss << "[";
          for (size_t i = 0; i < query_vector.size(); ++i) {
              if (i > 0) ss << ",";
              ss << query_vector[i];
          }
          ss << "]";
          string query_vec_str = ss.str();

          string sql = "SELECT id, embedding , content FROM documents ORDER BY embedding <-> '" + query_vec_str + "' LIMIT 1;";

          pqxx::result result = txn.exec(sql);

          cout << "Top similar vectors:\n";
          std::string matches = "";
          for (const auto& row : result) {
              int id = row["id"].as<int>();
              string emb = row["embedding"].c_str();
              std::string content = row["content"].c_str();
              matches = content;
              //cout << "ID: " << id << ", content: " << content << endl;
          }
          txn.commit();

          ret = matches;
          return ret;
      } catch (const exception &e) {
          cerr << e.what() << endl;
          return ret;
      }
      return ret;
    }

};
