# discord_1

 Version: 0.9.1

 date    : 2026/03/24

 update :

***

C++ , discord example

* gcc version 14.2.0 


***

### setup

* LIB add

```
sudo apt install libcurl4-openssl-dev   # Ubuntu/Debian
sudo apt install nlohmann-json3-dev
```

***
* main.cpp
* WEBHOOK_URL change
```
const std::string WEBHOOK_URL = "https://discord.com/api/webhooks/123/456";
```
***
### build

```
g++ -std=c++17 -o discord_post main.cpp -lcurl
```
***
* start
```
./discord_post hello
```

***
### Blog

