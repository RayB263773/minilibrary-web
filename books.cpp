#include <pqxx/pqxx>
#include <stdexcept>
#include <functional>
#include <random>

#include "books.h"

// DB WORK FUNCTIONS

void init_db(const char* db_url) {
    try {
        pqxx::connection conn(db_url);
        pqxx::work tx(conn);
        tx.exec(
            "CREATE TABLE IF NOT EXISTS users ("
            "id SERIAL PRIMARY KEY, "
            "username TEXT UNIQUE NOT NULL, "
            "password_hash TEXT NOT NULL, "
            "token TEXT)"
        );
        tx.exec(
            "CREATE TABLE IF NOT EXISTS book ("
            "id SERIAL PRIMARY KEY, "
            "name TEXT NOT NULL, "
            "author TEXT NOT NULL, "
            "year INT NOT NULL, "
            "content TEXT NOT NULL DEFAULT '')"
        );
        tx.exec("ALTER TABLE book ADD COLUMN IF NOT EXISTS content TEXT NOT NULL DEFAULT ''");
        tx.exec("ALTER TABLE book ADD COLUMN IF NOT EXISTS user_id INT REFERENCES users(id) ON DELETE CASCADE");
        tx.commit();
    } catch (const std::exception& e) {
        throw std::runtime_error(std::string("Ошибка инициализации БД: ") + e.what());
    }
}

std::vector<Book> get_all_books(const char* db_url, int user_id) {
    std::vector<Book> books;
    try {
        pqxx::connection conn(db_url);
        pqxx::read_transaction tx(conn);
        pqxx::result rows;

        if (user_id <= 0) {
            rows = tx.exec("SELECT id, name, author, year FROM book WHERE user_id IS NULL ORDER BY id");
        } else {
            rows = tx.exec(
                "SELECT id, name, author, year FROM book WHERE user_id = $1 ORDER BY id",
                pqxx::params(user_id)
            );
        }

        for (const auto& row : rows) {
            Book book;
            book.id = row["id"].as<int>();
            book.name = row["name"].as<std::string>();
            book.author = row["author"].as<std::string>();
            book.year = row["year"].as<int>();
            books.push_back(std::move(book));
        }
    } catch (const std::exception& e) {
        throw std::runtime_error(std::string("Ошибка при получении книг: ") + e.what());
    }
    return books;
}

std::vector<Book> find_books(const char* db_url, const std::string& search_term, int user_id) {
    std::vector<Book> books;
    try {
        pqxx::connection conn(db_url);
        pqxx::read_transaction tx(conn);
        pqxx::result rows;

        if (user_id <= 0) {
            rows = tx.exec(
                "SELECT id, name, author, year FROM book "
                "WHERE (name ILIKE $1 OR author ILIKE $1) AND user_id IS NULL ORDER BY id",
                pqxx::params("%" + search_term + "%")
            );
        } else {
            rows = tx.exec(
                "SELECT id, name, author, year FROM book "
                "WHERE (name ILIKE $1 OR author ILIKE $1) AND user_id = $2 ORDER BY id",
                pqxx::params("%" + search_term + "%", user_id)
            );
        }

        for (const auto& row : rows) {
            Book book;
            book.id = row["id"].as<int>();
            book.name = row["name"].as<std::string>();
            book.author = row["author"].as<std::string>();
            book.year = row["year"].as<int>();
            books.push_back(std::move(book));
        }
    } catch (const std::exception& e) {
        throw std::runtime_error(std::string("Ошибка при поиске книг: ") + e.what());
    }
    return books;
}

std::string get_book_content(const char* db_url, int id, int user_id) {
    try {
        pqxx::connection conn(db_url);
        pqxx::read_transaction tx(conn);
        pqxx::result rows;
        if (user_id <= 0) {
            rows = tx.exec(
                "SELECT content FROM book WHERE id = $1 AND user_id IS NULL",
                pqxx::params(id)
            );
        } else {
            rows = tx.exec(
                "SELECT content FROM book WHERE id = $1 AND user_id = $2",
                pqxx::params(id, user_id)
            );
        }
        if (rows.empty()) {
            return "";
        }
        return rows[0]["content"].as<std::string>();
    } catch (const std::exception& e) {
        throw std::runtime_error(std::string("Ошибка при загрузке текста книги: ") + e.what());
    }
}

void add_book(const char* db_url, const std::string &name, const std::string &author, int year, const std::string &content, int user_id) {
    try {
        pqxx::connection conn(db_url);
        pqxx::work tx(conn);
        if (user_id <= 0){tx.exec(
            "INSERT INTO book (name, author, year, content) VALUES ($1, $2, $3, $4)",
            pqxx::params(name, author, year, content)
            );}
        else {
            tx.exec(
            "INSERT INTO book (name, author, year, content, user_id) VALUES ($1, $2, $3, $4, $5)",
            pqxx::params(name, author, year, content, user_id)
            );
        }
        tx.commit();
    } catch (const std::exception& e) {
        throw std::runtime_error(std::string("Ошибка при добавлении книги: ") + e.what());
    }
}

void delete_book(const char* db_url, int id, int user_id) {
    try {
        pqxx::connection conn(db_url);
        pqxx::work tx(conn);
        tx.exec(
        "DELETE FROM book WHERE id = $1 AND (user_id IS NULL OR user_id = $2)",
        pqxx::params(id, user_id)
        );
        tx.commit();
    } catch (const std::exception& e) {
        throw std::runtime_error(std::string("Ошибка при удалении книги: ") + e.what());
    }
}

void update_book(const char* db_url, int id, const std::string& name, const std::string& author, int year, const std::string& content, int user_id) {
    try {
        pqxx::connection conn(db_url);
        pqxx::work tx(conn);
        if (content.empty()) {
            tx.exec(
                "UPDATE book SET name = $1, author = $2, year = $3 WHERE id = $4 AND (user_id IS NULL OR user_id = $5)",
                pqxx::params(name, author, year, id, user_id)
            );
        } else {
            tx.exec(
                "UPDATE book SET name = $1, author = $2, year = $3, content = $4 WHERE id = $5 AND (user_id IS NULL OR user_id = $6)",
                pqxx::params(name, author, year, content, id, user_id)
            );
        }
        tx.commit(); // Фиксируем изменения в базе данных
    } catch (const std::exception& e) {
        throw std::runtime_error(std::string("Ошибка при редактировании книги: ") + e.what());
    }
}
// ACCOUNT FUNCTIONS
std::string hash_password(const std::string& password) {
    std::size_t h = std::hash<std::string>{}(password + "minilib_secret_salt");
    return std::to_string(h);
}

std::string generate_token() {
    // 1. Задаём алфавит, из которого будем собирать токен
    const std::string chars = "0123456789abcdef";

    // 2. Настраиваем генератор случайных индексов от 0 до 15
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(0, chars.size() - 1);

    // 3. Создаём пустую строку для будущего токена
    std::string token;

    for (int i = 0; i < 32; i++) { token += chars[dist(gen)];}
    return token;
}

std::string register_user(const char* db_url, const std::string &username, const std::string &password){
    std::string hashed = hash_password(password);
    std::string token = generate_token();

    try {
        pqxx::connection conn(db_url);
        pqxx::work tx(conn);
        tx.exec(
        "INSERT INTO users (username, password_hash, token) VALUES ($1, $2, $3)",
        pqxx::params(username, hashed, token)
        );
        tx.commit();
        return token;
    } catch (const std::exception& e) { throw std::runtime_error(std::string("Имя пользователя уже занято!") + e.what()); }
}

std::string login_user(const char* db_url, const std::string& username, const std::string& password){
    std::string hashed = hash_password(password);
    std::string token = generate_token();

    pqxx::connection conn(db_url);
    pqxx::work tx(conn);

    pqxx::result res = tx.exec(
    "SELECT id FROM users WHERE username = $1 AND password_hash = $2",
    pqxx::params(username, hashed)
    );
    if (res.empty()){ throw std::runtime_error("Неверный логин или пароль"); }
    else {
        tx.exec(
        "UPDATE users SET token = $1 WHERE username = $2",
        pqxx::params(token, username)
        );
    }
    tx.commit();
    return token;
}

int get_user_id_by_token(const char* db_url, const std::string& token){
    if (token.empty()) {return -1; }

    try {
        pqxx::connection conn(db_url);
        pqxx::nontransaction tx(conn);

        pqxx::result res = tx.exec(
        "SELECT id FROM users WHERE token = $1",
        pqxx::params(token)
        );

        if (res.empty()) {return -1; }
        return res[0]["id"].as<int>();
    } catch(const std::exception& e) {return -1; }
}

void logout_user(const char* db_url, const std::string& token) {
    if (token.empty()) return;
    try {
        pqxx::connection conn(db_url);
        pqxx::work tx(conn);
        tx.exec("UPDATE users SET token = NULL WHERE token = $1", pqxx::params(token));
        tx.commit();
    } catch (const std::exception& e) {
        throw std::runtime_error(std::string("Ошибка при выходе из аккаунта: ") + e.what());
    }
}
