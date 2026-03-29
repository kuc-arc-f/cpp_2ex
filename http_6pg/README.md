# http_6pg

 Version: 0.9.1

 date    : 2026/03/29

 update :

***

C++ postgreSQL DB + cpp-httplib  , todo API server

* postgreSQL DB use
* gcc version 14.2.0 

***
### related

* https://github.com/nlohmann/json

* https://github.com/yhirose/cpp-httplib/blob/master/httplib.h

***

### setup

* LIB

```
sudo apt-get update
sudo apt-get install libpq-dev
sudo apt install nlohmann-json3-dev
```

***
### env
```
export DATABASE_URL="host=localhost port=5432 dbname=mydb user=root password=admin"
```

***
* table: schema.sql
```sql
CREATE TABLE IF NOT EXISTS todos (
    id          SERIAL PRIMARY KEY,
    title       VARCHAR(255) NOT NULL,
    description TEXT,
    due_date    DATE,
    priority    SMALLINT DEFAULT 0,   -- 0:low 1:medium 2:high
    done        BOOLEAN NOT NULL DEFAULT FALSE,
    created_at  TIMESTAMPTZ NOT NULL DEFAULT NOW(),
    updated_at  TIMESTAMPTZ NOT NULL DEFAULT NOW()
);
```

***
* build
```
g++ -std=c++17 -I/usr/include/postgresql -pthread main.cpp -o server -lpq
```

***
* start
* localhost:8000 start

```
./server
```

***
### test-data

* add
```
curl -X POST http://localhost:8000/todos \
  -H "Content-Type: application/json" \
  -d '{
    "title": "買い物",
    "description": "牛乳とパン",
    "due_date": "2026-04-01",
    "priority": 1,
    "done": false
  }'

```

* List

```
curl http://localhost:8000/todos
```

* DELETE
```
curl -X DELETE http://localhost:8000/todos/1
```

***
### Blog

https://zenn.dev/knaka0209/scraps/fe734701aad614

