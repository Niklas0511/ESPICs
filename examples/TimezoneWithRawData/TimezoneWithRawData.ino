#include "ESPICs.h"
#include <WiFi.h>
#include <HTTPClient.h>
const char* ssid = ""; // Your WiFi SSID
const char* password = ""; //Your WiFi Password
char* cURL = "";//Your Calendar URL
ESPICs CalHelper;
String titleNow, titleNext;
String TEndNow, TBegNext;
void setup() {
  Serial.begin(115200);
  Serial.println("Start");
  CalHelper.init(cURL);
  connectWiFi();
  CalHelper.syncTime();
  CalHelper.syncCal();
  CalHelper.printTermine();
  CalHelper.setTimezone("UTC");

}

void loop() {
    if(CalHelper.getStatusRaw(&titleNow, &TEndNow, &titleNext, &TBegNext) == 0){
    Serial.print("free, Next Event: ");
    Serial.println(titleNext);
    Serial.print("Starts at:");
    Serial.println(TBegNext);
  }else{
    Serial.print("occupied with:");
    Serial.println(titleNow);
    Serial.print("Event ends:");
    Serial.println(TEndNow);
    Serial.print("Next Event: ");
    Serial.println(titleNext);
    Serial.print("Starts at:");
    Serial.println(TBegNext);
  }  
  delay(2000);
}
int connectWiFi() {
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi connected");
  return 0;
}