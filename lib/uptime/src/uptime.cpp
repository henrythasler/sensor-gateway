#include <uptime.h>

Uptime::Uptime(void)
{
}

uint32_t Uptime::getSeconds(void)
{
    timeval curTime;
    gettimeofday(&curTime, NULL);
    return curTime.tv_sec;
};

bool Uptime::setTime(tm time)
{
    time_t t = mktime(&time);
    struct timeval now = {.tv_sec = t};
    settimeofday(&now, NULL);
    return true;
}

tm *Uptime::getTime(void)
{
    time_t nowtime = getSeconds();
    return localtime(&nowtime);
}
