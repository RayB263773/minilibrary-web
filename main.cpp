#include <crow.h>
#include <cstdlib>
#include <iostream>
#include <string>

#include "books.h"

int main()
{
    crow::SimpleApp app;
    const char* db_url = std::getenv("DATABASE_URL");
    if (db_url == nullptr) {
        std::cerr << "Ошибка: DATABASE_URL не задан\n";
        return 1;
    }

    CROW_ROUTE(app, "/api/books")( [db_url] {
        try {
            std::vector<Book> books = get_all_books(db_url);
            crow::json::wvalue::list books_list;
            for (const auto& b : books) {
                crow::json::wvalue book_json;
                book_json["id"] = b.id;
                book_json["name"] = b.name;
                book_json["author"] = b.author;
                book_json["year"] = b.year;
                books_list.push_back(std::move(book_json));
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