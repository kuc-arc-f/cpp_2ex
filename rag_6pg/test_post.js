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
