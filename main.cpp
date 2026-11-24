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

    try {
        init_db(db_url);
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        return 1;
    }

    // Маршрут для получения списка книг (без тяжелого текста)
    CROW_ROUTE(app, "/api/books")([db_url](const crow::request& req) {
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

    // Маршрут для подгрузки текста одной книги по клику на "Читать"
    CROW_ROUTE(app, "/api/books/<int>").methods(crow::HTTPMethod::GET)([db_url](int id) {
        try {
            std::string content = get_book_content(db_url, id);
            crow::json::wvalue result;
            result["content"] = std::move(content);
            return crow::response{result};
        } catch (const std::exception& e) {
            return crow::response(500, std::string("Ошибка: ") + e.what());
        }
    });

    // Маршрут для добавления книги
    CROW_ROUTE(app, "/api/books").methods(crow::HTTPMethod::POST)([db_url](const crow::request& req) {
        try {
            auto body = crow::json::load(req.body);
            if (!body || !body.has("name") || !body.has("author") || !body.has("year")) {
                return crow::response(400, "Ошибка: нужны поля name, author и year");
            }

            std::string name = body["name"].s();
            std::string author = body["author"].s();
            int year = body["year"].i();
            std::string content = body.has("content") ? std::string(body["content"].s()) : "";

            add_book(db_url, name, author, year, content);
            return crow::response(201, "Книга успешно добавлена");
        } catch (const std::exception& e) {
            return crow::response(400, std::string("Ошибка: ") + e.what());
        }
    });

    // Маршрут для удаления книги
    CROW_ROUTE(app, "/api/books/<int>").methods(crow::HTTPMethod::DELETE)([db_url](int id) {
        try {
            delete_book(db_url, id);
            return crow::response(200, "Книга успешно удалена");
        } catch (const std::exception& e) {
            return crow::response(500, std::string("Ошибка: ") + e.what());
        }
    });

    // Маршрут для главной страницы
    CROW_ROUTE(app, "/")([]{
        crow::response res;
        res.set_static_file_info("public/index.html");
        return res;
    });

    // Маршрут для таблицы стилей
    CROW_ROUTE(app, "/style.css")([]{
        crow::response res;
        res.set_static_file_info("public/style.css");
        return res;
    });

    CROW_ROUTE(app, "/api/books/<int>").methods(crow::HTTPMethod::PUT)([db_url](const crow::request& req, int id) {
        try {
            auto body = crow::json::load(req.body);
            if (!body || !body.has("name") || !body.has("author") || !body.has("year")) {
                return crow::response(400, "Ошибка: нужны поля name, author и year");
            }
            std::string name = body["name"].s();
            std::string author = body["author"].s();
            int year = body["year"].i();
            std::string content = body.has("content") ? std::string(body["content"].s()) : "";
            update_book(db_url, id, name, author, year, content);
            return crow::response(200, "Книга успешно обновлена");
        } catch (const std::exception& e) {
            return crow::response(400, std::string("Ошибка: ") + e.what());
        }
    });

    const char* port_env = std::getenv("PORT");
    int port = (port_env != nullptr) ? std::stoi(port_env) : 18080;
    app.port(port).run();
}