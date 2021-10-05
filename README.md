## Lexer

1. Сборка
```
cmake --build cmake-build-debug/                                # обычная
cmake -DCMAKE_BUILD_TYPE=ASAN --build cmake-build-debug_asan/   # с address санитайзером 
cmake -DCMAKE_BUILD_TYPE=UBSAN --build cmake-build-debug_ubsan/ # с undefined behavior санитайзером 
```
2. Запуск тестов
```
cd cmake-build-debug_(asan|ubsan)/
ctest
```