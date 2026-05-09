#ifndef ESPICS_ESPICS_H
#define ESPICS_ESPICS_H
#include <Arduino.h>
#include <time.h>

class ESPICs {
public:
    static const int maxTermine = 25;
    static const int maxSyncTermine = 200;

    static void printTermine();
    static void setTimezone(const char* timezone);
    static void init(char *URLCalendar);
    static time_t parseICalUTC(String s, struct tm* t);
    static int syncTime();
    static int syncCal();
    static int getStatus(String* TitleNow, struct tm* TEndNow, String* TitleNext, struct tm* TBegNext);
    static int getStatusRaw(String* TitleNow, String* TEndNow, String* TitleNext, String* TBegNext);

private:
    static void filterNextDays(int days,String ptermine[][3], int length);
    static void compactTermine(String ptermine[][3], int length);
    static void sortTermine(String ptermine[][3], int length);
    static void saveTermine(String ptermine[maxTermine][3]);
};

#endif //ESPICS_ESPICS_H
