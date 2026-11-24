FROM archlinux:latest

# Устанавливаем компилятор, CMake, libpqxx и зависимости для Crow
RUN pacman -Syu --noconfirm base-devel cmake pkgconf libpqxx asio git && \
    git clone --depth 1 https://github.com/CrowCpp/Crow.git /tmp/crow && \
    cp -r /tmp/crow/include/* /usr/include/ && \
    rm -rf /tmp/crow && \
    pacman -Scc --noconfirm

WORKDIR /app

# Копируем файлы проекта и собираем сервер через CMake
COPY . .
RUN cmake -B build && cmake --build build

# Запускаем скомпилированный сервер
CMD ["./build/minilibrary"]