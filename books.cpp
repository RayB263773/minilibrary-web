#include <pqxx/pqxx>
#include <stdexcept>

#include "books.h"
// Функция для получения всех книг из базы данных
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
std::vector<Book> find_books(const char* db_url, const std::string& search_term){
    std::vector<Book> books;
    try {
        pqxx::connection conn(db_url);
        pqxx::read_transaction tx(conn);
        pqxx::result rows = tx.exec(
    "SELECT id, name, author, year FROM book WHERE name ILIKE $1 OR author ILIKE $1 ORDER BY id",
    pqxx::params("%" + search_term + "%")
);
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
// Функция для добавления книги в базу данных
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
// Функция для удаления книги из базы данных
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