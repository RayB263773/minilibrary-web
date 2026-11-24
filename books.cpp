#include <pqxx/pqxx>
#include <stdexcept>

#include "books.h"

void init_db(const char* db_url) {
    try {
        pqxx::connection conn(db_url);
        pqxx::work tx(conn);
        tx.exec(
            "CREATE TABLE IF NOT EXISTS book ("
            "id SERIAL PRIMARY KEY, "
            "name TEXT NOT NULL, "
            "author TEXT NOT NULL, "
            "year INT NOT NULL, "
            "content TEXT NOT NULL DEFAULT '')"
        );
        // Безопасно добавляем колонку content, если таблица была создана раньше
        tx.exec("ALTER TABLE book ADD COLUMN IF NOT EXISTS content TEXT NOT NULL DEFAULT ''");
        tx.commit();
    } catch (const std::exception& e) {
        throw std::runtime_error(std::string("Ошибка инициализации БД: ") + e.what());
    }
}

std::vector<Book> get_all_books(const char* db_url) {
    std::vector<Book> books;
    try {
        pqxx::connection conn(db_url);
        pqxx::read_transaction tx(conn);
        pqxx::result rows = tx.exec("SELECT id, name, author, year, content FROM book ORDER BY id");
        for (const auto& row : rows) {
            Book book;
            book.id = row["id"].as<int>();
            book.name = row["name"].as<std::string>();
            book.author = row["author"].as<std::string>();
            book.year = row["year"].as<int>();
            book.content = row["content"].as<std::string>();
            books.push_back(std::move(book));
        }
    } catch (const std::exception& e) {
        throw std::runtime_error(std::string("Ошибка при получении книг: ") + e.what());
    }
    return books;
}

std::vector<Book> find_books(const char* db_url, const std::string& search_term) {
    std::vector<Book> books;
    try {
        pqxx::connection conn(db_url);
        pqxx::read_transaction tx(conn);
        pqxx::result rows = tx.exec(
            "SELECT id, name, author, year, content FROM book WHERE name ILIKE $1 OR author ILIKE $1 ORDER BY id",
            pqxx::params("%" + search_term + "%")
        );
        for (const auto& row : rows) {
            Book book;
            book.id = row["id"].as<int>();
            book.name = row["name"].as<std::string>();
            book.author = row["author"].as<std::string>();
            book.year = row["year"].as<int>();
            book.content = row["content"].as<std::string>();
            books.push_back(std::move(book));
        }
    } catch (const std::exception& e) {
        throw std::runtime_error(std::string("Ошибка при поиске книг: ") + e.what());
    }
    return books;
}

void add_book(const char* db_url, const std::string &name, const std::string &author, int year, const std::string &content) {
    try {
        pqxx::connection conn(db_url);
        pqxx::work tx(conn);
        tx.exec(
            "INSERT INTO book (name, author, year, content) VALUES ($1, $2, $3, $4)",
            pqxx::params(name, author, year, content)
        );
        tx.commit();
    } catch (const std::exception& e) {
        throw std::runtime_error(std::string("Ошибка при добавлении книги: ") + e.what());
    }
}

void delete_book(const char* db_url, int id) {
    try {
        pqxx::connection conn(db_url);
        pqxx::work tx(conn);
        tx.exec("DELETE FROM book WHERE id = $1", pqxx::params(id));
        tx.commit();
    } catch (const std::exception& e) {
        throw std::runtime_error(std::string("Ошибка при удалении книги: ") + e.what());
    }
}