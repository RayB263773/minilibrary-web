#include <crow.h>

int main()
{
    crow::SimpleApp app;

    CROW_ROUTE(app, "/api/books")( [] {
        return "Minilibrary server is running!";
    });
    app.port(18080).run();
}