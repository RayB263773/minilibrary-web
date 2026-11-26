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

void init_db(const char* db_url);

std::vector<Book> get_all_books(const char* db_url, int user_id = -1);
std::vector<Book> find_books(const char* db_url, const std::string& search_term, int user_id = -1);
std::string get_book_content(const char* db_url, int id, int user_id = -1);

void add_book(const char* db_url, const std::string& name, const std::string& author, int year, const std::string& content = "", int user_id = -1);
void update_book(const char* db_url, int id, const std::string& name, const std::string& author, int year, const std::string& content, int user_id = -1);
void delete_book(const char* db_url, int id, int user_id = -1);

std::string register_user(const char* db_url, const std::string& username, const std::string& password);
std::string login_user(const char* db_url, const std::string& username, const std::string& password);
int get_user_id_by_token(const char* db_url, const std::string& token);
void logout_user(const char* db_url, const std::string& token);

#endif // BOOKS_H