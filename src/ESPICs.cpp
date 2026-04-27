#include "ESPICs.h"
#include <WiFi.h>
#include <HTTPClient.h>
const char* ntpServer = "de.pool.ntp.org";
struct tm timeinfo;
const int maxTermine = 25;
String termine[maxTermine][3];
const int maxSyncTermine = 200;
String syncTermine[maxSyncTermine][3];
static char* URL;
void ESPICs::init(char* URLCalendar) {
    URL = URLCalendar;
}
void ESPICs::compactTermine(String ptermine[][3], int length) {
    int writeIndex = 0;

    for (int i = 0; i < length; i++) {
        if (ptermine[i][1] != "") {
            if (i != writeIndex) {
                ptermine[writeIndex][0] = ptermine[i][0];
                ptermine[writeIndex][1] = ptermine[i][1];
                ptermine[writeIndex][2] = ptermine[i][2];

                ptermine[i][0] = "";
                ptermine[i][1] = "";
                ptermine[i][2] = "";
            }
            writeIndex++;
        }
    }
    sortTermine(ptermine,length);
}
void ESPICs::printTermine() {
    Serial.println("=== Termine ===");

    for (int i = 0; i < maxTermine; i++) {

        if (termine[i][0] == "") {
            continue;
        }

        Serial.print("Termin ");
        Serial.println(i);

        Serial.print("  Name: ");
        Serial.println(termine[i][0]);

        Serial.print("  Start:  ");
        Serial.println(termine[i][1]);

        Serial.print("  Ende:  ");
        Serial.println(termine[i][2]);

        Serial.println("----------------------");
    }
}
void ESPICs::sortTermine(String ptermine[][3],int length) {
    for (int i = 0; i < length - 1; i++) {
        for (int j = 0; j < length - i - 1; j++) {
            if (ptermine[j][1] == "" || ptermine[j + 1][1] == "") {
                continue;
            }

            if (ptermine[j][1] > ptermine[j + 1][1]) {
                for (int k = 0; k < 3; k++) {
                    String temp = ptermine[j][k];
                    ptermine[j][k] = ptermine[j + 1][k];
                    ptermine[j + 1][k] = temp;
                }
            }
        }
    }
    saveTermine(ptermine);
}
void ESPICs::saveTermine(String ptermine[maxTermine][3]) {
    for (int i = 0; i < maxTermine; i++) {
        termine[i][0] = ptermine[i][0];
        termine[i][1] = ptermine[i][1];
        termine[i][2] = ptermine[i][2];
    }
}
void ESPICs::filterNextDays(int Days,String ptermine[][3], int length) {
    time_t now;
    time(&now);

    time_t limit = now + Days * 24 * 60 * 60;

    for (int i = 0; i < length; i++) {
        if (ptermine[i][1] == "") {
            continue;
        }
        struct tm t;
        time_t startTime = parseICalUTC(ptermine[i][1], &t);
        time_t endTime = parseICalUTC(ptermine[i][2], &t);

        if (endTime < now || startTime > limit) {
            ptermine[i][0] = "";
            ptermine[i][1] = "";
            ptermine[i][2] = "";
        }
    }
    compactTermine(ptermine,length);
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

    setenv("TZ", "UTC0", 1);
    tzset();

    time_t result = mktime(t);

    setenv("TZ", "CET-1CEST,M3.5.0,M10.5.0/3", 1);
    tzset();

    return result;
}
int ESPICs::syncTime() {
    Serial.println("Set Time");
    configTime(0, 0, ntpServer);
    while (!getLocalTime(&timeinfo)) {
        Serial.println("Waiting for NTP time...");
        delay(100);
    }
    setenv("TZ", "CET-1CEST,M3.5.0,M10.5.0/3", 1);
    tzset();
    getLocalTime(&timeinfo);
    Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
    return 0;
}
int ESPICs::getStatus(String* TitleNow, struct tm* TEndNow, String* TitleNext, struct tm* TBegNext) {
    filterNextDays(7,termine,maxTermine);
    int result;
    struct tm temp;
    time_t startTime = parseICalUTC(termine[0][1], &temp);
    time_t endTime = parseICalUTC(termine[0][2], &temp);
    time_t now;
    time(&now);

    if (startTime < now && endTime > now) {
        result = 1;
        *TitleNow = termine[0][0];
        *TitleNext = termine[1][0];
        localtime_r(&endTime, TEndNow);

        time_t nextStart = parseICalUTC(termine[1][1], &temp);
        localtime_r(&nextStart, TBegNext);
    }else {
        result = 0;
        *TitleNext = termine [0][0];
        localtime_r(&startTime, TBegNext);
    }

    return result;
}
int ESPICs::syncCal() {
    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
        http.begin(URL);;
        String payload;
        int httpCode = http.GET();
        if (httpCode > 0) {
            payload = http.getString();
        } else {
            return 1000 + httpCode;
        }
        http.end();

        for (int i = 0; i < maxSyncTermine; i++) {
            syncTermine[i][0] = "";
            syncTermine[i][1] = "";
            syncTermine[i][2] = "";
        }
        int eventIndex = -1;
        int pos = 0;
        while (pos < payload.length()) {
            int nextPos = payload.indexOf('\n', pos);
            if (nextPos == -1) {
                nextPos = payload.length();
            }

            String line = payload.substring(pos, nextPos);
            line.trim();
            if (line == "BEGIN:VEVENT") {
                eventIndex++;
                if (eventIndex >= maxSyncTermine) {
                    break;
                }
            } else if (eventIndex >= 0 && line.startsWith("DTSTART")) {
                int sep = line.indexOf(':');
                if (sep != -1) {
                    syncTermine[eventIndex][1] = line.substring(sep + 1);
                }
            } else if (eventIndex >= 0 && line.startsWith("DTEND")) {
                int sep = line.indexOf(':');
                if (sep != -1) {
                    syncTermine[eventIndex][2] = line.substring(sep + 1);
                }
            } else if (eventIndex >= 0 && line.startsWith("SUMMARY")) {
                int sep = line.indexOf(':');
                if (sep != -1) {
                    syncTermine[eventIndex][0] = line.substring(sep + 1);
                }
            }

            pos = nextPos + 1;
        }
        filterNextDays(7,syncTermine,maxSyncTermine);
        return 0;
    } else {
        return 100;
    }
}