#ifndef ESPICS_ESPICS_H
#define ESPICS_ESPICS_H
#include <Arduino.h>
#include <time.h>

class ESPICs {
public:
    static void init(char *URLCalendar);
    static time_t parseICalUTC(String s, struct tm* t);
    static int syncTime();
    static int syncCal();
    static int getStatus(String* TitleNow, struct tm* TEndNow, String* TitleNext, struct tm* TBegNext);

private:
    static void filterNextDays(int days);
    static void compactTermine();
    static void sortTermine();
};

#endif //ESPICS_ESPICS_H
