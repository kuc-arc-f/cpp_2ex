# rag_3

 Version: 0.9.1

 date    : 2026/03/23

 update :

***

C++ RAG Search, Qdrant DB + llama.cpp

* llama.cpp , llama-server 
* Embedding-model : Qwen3-Embedding-0.6B-Q8_0.gguf
* gcc version 14.2.0 

***

### setup

* LIB

```
sudo apt install libcurl4-openssl-dev
sudo apt install nlohmann-json3-dev
```

* llama-server start
```
/home/user123/llama-server -m /var/lm_data/Qwen3-Embedding-0.6B-Q8_0.gguf --embedding  -c 1024 --port 8080
```

***
* build
```
g++ -std=c++17 -o init init.cpp -lcurl
g++ -std=c++17 -o embed embed.cpp -lcurl -luuid
g++ -std=c++17 -o search search.cpp -lcurl -luuid
```

***
* init-DB
```
./init
```

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

https://zenn.dev/knaka0209/scraps/48712a328b6a96
