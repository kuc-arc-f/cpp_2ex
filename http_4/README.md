# http_4

 Version: 0.9.1

 date    : 2026/03/20

 update :

***

C++ React + cpp-httplib , todo server

* SQLite DB
* gcc version 14.2.0 
* node 22

***
### related

* https://github.com/nlohmann/json

* https://github.com/yhirose/cpp-httplib/blob/master/httplib.h

***

### setup

* LIB

```
sudo apt install nlohmann-json3-dev
sudo apt-get install libsqlite3-dev
```
***
* front-build

```
npm i
npm run build
```

***
* build
```
make all
```

* start
```
./server
```

***
* test-data

* add
```
curl -X POST http://localhost:8000/todos \
     -H "Content-Type: application/json" \
     -d '{"title": "tit-21"}'
```

* DELETE
```
curl -X DELETE http://localhost:8000/todos/3
```

* list
```
curl http://localhost:8000/todos
```

***
### Blog

https://zenn.dev/knaka0209/scraps/5316fbd0947f74

