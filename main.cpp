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

    // GET /api/books — получение списка или поиск с учетом пользователя (Общая или Моя библиотека)
    CROW_ROUTE(app, "/api/books").methods(crow::HTTPMethod::GET)([db_url](const crow::request& req) {
        try {
            std::string auth_header = req.get_header_value("Authorization");
            std::string token = "";
            if (auth_header.rfind("Bearer ", 0) == 0) {
                token = auth_header.substr(7);
            }
            int user_id = get_user_id_by_token(db_url, token);

            char* q_param = req.url_params.get("q");
            std::vector<Book> books;

            if (q_param) {
                books = find_books(db_url, q_param, user_id);
            } else {
                books = get_all_books(db_url, user_id);
            }

            crow::json::wvalue res;
            crow::json::wvalue::list books_list;
            for (const auto& b : books) {
                crow::json::wvalue book_obj;
                book_obj["id"] = b.id;
                book_obj["name"] = b.name;
                book_obj["author"] = b.author;
                book_obj["year"] = b.year;
                books_list.push_back(std::move(book_obj));
            }
            res["books"] = std::move(books_list);
            return crow::response(200, res);
        } catch (const std::exception& e) {
            return crow::response(500, std::string("Ошибка: ") + e.what());
        }
    });

    // POST /api/books — добавление книги в текущую библиотеку
    CROW_ROUTE(app, "/api/books").methods(crow::HTTPMethod::POST)([db_url](const crow::request& req) {
        try {
            std::string auth_header = req.get_header_value("Authorization");
            std::string token = "";
            if (auth_header.rfind("Bearer ", 0) == 0) {
                token = auth_header.substr(7);
            }
            int user_id = get_user_id_by_token(db_url, token);

            auto body = crow::json::load(req.body);
            if (!body || !body.has("name") || !body.has("author") || !body.has("year")) {
                return crow::response(400, "Ошибка: нужны поля name, author и year");
            }

            std::string name = body["name"].s();
            std::string author = body["author"].s();
            int year = body["year"].i();
            std::string content = body.has("content") ? std::string(body["content"].s()) : "";

            add_book(db_url, name, author, year, content, user_id);
            return crow::response(201, "Книга успешно добавлена");
        } catch (const std::exception& e) {
            return crow::response(400, std::string("Ошибка: ") + e.what());
        }
    });

    // GET /api/books/<int> — получение текста книги для читалки
    CROW_ROUTE(app, "/api/books/<int>").methods(crow::HTTPMethod::GET)([db_url](const crow::request& req, int id) {
        try {
            const std::string auth_header = req.get_header_value("Authorization");
            const std::string token = auth_header.rfind("Bearer ", 0) == 0 ? auth_header.substr(7) : "";
            const int user_id = get_user_id_by_token(db_url, token);
            const std::string content = get_book_content(db_url, id, user_id);
            crow::json::wvalue res;
            res["content"] = content;
            return crow::response(200, res);
        } catch (const std::exception& e) {
            return crow::response(500, std::string("Ошибка: ") + e.what());
        }
    });

    // PUT /api/books/<int> — редактирование книги (только своей или общей)
    CROW_ROUTE(app, "/api/books/<int>").methods(crow::HTTPMethod::PUT)([db_url](const crow::request& req, int id) {
        try {
            std::string auth_header = req.get_header_value("Authorization");
            std::string token = "";
            if (auth_header.rfind("Bearer ", 0) == 0) {
                token = auth_header.substr(7);
            }
            int user_id = get_user_id_by_token(db_url, token);

            auto body = crow::json::load(req.body);
            if (!body || !body.has("name") || !body.has("author") || !body.has("year")) {
                return crow::response(400, "Ошибка: нужны поля name, author и year");
            }

            std::string name = body["name"].s();
            std::string author = body["author"].s();
            int year = body["year"].i();
            std::string content = body.has("content") ? std::string(body["content"].s()) : "";

            update_book(db_url, id, name, author, year, content, user_id);
            return crow::response(200, "Книга успешно обновлена");
        } catch (const std::exception& e) {
            return crow::response(400, std::string("Ошибка: ") + e.what());
        }
    });

    // DELETE /api/books/<int> — удаление книги (только своей или общей)
    CROW_ROUTE(app, "/api/books/<int>").methods(crow::HTTPMethod::DELETE)([db_url](const crow::request& req, int id) {
        try {
            std::string auth_header = req.get_header_value("Authorization");
            std::string token = "";
            if (auth_header.rfind("Bearer ", 0) == 0) {
                token = auth_header.substr(7);
            }
            int user_id = get_user_id_by_token(db_url, token);

            delete_book(db_url, id, user_id);
            return crow::response(200, "Книга успешно удалена");
        } catch (const std::exception& e) {
            return crow::response(400, std::string("Ошибка: ") + e.what());
        }
    });

    // Маршрут для главной страницы
    CROW_ROUTE(app, "/")([]{
        crow::response res;
        res.set_static_file_info("public/index.html");
        res.set_header("Content-Type", "text/html; charset=utf-8");
        return res;
    });

    // Маршрут для таблицы стилей
    CROW_ROUTE(app, "/style.css")([]{
        crow::response res;
        res.set_static_file_info("public/style.css");
        res.set_header("Content-Type", "text/css; charset=utf-8");
        return res;
    });

    // Маршруты для аккаунтов

// Регистрация нового аккаунта
    CROW_ROUTE(app, "/api/register").methods(crow::HTTPMethod::POST)([db_url](const crow::request& req) {
        try {
            auto body = crow::json::load(req.body);
            if (!body || !body.has("username") || !body.has("password")) {
                return crow::response(400, "Ошибка: нужны поля username и password");
            }

            std::string username = body["username"].s();
            std::string password = body["password"].s();

            if (username.empty() || password.empty()) {
                return crow::response(400, "Логин и пароль не могут быть пустыми");
            }

            std::string token = register_user(db_url, username, password);

            crow::json::wvalue res;
            res["token"] = token;
            res["username"] = username;
            return crow::response(200, res);
        } catch (const std::exception& e) {
            return crow::response(400, e.what());
        }
    });

    // Вход в существующий аккаунт
    CROW_ROUTE(app, "/api/login").methods(crow::HTTPMethod::POST)([db_url](const crow::request& req) {
        try {
            auto body = crow::json::load(req.body);
            if (!body || !body.has("username") || !body.has("password")) {
                return crow::response(400, "Ошибка: нужны поля username и password");
            }

            std::string username = body["username"].s();
            std::string password = body["password"].s();

            std::string token = login_user(db_url, username, password);

            crow::json::wvalue res;
            res["token"] = token;
            res["username"] = username;
            return crow::response(200, res);
        } catch (const std::exception& e) {
            return crow::response(400, e.what());
        }
    });

    // Завершение пользовательской сессии: токен становится недействительным в БД
    CROW_ROUTE(app, "/api/logout").methods(crow::HTTPMethod::POST)([db_url](const crow::request& req) {
        try {
            const std::string auth_header = req.get_header_value("Authorization");
            const std::string token = auth_header.rfind("Bearer ", 0) == 0 ? auth_header.substr(7) : "";
            logout_user(db_url, token);
            return crow::response(200, "Выход выполнен");
        } catch (const std::exception& e) {
            return crow::response(400, e.what());
        }
    });

    const char* port_env = std::getenv("PORT");
    int port = (port_env != nullptr) ? std::stoi(port_env) : 18080;
    app.port(port).run();
}