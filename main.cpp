#include <crow.h>
#include <cstdlib>
#include <pqxx/pqxx>

int main()
{
    crow::SimpleApp app;


    

    CROW_ROUTE(app, "/api/books")( [] {
        crow::json::wvalue result;
        result["message"] = "Маршрут работaет";
        result["books"] = crow::json::wvalue::list{};
        return crow::response{result};
    });

    CROW_ROUTE(app, "/api/db-check")( [] {
        const char* db_url = std::getenv("DATABASE_URL");
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

    app.port(18080).run();
}