#include <crow.h>

int main()
{
    crow::SimpleApp app;

    CROW_ROUTE(app, "/api/books")( [] {
        crow::json::wvalue result;
        result["message"] = "Маршрут работaет";
        result["books"] = crow::json::wvalue::list{};
        return crow::response{result};
    });
    app.port(18080).run();
}