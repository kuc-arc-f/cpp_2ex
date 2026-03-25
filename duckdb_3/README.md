# duckdb_3

 Version: 0.9.1

 date    : 2026/03/24

 update :

***

C++ , duckdb example

* duckdb
* gcc version 14.2.0 

***
### related

* https://github.com/duckdb/duckdb


***

### setup

* LIB

```
wget https://github.com/duckdb/duckdb/releases/latest/download/libduckdb-linux-amd64.zip
unzip libduckdb-linux-amd64.zip -d /usr/local/lib/duckdb
```

* LD_LIBRARY_PATH add 
```
export LD_LIBRARY_PATH=/usr/local/lib/duckdb
```

* DB add
```
duckdb todos.db < ./table.sql
```
***
### build
* create

```

g++ -std=c++17 -I/usr/local/lib/duckdb \
    -L/usr/local/lib/duckdb \
    create.cpp -lduckdb -o create
```

* delete 
```
g++ -std=c++17 -I/usr/local/lib/duckdb \
    -L/usr/local/lib/duckdb \
    delete.cpp -lduckdb -o delete
```

* list
```
g++ -std=c++17 -I/usr/local/lib/duckdb \
    -L/usr/local/lib/duckdb \
    list.cpp -lduckdb -o list

```

***
* create
```
./create hello
```

* delete, id input
```
./delete 1
```

* list
```
./list
```

***
### Blog

