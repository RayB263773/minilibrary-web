# 📚 Mini Library Web

![C++20](https://img.shields.io/badge/C%2B%2B-20-00599C?logo=c%2B%2B&logoColor=white)
![PostgreSQL](https://img.shields.io/badge/PostgreSQL-libpqxx-336791?logo=postgresql&logoColor=white)
![CMake](https://img.shields.io/badge/Build-CMake-064F8C?logo=cmake&logoColor=white)
![Docker](https://img.shields.io/badge/Deploy-Docker-2496ED?logo=docker&logoColor=white)

Клиент-серверное веб-приложение для управления личной библиотекой и постраничного чтения книг онлайн. Серверная часть разработана на **C++20** с использованием HTTP-микрофреймворка **Crow** и реляционной СУБД **PostgreSQL** (`libpqxx`).

🌐 **Демо-версия в облаке:** [https://minilibrary-web.onrender.com](https://minilibrary-web.onrender.com)

## Основные возможности

- **Полный цикл CRUD:** Просмотр каталога, добавление новых книг, редактирование метаданных и текста, удаление записей.
- **Живой поиск (Live Search):** Мгновенная фильтрация книг по названию и автору на стороне PostgreSQL (`ILIKE`) с защитой от лишних запросов (`debounce`).
- **Загрузка книг из `.txt`:** Прикрепление текстовых файлов с автоматическим распознаванием кодировок **UTF-8** и **Windows-1251**.
- **Оптимизированная постраничная читалка:** Ленивая подгрузка текста книги только в момент открытия (`GET /api/books/<id>`), разбивка на страницы без зависаний браузера и автосохранение закладки в `localStorage`.
- **Безопасность и автоинициализация:** Защита всех SQL-запросов от инъекций через `pqxx::params` и автоматическое развёртывание схемы БД (`init_db`) при старте сервера.

## Технологический стек

- **Backend:** C++20, Crow HTTP Framework
- **База данных:** PostgreSQL, драйвер `libpqxx`
- **Сборка и деплой:** CMake, Docker (Arch Linux), Render.com + Neon.tech
- **Frontend:** HTML5, CSS3, Vanilla JavaScript (Fetch API, FileReader, TextDecoder), иконки Icons8

## Маршруты REST API

| Метод | Маршрут | Назначение |
| :--- | :--- | :--- |
| `GET` | `/api/books` | Получение списка всех книг (метаданные без тяжёлого текста) |
| `GET` | `/api/books?q=...` | Поиск книг по названию или автору |
| `GET` | `/api/books/<id>` | Подгрузка полного текста выбранной книги для читалки |
| `POST` | `/api/books` | Добавление новой книги и текста из `.txt` |
| `PUT` | `/api/books/<id>` | Редактирование названия, автора, года и замена `.txt`-файла |
| `DELETE` | `/api/books/<id>` | Удаление книги по её `id` |

## Структура проекта

```text
minilibrary-web/
├── public/
│   ├── index.html       # Пользовательский интерфейс, читалка и клиентская логика (JS)
│   └── style.css        # Тёмная тема оформления, анимации и адаптивная вёрстка
├── books.h              # Структура Book и объявления функций работы с БД
├── books.cpp            # Параметризованные SQL-запросы к PostgreSQL (libpqxx)
├── main.cpp             # HTTP-сервер Crow, маршруты REST API и раздача статики
├── CMakeLists.txt       # Конфигурация сборки CMake + PkgConfig
├── Dockerfile           # Инструкция сборки образа для облачного хостинга
├── .env.example         # Шаблон переменных окружения
└── .gitattributes       # Настройки переносов строк и статистики языков GitHub
```

## Локальный запуск проекта

### 1. Сборка через CMake

Убедитесь, что в системе установлены компилятор с поддержкой C++20, `cmake`, `libpqxx` и заголовки `crow`:

```bash
cmake -B build
cmake --build build
```

### 2. Настройка подключения к PostgreSQL и старт

Задайте переменную окружения `DATABASE_URL` и запустите сервер (таблица `book` создастся автоматически):

```bash
export DATABASE_URL="postgresql://postgres:password@localhost:5432/minilibrary"
./build/minilibrary
```

После запуска откройте в браузере: `http://localhost:18080`

### 3. Запуск в Docker

```bash
docker build -t minilibrary-web .
docker run -p 18080:18080 -e DATABASE_URL="postgresql://..." minilibrary-web
```