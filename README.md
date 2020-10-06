# call45aggr


# Установка зависимостей conan
Для сборки необходимо установить conan.io через pip
pip install conan

Возможно придется обновить pip для этого

Первичная установка всех зависимостей происходит единожды следующим образом

conan remote add conan-transit https://api.bintray.com/conan/conan/conan-transit
conan install . -pr profile --build bzip2 --build zlib  --build jsoncpp --build boost

profile - файл с профилем текущей системы. Должен храниться в каком-нибудь постоянном месте (/root/conan_profile). 

Пример файла профиля сборки
```
[settings]
os_build=Linux
arch_build=x86_64
os=Linux
arch=x86_64
compiler=gcc
compiler.version=7
compiler.libcxx=libstdc++
```
