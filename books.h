#ifndef BOOKS_H
#define BOOKS_H

#include <string>
#include <vector>

struct Book {
    int id;
    std::string name;
    std::string author;
    int year;
};

std::vector<Book> get_all_books(const char* db_url);
std::vector<Book> find_books(const char* db_url, const std::string& search_term);

void add_book(const char* db_url, const std::string &name, const std::string &author, int year);
void delete_book(const char* db_url, int id);

#endif // BOOKS_H