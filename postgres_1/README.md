# postgres_1

 Version: 0.9.1

 date    : 2026/03/24

 update :

***

C++ , postgres example

* postgres DB
* gcc version 14.2.0 


***

### setup

* LIB

```
sudo apt-get update
sudo apt-get install libpq-dev
```

***
* environmental variables
```
export PGHOST=localhost
export PGDATABASE=mydb
export PGUSER=root
export PGPASSWORD=admin
```
***
### build

```
g++ -std=c++17 -I/usr/include/postgresql -Wall -Wextra -o todo main.cpp -lpq
```

***

* add
```
./todo add hello
```

* list
```
./todo list
```

* delete: id-input
```
./todo delete 1
```

***
### Blog

https://zenn.dev/knaka0209/scraps/3e0b001441c06e

