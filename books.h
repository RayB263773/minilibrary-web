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
#endif // BOOKS_H