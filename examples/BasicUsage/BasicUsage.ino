#include "ESPICs.h"
#include <WiFi.h>
#include <HTTPClient.h>
const char* ssid = ""; // Your WiFi SSID
const char* password = ""; //Your WiFi Password
char* cURL = "";//Your Calendar URL
ESPICs CalHelper;
String titleNow, titleNext;
struct tm TEndNow, TBegNext;
void setup() {
  Serial.begin(115200);
  Serial.println("Start");
  CalHelper.init(cURL);
  connectWiFi();
  CalHelper.syncTime();
  CalHelper.syncCal();
  CalHelper.printTermine();

}

void loop() {
  
  if(CalHelper.getStatus(&titleNow, &TEndNow, &titleNext, &TBegNext) == 0){
    Serial.print("free, Next Event: ");
    Serial.println(titleNext);
    Serial.print("Starts at:");
    Serial.println(&TBegNext, "%A, %B %d %Y %H:%M:%S");
  }else{
    Serial.print("occupied with:");
    Serial.println(titleNow);
    Serial.print("Event ends:");
    Serial.println(&TEndNow, "%A, %B %d %Y %H:%M:%S");
    Serial.print("Next Event: ");
    Serial.println(titleNext);
    Serial.print("Starts at:");
    Serial.println(&TBegNext, "%A, %B %d %Y %H:%M:%S");
  }
  delay(20000);
}
int connectWiFi() {
  WiFi.begin(ssid, password);
  Serial.print("WLAN-Verbindung wird hergestellt...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nVerbunden mit dem WLAN");
  return 0;
}
