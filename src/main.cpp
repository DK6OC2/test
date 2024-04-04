/*
******************************************************************************************************
* Newsbox-Projekt des OVV R01 und R04    *
* Version 2.0                            *                               
******************************************************************************************************
*/
#include <Arduino.h>
#include <globals.h>
#include <FS.h>         // enable Filesystem
#include <SPIFFS.h>     // enable SPI Flash FileSystem
#include <WiFi.h>
#include <WiFiMulti.h>
#include <WiFiClientSecure.h>
//#include <WiFiManager.h>
#include <Wire.h>
#include <HTTPClient.h>
#include <config.h>
#include <ArduinoJson.h>

//Wähle Includefile für Board und Display
#include <boards/az-delivery-devkit-v4.h>
#include <displays/epaper29_bw.h>

WiFiMulti wifiMulti;
JsonDocument doc; //JSON Opject
// Debug Switch - auf true um Ausgaben auf die Konsole zu bekommen
boolean __DEBUG = true; 
WiFiClientSecure *client = new WiFiClientSecure ; // initialisieren des WifiClients mit SSL
bool new_wifi = true ; // Flag für neue Wifi-Verbindung nach Verbindungsverlust

void setup()
{
  // Start Serial
  Serial.begin(9600);
  Serial.setTimeout(1000); // miliseconds to wait (50 mili) for USB Data. Default 1000
  init_display();
  display_splash();
  WiFi.mode(WIFI_STA);
  WiFi.macAddress(mac);
  MacAddr += String(mac[5],HEX);
  MacAddr += String(mac[4],HEX);
  MacAddr += String(mac[3],HEX);
  MacAddr += String(mac[2],HEX);
  MacAddr += String(mac[1],HEX);
  MacAddr += String(mac[0],HEX);
  if (__DEBUG) Serial.println("MAC: "+MacAddr);

  display_boxdata();
  
  delay(3000);
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  
  // Add list of wifi networks
  wifiMulti.addAP(WIFI_SSID1, WIFI_PASSWORD1);
  wifiMulti.addAP(WIFI_SSID2, WIFI_PASSWORD2);
  wifiMulti.addAP(WIFI_SSID3, WIFI_PASSWORD3);
  wifiMulti.addAP(WIFI_SSID4, WIFI_PASSWORD4);
  wifiMulti.addAP(WIFI_SSID5, WIFI_PASSWORD5);

 // WiFi.scanNetworks will return the number of networks found
  int n = WiFi.scanNetworks();
  Serial.println("scan done");
  if (n == 0) {
      Serial.println("no networks found");
  } 
  else {
    Serial.print(n);
    Serial.println(" networks found");
    for (int i = 0; i < n; ++i) {
      // Print SSID and RSSI for each network found
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(WiFi.SSID(i));
      Serial.print(" (");
      Serial.print(WiFi.RSSI(i));
      Serial.print(")");
      Serial.println((WiFi.encryptionType(i) == WIFI_AUTH_OPEN)?" ":"*");
      delay(10);
    }
  }

  // Connect to Wi-Fi using wifiMulti (connects to the SSID with strongest connection)
  Serial.println("Connecting Wifi...");
  display_wifi_connecting();
  
  if(wifiMulti.run() == WL_CONNECTED) {
    IP = WiFi.localIP().toString();
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(IP);
    new_wifi = false;
    display_wifi_connected();
    delay(5000);
  }
  
}

void loop()
{
   if (wifiMulti.run(connectTimeoutMs) == WL_CONNECTED) {
    if (new_wifi)
      {
        Serial.print("WiFi connected: ");
        Serial.print(WiFi.SSID());
        Serial.print(" ");
        Serial.println(WiFi.RSSI());
        new_wifi = false;
      }
  }
  else {
    Serial.println("WiFi not connected!");
    new_wifi = true;
  }
  client->setInsecure(); // ignoriere SSL-Cert
  
  HTTPClient http ; //Webclient starten
  //Nachricht holen, wenn erster Durchlauf oder Intervall abgelaufen ist
  if ((fetchmessage) && (WiFi.status() == WL_CONNECTED))
    {
      //setze Abrufsignal ('*')
      display_fetch_flag();
      Serial.println("Fetching ... "+URL);
      http.begin(*client, URL+"?mac="+MacAddr+"&call="+Rufzeichen+"&loc="+Locator); //Verbindung zum Server aufbauen
      http.addHeader("Content-Type", "application/x-www-form-urlencoded");
      // Send HTTP GET request
      int httpResponseCode = http.GET();

      if (httpResponseCode > 0)
      {
        JSonMessage = http.getString(); // Abruf JsonObject
        Serial.println("Server Antwort: "+JSonMessage);
        DeserializationError err = deserializeJson(doc, JSonMessage); //Parse message   
        if (err) 
        {
            Serial.print(F("deserializeJson() failed with code: "));
            Serial.println(err.f_str());
            news_id = 10;
            news_topic = "ERROR";
            news_line1 = "";
            news_line2 = "Nachrichtenfehler!";
            news_line3 = "Bitte warten ...";
        }
        else
        {
          news_id = doc["messages"][0]["id"];
          news_date = doc["messages"][0]["date"];
          news_topic = doc["messages"][0]["topic"];
          news_line1 = doc["messages"][0]["line1"];
          news_line2 = doc["messages"][0]["line2"];
          news_line3 = doc["messages"][0]["line3"];
          if(strlen(news_topic) > 9) news_date = "";  // wenn das TOIPC mehr als 9 Zeichen hat, wird das Datum nicht ausgegeben...
        }
      } 
      else 
      {
          //Fehlermeldung für Debugzwecke
          Serial.print("Error code: ");
          Serial.println(httpResponseCode);
      }
      // Free resources
      http.end();
      fetchmessage = false; // Pause für den Abruf
      startTime = millis(); // Startzeit für Abruf neusetzten auf aktuellen Stand
      delay(1000); //Wartezeit für Sichtbarkeit des Abrufsignals
      remove_fetch_flag();
  
    if (news_id != old_id)  //bei neuer Nachricht auf dem Server
      {
        
        if (__DEBUG) {
          Serial.println(news_topic);
          Serial.println(news_date);
          Serial.println(news_line1);
          Serial.println(news_line2);
          Serial.println(news_line3);
        }

        display_message(); 
        old_id = news_id; //Sichere alte Nachrichten-id zum Vergleich
        #if defined(ENABLE_LED)
        digitalWrite(LED_PIN, HIGH); // Schalte LED ein
        #endif
        #if defined(ENABLE_BUZZER)
          #ifdef BUZZER_PASSIVE
            tone(BUZZER_PIN, 1000, 1000);
          #endif
          #ifdef BUZZER_ACTIVE
            digitalWrite(BUZZER_PIN, HIGH);
            delay(1000);
            digitalWrite(BUZZER_PIN, LOW);
          #endif    
        #endif        
      }
    }
    B_currentState = digitalRead(BUTTON_PIN);
    
    if (B_lastState == HIGH && B_currentState == LOW)
      {
       #if defined(ENABLE_LED)
         digitalWrite(LED_PIN, LOW); //Schalte LED aus wenn Taste gedrückt
       #endif
      } 
    B_lastState = B_currentState; // save the the last state of the button 
    
    if (millis() - startTime >= interval)
      {
        fetchmessage = true; //wenn Interval um, hole neue Nachricht vom Server in der nächsten loop
      }
}
