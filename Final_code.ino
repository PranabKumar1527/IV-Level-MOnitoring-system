#include <Arduino.h>
#include "HX711.h"
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>  // Universal Telegram Bot Library written by Brian Lough: https://github.com/witnessmenow/Universal-Arduino-Telegram-Bot
#include <ArduinoJson.h>

const char* ssid = "POCO X2";
const char* password = "Ankit@0987";
#define BOTtoken "5659692013:AAHNxJKHop0JtaZmew6HurhCWJaRhGNxT-c"
#define CHAT_ID "2029208249"

#ifdef ESP8266
X509List cert(TELEGRAM_CERTIFICATE_ROOT);
#endif

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);
unsigned long lastTimeBotRan;
// HX711 circuit wiring
const int LOADCELL_DOUT_PIN = 12;
const int LOADCELL_SCK_PIN = 13;

HX711 scale;
String getReadings() {
  float weight;
  weight = scale.get_units();
  String message = "Weight: " + String(weight) + "  gm\n";
  // message += "Humidity: " + String (humidity) + " % \n";
  return message;
}

void handleNewMessages(int numNewMessages) {
  Serial.println("handleNewMessages");
  Serial.println(String(numNewMessages));

  for (int i = 0; i < numNewMessages; i++) {
    // Chat id of the requester
    String chat_id = String(bot.messages[i].chat_id);
    if (chat_id != CHAT_ID) {
      bot.sendMessage(chat_id, "Unauthorized user", "");
      continue;
    }

    // Print the received message
    String text = bot.messages[i].text;
    Serial.println(text);

    String from_name = bot.messages[i].from_name;

    if (text == "/start") {
      String welcome = "Welcome, " + from_name + ".\n";
      welcome += "Use the following commands to control your outputs.\n\n";
      welcome += "/weight of the saline present \n";
      bot.sendMessage(chat_id, welcome, "");
    }

    if (text == "/weight") {
      String weight = getReadings();
      bot.sendMessage(chat_id, weight, "");
    }

  }
}
void setup() {
  Serial.begin(115200);
  configTime(0, 0, "pool.ntp.org");  // get UTC time via NTP
  client.setTrustAnchors(&cert);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);


//#ifdef ESP8266
  //client.setCACert(TELEGRAM_CERTIFICATE_ROOT);  // Add root certificate for api.telegram.org
//#endif

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
  // Print ESP32 Local IP Address
  Serial.println(WiFi.localIP());
  Serial.println("Initializing the scale");

  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);

  scale.set_scale(327.686);
  //scale.set_scale(-471.497);                      // this value is obtained by calibrating the scale with known weights; see the README for details
  scale.tare();  // reset the scale to 0

  Serial.println("Readings:");
}

void loop() {
  Serial.print("one reading:\t");
  Serial.print(scale.get_units(), 1);
  Serial.print("\t| average:\t");
  Serial.println(scale.get_units(10), 5);

  int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
  float weight = scale.get_units();

  while (numNewMessages) {
    Serial.println("got response");
    handleNewMessages(numNewMessages);
    numNewMessages = bot.getUpdates(bot.last_message_received + 1);
  }
  if (weight <= 150) {
    bot.sendMessage(CHAT_ID, "Alert!! Saline Level <=150ml", "");
    Serial.println("Saline Level <=150ml");
    // motionDetected = false;
  }
  
  else {
    bot.sendMessage(CHAT_ID, "Saline is Filled", "");
    Serial.println("Saline is Filled");
 
}

  // scale.power_down();  // put the ADC in sleep mode
  // // delay(5000);
  // scale.power_up();
}
