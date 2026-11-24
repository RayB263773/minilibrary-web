cmake_minimum_required(VERSION 3.15)
project(minilibrary CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Ищем pthread и библиотеки PostgreSQL через системный pkg-config
find_package(Threads REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(PQXX REQUIRED IMPORTED_TARGET libpqxx libpq)

add_executable(minilibrary
    main.cpp
    books.cpp
)

target_link_libraries(minilibrary PRIVATE
    PkgConfig::PQXX
    Threads::Threads
)