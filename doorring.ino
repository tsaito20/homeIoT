#include <M5Stack.h>  // M5Stackライブラリのインクルード
#include <WiFi.h>
#include <HTTPClient.h>

const char* ssid = "ssid";            // Wi-FiのSSID
const char* password = "pass";     // Wi-Fiのパスワード
const String lineToken = "Token"; // LINEチャネルアクセストークン

const int doorbellPin = 2;  // ドアホンのA接点出力を接続するGPIOピン番号
int previousState = HIGH;

void setup() {
  M5.begin();           // M5Stackの初期化
  M5.Power.begin();
  if(!M5.Power.canControl()) {
    //can't control.
    M5.Lcd.print("NG");
    return;
  }
  M5.Lcd.clear();       // LCD画面のクリア
  M5.Lcd.setTextSize(2); // テキストサイズの設定
  Serial.begin(115200);
  pinMode(doorbellPin, INPUT_PULLUP);

  // Wi-Fi接続
  WiFi.begin(ssid, password);
  M5.Lcd.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    M5.Lcd.print(".");
  }
  Serial.println("\nWiFi Connected");
  M5.Lcd.clear();
  M5.Lcd.setCursor(0, 0);
  M5.Lcd.print("Waiting for Doorbell...");
}

void loop() {
  int currentState = digitalRead(doorbellPin);

  // ドアホンが鳴ったとき（A接点がONになったとき）通知を送信しLCDに表示
  if (currentState == LOW && previousState == HIGH) {
    M5.Lcd.clear();
    M5.Lcd.setCursor(0, 0);
    M5.Lcd.print("Doorbell is ringing!");
    sendLineBroadcast("ドアホンが鳴りました！");
    delay(5000); // 過剰な通知を防ぐために少し待機
  } else if (currentState == HIGH && previousState == LOW) {
    M5.Lcd.clear();
    M5.Lcd.setCursor(0, 0);
    M5.Lcd.print("Waiting for Doorbell...");
  }

  previousState = currentState;

/*
  // Aボタンが押されたときにLINE通知をテスト送信
  if (M5.BtnA.wasPressed()) {
    M5.Lcd.clear();
    M5.Lcd.setCursor(0, 0);
    M5.Lcd.print("Sending test notification...");
    sendLineBroadcast("テスト通知: ボタンAが押されました");
  }
*/

  // バッテリーと充電状況を表示
  displayBatteryStatus();

  M5.update();  // ボタン状態の更新

  delay(100); // 短い待機時間で状態チェック
}

void sendLineBroadcast(String message) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin("https://api.line.me/v2/bot/message/broadcast");
    http.addHeader("Content-Type", "application/json");
    http.addHeader("Authorization", "Bearer " + lineToken);

    String jsonPayload = "{\"messages\":[{\"type\":\"text\",\"text\":\"" + message + "\"}]}";

    int httpResponseCode = http.POST(jsonPayload);

    if (httpResponseCode > 0) {
      Serial.printf("Message broadcasted: %d\n", httpResponseCode);
    } else {
      Serial.printf("Error broadcasting message: %s\n", http.errorToString(httpResponseCode).c_str());
    }
    
    http.end();
  } else {
    Serial.println("WiFi not connected");
  }
}

void displayBatteryStatus() {
  // バッテリー残量と充電状態の取得
  int batteryLevel = M5.Power.getBatteryLevel();      // バッテリー残量を取得
  bool isCharging = M5.Power.isCharging();            // 充電中かどうかを取得

  // バッテリー情報の表示
  M5.Lcd.setCursor(0, 50);
  M5.Lcd.printf("Battery: %d%%", batteryLevel);
  M5.Lcd.setCursor(0, 70);
  M5.Lcd.print(isCharging ? "Charging..." : "Not Charging");
}
