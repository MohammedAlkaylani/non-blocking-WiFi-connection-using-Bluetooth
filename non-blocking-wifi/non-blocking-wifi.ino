#include <WiFi.h>
#include <Preferences.h>
#include <ArduinoJson.h>
#include <BluetoothSerial.h>

#define BUTTON_PIN 16

Preferences preferences;
BluetoothSerial SerialBT;

/*WIFI millis*/
static unsigned long connectingTimeOut = 30000;
static unsigned long connectingTimeOutDelay;
static unsigned long lastDotDelay;
static unsigned long lastTimeOutDelay;
static unsigned long OldlastTimeOutDelay;

enum WifiState {
  START_LOGIN,
  DISCONNECTED,
  RESETED,
  OPEN_BLUETOOTH,
  NEW_CONNECTING,
  NEW_CONNECTED,
  OLD_CONNECTING,
  OLD_CONNECTED,
  LOOP,
  OLD_TIME_OUT
};

typedef struct {
  String ssid;
  String password;
  WifiState state;
} WifiStructType;

WifiStructType wifiStruct;

void wifiLoop() {
  switch (wifiStruct.state) {
    case START_LOGIN:
      preferences.begin("wifi-creds", false);
      wifiStruct.ssid = preferences.getString("ssid", "");
      wifiStruct.password = preferences.getString("password", "");
      preferences.end();
      if (wifiStruct.ssid.isEmpty()) {
        SerialBT.begin("MY ESP");
        Serial.println("No WiFi credentials found.");
        Serial.println("Bluetooth started");
        wifiStruct.state = OPEN_BLUETOOTH;
      } else {
        Serial.println("OLD_CONNECTING");
        WiFi.begin(wifiStruct.ssid.c_str(), wifiStruct.password.c_str());
        wifiStruct.state = OLD_CONNECTING;
        connectingTimeOutDelay = millis();
      }
      break;

    case DISCONNECTED:
      WiFi.begin(wifiStruct.ssid.c_str(), wifiStruct.password.c_str());
      wifiStruct.state = OLD_CONNECTING;
      break;

    case RESETED:
      Serial.println("Button pressed. Clearing WiFi credentials.");
      preferences.begin("wifi-creds", false);
      preferences.remove("ssid");
      preferences.remove("password");
      preferences.end();
      WiFi.disconnect(true);
      Serial.println("Bluetooth started");
      SerialBT.begin("MY ESP");
      wifiStruct.state = OPEN_BLUETOOTH;
      break;

    case OPEN_BLUETOOTH:
      if (SerialBT.available()) {
        String receivedData = SerialBT.readStringUntil('\n');
        Serial.print("Received JSON: ");
        Serial.println(receivedData);
        receivedData.trim();

        // Parse JSON
        DynamicJsonDocument doc(256);  // Adjust size as needed
        DeserializationError error = deserializeJson(doc, receivedData);

        if (error) {
          Serial.print("JSON parse failed: ");
          Serial.println(error.c_str());
          break;
        }
        // Extract values
        const char* newSSID = doc["ssid"];
        const char* newPassword = doc["password"];

        if (newSSID && newPassword) {
          Serial.println("Parsed values:");
          Serial.printf("SSID: %s\n", newSSID);
          Serial.printf("Password: %s\n", newPassword);
          wifiStruct.ssid = newSSID;
          wifiStruct.password = newPassword;
          WiFi.begin(wifiStruct.ssid.c_str(), wifiStruct.password.c_str());
          Serial.println("Connecting to WiFi...");
          wifiStruct.state = NEW_CONNECTING;
          connectingTimeOutDelay = millis();
        }
      }

      break;

    case NEW_CONNECTING:
      if (WiFi.status() != WL_CONNECTED) {
        if (millis() - lastDotDelay > 500) {
          lastDotDelay = millis();
          Serial.print(".");

          if (millis() - connectingTimeOutDelay > connectingTimeOut) {
            connectingTimeOutDelay = millis();
            Serial.println("Time Out");
            /*Here I want send to the mobile app*/
            SerialBT.println("Time Out: Could not connect to WiFi");
            wifiStruct.state = OPEN_BLUETOOTH;
          }
        }
      } else {
        wifiStruct.state = NEW_CONNECTED;
      }
      break;

    case NEW_CONNECTED:
      preferences.begin("wifi-creds", false);
      preferences.putString("ssid", wifiStruct.ssid);
      preferences.putString("password", wifiStruct.password);
      preferences.end();
      SerialBT.end();
      Serial.println("\nWiFi Connected!");
      Serial.print("IP Address: ");
      Serial.println(WiFi.localIP());
      wifiStruct.state = LOOP;
      break;

    case OLD_CONNECTING:
      if (WiFi.status() != WL_CONNECTED) {
        if (millis() - lastDotDelay > 500) {
          lastDotDelay = millis();
          Serial.print(".");
          if (millis() - connectingTimeOutDelay > connectingTimeOut) {
            connectingTimeOutDelay = millis();
            Serial.println("Time Out");
            wifiStruct.state = OLD_TIME_OUT;
            OldlastTimeOutDelay = millis();
          }
        }
      } else {
        wifiStruct.state = OLD_CONNECTED;
      }
      break;

    case OLD_CONNECTED:
      Serial.println("\nWiFi Connected!");
      Serial.print("IP Address: ");
      Serial.println(WiFi.localIP());
      wifiStruct.state = LOOP;
      break;

    case OLD_TIME_OUT:
      if (millis() - OldlastTimeOutDelay > connectingTimeOut) {
        OldlastTimeOutDelay = millis();
        wifiStruct.state = OLD_CONNECTING;
        connectingTimeOutDelay = millis();
      }
      break;

    case LOOP:
      if (WiFi.status() == WL_DISCONNECTED) {
        wifiStruct.state = DISCONNECTED;
      }
      break;
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  wifiStruct.state = START_LOGIN;

}

void loop() {
    wifiLoop();

  if (digitalRead(BUTTON_PIN) == LOW) {
    wifiStruct.state = RESETED;
    delay(300);  // debounce
  }

}
