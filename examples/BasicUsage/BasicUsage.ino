#include "ESPICs.h"
#include <WiFi.h>
#include <HTTPClient.h>
const char* ssid = ""; // Your WiFi SSID
const char* password = ""; //Your WiFi Password
char* cURL = "";//Your Calendar URL
ESPICs CalHelper;
String titleNow, titleNext;
struct tm TBegNow, TEndNow, TBegNext, TEndNext;
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
  
  if(CalHelper.getStatus(&titleNow,&TBegNow, &TEndNow, &titleNext, &TBegNext, &TEndNext) == 0){
    Serial.print("free, Next Event: ");
    Serial.println(titleNext);
    Serial.print("Starts at:");
    Serial.println(&TBegNext, "%A, %B %d %Y %H:%M:%S");
    Serial.print("Ends at:");
    Serial.println(&TEndNext, "%A, %B %d %Y %H:%M:%S");
  }else{
    Serial.print("occupied with:");
    Serial.println(titleNow);
    Serial.print("Event began:");
    Serial.println(&TBegNow, "%A, %B %d %Y %H:%M:%S");
    Serial.print("Event ends:");
    Serial.println(&TEndNow, "%A, %B %d %Y %H:%M:%S");
    Serial.print("Next Event: ");
    Serial.println(titleNext);
    Serial.print("Starts at:");
    Serial.println(&TBegNext, "%A, %B %d %Y %H:%M:%S");
    Serial.print("Ends at:");
    Serial.println(&TEndNext, "%A, %B %d %Y %H:%M:%S");
  }
  delay(20000);
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
