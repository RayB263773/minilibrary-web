#include <crow.h>
#include <cstdlib>
#include <pqxx/pqxx>
#include <string>

int main()
{
    crow::SimpleApp app;
    const char* db_url = std::getenv("DATABASE_URL");
    if (db_url == nullptr) {
        std::cerr << "Ошибка: DATABASE_URL не задан\n";
        return 1;
    }

    CROW_ROUTE(app, "/api/db-check")( [db_url] {
        if (db_url == nullptr) {
            return crow::response(500, "Ошибка: DATABASE_URL не задан");
        }
        try {
            pqxx::connection conn(db_url);
            pqxx::read_transaction tx(conn);
            tx.exec("SELECT 1");
            return crow::response(200, "Соединение с базой данных успешно установлено");
        } catch (const std::exception& e) {
            return crow::response(500, std::string("Ошибка: ") + e.what());
        }
    });

    CROW_ROUTE(app, "/api/books")( [db_url] {
        try {
            pqxx::connection conn(db_url);
            pqxx::read_transaction tx(conn);
            pqxx::result rows = tx.exec("SELECT id, name, author, year FROM book ORDER BY id");
            crow::json::wvalue::list books_list;
            for (const auto& row : rows) {
                crow::json::wvalue book;
                book["id"] = row["id"].as<int>();
                book["name"] = row["name"].as<std::string>();
                book["author"] = row["author"].as<std::string>();
                book["year"] = row["year"].as<int>();
                books_list.push_back(std::move(book));
            }
            crow::json::wvalue result;
            result["books"] = std::move(books_list);
            return crow::response{result};
            
        } catch (const std::exception& e) {
            return crow::response(500, std::string("Ошибка: ") + e.what());
        }
    });
    app.port(18080).run();
}