# Используем официальный образ GCC как базовый
FROM gcc:latest

# Устанавливаем дополнительные утилиты (по желанию)
RUN apt-get update && apt-get install -y \
    cmake \
    make \
    gdb \
    && rm -rf /var/lib/apt/lists/*

# Устанавливаем рабочую директорию внутри контейнера
WORKDIR /workspace

# Копируем исходный код
COPY . .

# Собираем программу (предполагается наличие Makefile или CMakeLists.txt)
# Пример с g++ напрямую для одного файла:
# RUN g++ -std=c++17 -O2 -Wall -o main main.cpp

# Или с использованием Makefile:
# RUN make

# Или с использованием CMake:
# RUN mkdir build && cd build && cmake .. && make

# Запускаем исполняемый файл
CMD ["./main"]