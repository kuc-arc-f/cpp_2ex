# rag_1

 Version: 0.9.1

 date    : 2026/03/23

 update :

***

C++ RAG Search, pgvector + llama.cpp

* llama.cpp , llama-server 
* Embedding-model : Qwen3-Embedding-0.6B-Q8_0.gguf
* gcc version 14.2.0 

***
### related

* https://github.com/nlohmann/json


***

### setup

* LIB

```
sudo apt install libcurl4-openssl-dev
sudo apt install libpq-dev libpqxx-dev
sudo apt install nlohmann-json3-dev
```

* llama-server start
```
/home/user123/llama-server -m /var/lm_data/Qwen3-Embedding-0.6B-Q8_0.gguf --embedding  -c 1024 --port 8080
```

***
* build
```
g++ -std=c++17 -o search search.cpp -lcurl -lpqxx -lpq
g++ -std=c++17 -o embed embed.cpp -lcurl -lpqxx -lpq
```

***

* vector data add
```
./embed ./data
```

* search
```
./search hello
```


***
### Blog

