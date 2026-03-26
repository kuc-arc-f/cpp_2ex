# qdrant_1

 Version: 0.9.1

 date    : 2026/03/24

 update :

***

C++ , Qdrant example

* Qdrant
* gcc version 14.2.0 

***

### setup

* LIB

```
sudo apt install libcurl4-openssl-dev
sudo apt install nlohmann-json3-dev
```

***
### build
* build

```
g++ -std=c++17 -o init init.cpp -lcurl
g++ -std=c++17 -o embed embed.cpp -lcurl
g++ -std=c++17 -o search search.cpp -lcurl
```

***
* start
```
./init
./embed
./search
```

***
### Blog

