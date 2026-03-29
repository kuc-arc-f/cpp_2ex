# rag_6pg

 Version: 0.9.1

 date    : 2026/03/27

 update :

***

C++ API Server , RAG Search  PGVector DB

* modl: Qwen3.5-2B-Q4_K_S.gguf
* Embedding-model : Qwen3-Embedding-0.6B-Q8_0.gguf
* llama.cpp , llama-server 
* gcc version 14.2.0 

***

### setup


* llama-server start
* port 8080: Qwen3-Embedding-0.6B
* port 8090: Qwen3.5-2B

```
#Qwen3-Embedding-0.6B
/home/user123/llama-server -m /var/lm_data/Qwen3-Embedding-0.6B-Q8_0.gguf --embedding  -c 1024 --port 8080

#Qwen3.5-2B
/home/user123/llama-server -m /var/lm_data/unsloth/Qwen3.5-2B-GGUF/Qwen3.5-2B-Q4_K_S.gguf \
 --chat-template-kwargs '{"enable_thinking": false}' --port 8090 

```

***
### related

https://huggingface.co/unsloth/Qwen3.5-2B-GGUF

https://huggingface.co/Qwen/Qwen3-Embedding-0.6B-GGUF

***
* LIB

```
sudo apt install libpq-dev libpqxx-dev
sudo apt install nlohmann-json3-dev
sudo apt install libcurl4-openssl-dev
```

***
* build
```
g++ -std=c++17 -pthread -o server server.cpp -lcurl -lpqxx -lpq
```

* start
* localhost:8000 , start

```
./server
```

***
### test-code
* node.js
* ./test_post.js

```
node ./test_post.js
```

```js
// 送信するデータ
const data = {
  input: "二十四節気",
};

const API_URL= "http://localhost:8000/search"

async function postData() {
  console.log("#start-post")  
  try {
    const response = await fetch(API_URL , {
      method: 'POST', // メソッドを指定
      headers: {
        'Content-Type': 'application/json', // JSONを送ることを伝える
      },
      body: JSON.stringify(data), // オブジェクトを文字列に変換
    });

    // 成功したかチェック（ステータスコード 200-299）
    if (!response.ok) {
      throw new Error(`HTTP error! status: ${response.status}`);
    }

    const result = await response.json(); // レスポンスを解析
    console.log('成功:');
    console.log(result.text);
    
  } catch (error) {
    console.error('エラー発生:', error);
  }
}

postData();

```
***
* curl POSt
```
curl -X POST http://localhost:8000/search \
     -H "Content-Type: application/json" \
     -d '{"input": "二十四節気"}'

```


***
### Blog

https://zenn.dev/knaka0209/scraps/2b365f750cc0a7

