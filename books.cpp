#include <pqxx/pqxx>
#include <stdexcept>

#include "books.h"

std::vector<Book> get_all_books(const char* db_url) {
    std::vector<Book> books;
    try {
        pqxx::connection conn(db_url);
        pqxx::read_transaction tx(conn);
        pqxx::result rows = tx.exec("SELECT id, name, author, year FROM book ORDER BY id");
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

void add_book(const char* db_url, const std::string &name, const std::string &author, int year) {
    try {
        pqxx::connection conn(db_url);
        pqxx::work tx(conn);
        tx.exec("INSERT INTO book (name, author, year) VALUES ($1, $2, $3)", pqxx::params(name, author, year));
        tx.commit();
    } catch (const std::exception& e) {
        throw std::runtime_error(std::string("Ошибка при добавлении книги: ") + e.what());
    }
}