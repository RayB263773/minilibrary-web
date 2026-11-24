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
// Маршрут для получения всех книг
    CROW_ROUTE(app, "/api/books")( [db_url](const crow::request& req) {
        try {
            std::vector<Book> books;
            const char* q = req.url_params.get("q");
            if (q != nullptr) {
                books = find_books(db_url, q);
            } else {
                books = get_all_books(db_url);
            }
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
// Маршрут для добавления книги
    CROW_ROUTE(app, "/api/books").methods(crow::HTTPMethod::POST)( [db_url](const crow::request& req) {
        try {
            auto body = crow::json::load(req.body);
            // Проверяем, что JSON корректный и все три поля присутствуют
            if (!body || !body.has("name") || !body.has("author") || !body.has("year")) {
                return crow::response(400, "Ошибка: нужны поля name, author и year");
            }

            std::string name = body["name"].s();
            std::string author = body["author"].s();
            int year = body["year"].i();
            add_book(db_url, name, author, year);
            return crow::response(201, "Книга успешно добавлена");
        } catch (const std::exception& e) {
            return crow::response(400, std::string("Ошибка: ") + e.what());
        }

    });
// Маршрут для удаления книги
    CROW_ROUTE(app, "/api/books/<int>").methods(crow::HTTPMethod::DELETE)( [db_url](int id) {
        try {
            delete_book(db_url, id);
            return crow::response(200, "Книга успешно удалена");
        } catch (const std::exception& e) {
            return crow::response(400, std::string("Ошибка: ") + e.what());
        }
    });
// Маршрут для отображения главной страницы
    CROW_ROUTE(app, "/")( []{
        crow::response res;
        res.set_static_file_info("public/index.html");
        return res;
    });
    app.port(18080).run();
}