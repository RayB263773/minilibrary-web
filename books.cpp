#include <pqxx/pqxx>
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