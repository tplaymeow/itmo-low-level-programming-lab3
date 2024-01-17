### [Отчет GitHub](https://github.com/tplaymeow/itmo-low-level-programming-lab3/blob/main/Report.pdf)
### [Отчет GitLab](https://gitlab.se.ifmo.ru/tplaymeow/low-level-programming-lab3/-/blob/main/Report.pdf)

## Зависимости
- `Bison 3.8.2`
- `Flex 2.6.4`
- `cJSON 1.7.17`

## Сборка и запуск
```
git clone git@github.com:tplaymeow/itmo-low-level-programming-lab3.git
git submodule init
git submodule update
```

Для сборки и запуска необходимо использовать Сmake.

Серверная программа
```
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
build/server_app/server_app file.db 2002
```

Клиентская программа
```
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
build/client_app/client_app 127.0.0.1 2002
```
