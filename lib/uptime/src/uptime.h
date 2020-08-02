#ifndef UPTIME_H
#define UPTIME_H

#define __BSD_VISIBLE 1

#include <stdint.h>
#include <stddef.h>
#include <time.h>
#include <sys/time.h>

using namespace std;

class Uptime
{
private:
public:
    Uptime(void);
    uint32_t getSeconds(void);
    bool setTime(tm time);
    tm *getTime(void);
};
#endif