#include "ESPICs.h"
#include <WiFi.h>
#include <HTTPClient.h>
const char* ntpServer = "de.pool.ntp.org";
struct tm timeinfo;
const int maxTermine = 25;
String termine[maxTermine][3];
static char* URL;
void ESPICs::init(char* URLCalendar) {
    URL = URLCalendar;
}
void ESPICs::compactTermine() {
    int writeIndex = 0;

    for (int i = 0; i < maxTermine; i++) {
        if (termine[i][1] != "") {
            if (i != writeIndex) {
                termine[writeIndex][0] = termine[i][0];
                termine[writeIndex][1] = termine[i][1];
                termine[writeIndex][2] = termine[i][2];

                termine[i][0] = "";
                termine[i][1] = "";
                termine[i][2] = "";
            }
            writeIndex++;
        }
    }
}
void ESPICs::sortTermine() {
    for (int i = 0; i < maxTermine - 1; i++) {
        for (int j = 0; j < maxTermine - i - 1; j++) {
            if (termine[j][1] == "" || termine[j + 1][1] == "") {
                continue;
            }

            if (termine[j][1] > termine[j + 1][1]) {
                for (int k = 0; k < 3; k++) {
                    String temp = termine[j][k];
                    termine[j][k] = termine[j + 1][k];
                    termine[j + 1][k] = temp;
                }
            }
        }
    }
}
void ESPICs::filterNextDays(int Days) {
    time_t now;
    time(&now);

    time_t limit = now + Days * 24 * 60 * 60;

    for (int i = 0; i < maxTermine; i++) {
        if (termine[i][1] == "") {
            continue;
        }
        struct tm t;
        time_t startTime = parseICalUTC(termine[i][1], &t);
        time_t endTime = parseICalUTC(termine[i][2], &t);

        if (endTime < now || startTime > limit) {
            termine[i][0] = "";
            termine[i][1] = "";
            termine[i][2] = "";
        }
    }
}
time_t ESPICs::parseICalUTC(String s, struct tm* t) {
    if (s.length() < 16) return 0;

    *t = {};
    t->tm_year = s.substring(0, 4).toInt() - 1900;
    t->tm_mon = s.substring(4, 6).toInt() - 1;
    t->tm_mday = s.substring(6, 8).toInt();
    t->tm_hour = s.substring(9, 11).toInt();
    t->tm_min = s.substring(11, 13).toInt();
    t->tm_sec = s.substring(13, 15).toInt();
    t->tm_isdst = 0;

    // aktuelle TZ sichern
    char* oldTZ = getenv("TZ");

    // auf UTC setzen
    setenv("TZ", "UTC0", 1);
    tzset();

    time_t result = mktime(t);

    // alte TZ zurücksetzen
    if (oldTZ) setenv("TZ", oldTZ, 1);
    else unsetenv("TZ");
    tzset();

    return result;
}
int ESPICs::syncTime() {
    Serial.println("Time");
    configTime(0, 0, ntpServer);
    while (!getLocalTime(&timeinfo)) {
        Serial.println("Waiting for NTP time...");
        delay(100);
    }
    setenv("TZ", "CET-1CEST,M3.5.0,M10.5.0/3", 1);
    getLocalTime(&timeinfo);
    Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
    return 0;
}
int ESPICs::getStatus(String* TitleNow, struct tm* TEndNow, String* TitleNext, struct tm* TBegNext) {
    filterNextDays(7);
    compactTermine();
    sortTermine();
    int result;
    time_t startTime = parseICalUTC(termine[0][1], TEndNow);
    time_t endTime = parseICalUTC(termine[0][2], TEndNow);
    time_t now;
    time(&now);

    if (startTime < now && endTime > now) {
        result = 1;
        *TitleNow = termine[0][0];
        *TitleNext = termine[1][0];
        parseICalUTC(termine[1][1], TBegNext);
    }else {
        result = 0;
        *TitleNext = termine [0][0];
        parseICalUTC(termine[0][1], TBegNext);
    }

    return result;
}
int ESPICs::syncCal() {
    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
        http.begin(URL);
        Serial.println("URL OK");
        String payload;
        int httpCode = http.GET();  //HTTP GET-Request senden
        if (httpCode > 0) {
            payload = http.getString();
            //Serial.println(payload);
        } else {
            return 1000 + httpCode;
        }
        http.end();
        // Termine [2][] soll folgendermassen mit den ICal daten des Payloads gefüllt werden:
        //Startzeit | Endzeit
        for (int i = 0; i < maxTermine; i++) {
            termine[i][0] = "";
            termine[i][1] = "";
            termine[i][2] = "";
        }
        Serial.println("Cleared");
        int eventIndex = -1;
        int pos = 0;
        Serial.println(payload.length());
        while (pos < payload.length()) {
            int nextPos = payload.indexOf('\n', pos);
            if (nextPos == -1) {
                nextPos = payload.length();
            }

            String line = payload.substring(pos, nextPos);
            line.trim();  // entfernt \r und Leerzeichen
            if (line == "BEGIN:VEVENT") {
                eventIndex++;
                if (eventIndex >= maxTermine) {
                    break;
                }
            } else if (eventIndex >= 0 && line.startsWith("DTSTART")) {
                int sep = line.indexOf(':');
                if (sep != -1) {
                    termine[eventIndex][1] = line.substring(sep + 1);
                }
            } else if (eventIndex >= 0 && line.startsWith("DTEND")) {
                int sep = line.indexOf(':');
                if (sep != -1) {
                    termine[eventIndex][2] = line.substring(sep + 1);
                }
            } else if (eventIndex >= 0 && line.startsWith("SUMMARY")) {
                int sep = line.indexOf(':');
                if (sep != -1) {
                    termine[eventIndex][0] = line.substring(sep + 1);
                }
            }

            pos = nextPos + 1;
        }
        return 0;
    } else {
        return 100;
    }
}