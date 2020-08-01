#ifndef __LL_LOG__
#define __LL_LOG__


enum LOG_TYPE{
    LOG_ERROR,
    LOG_WARNNING,
    LOG_INFORMATION,
    LOG_DEBUG
};

#define DEBUG(format,args...)  ll_log(LOG_DEBUG,format,##args)
#define INFO(format,args...)   ll_log(LOG_INFORMATION,format,##args)
#define WARN(format,args...)   ll_log(LOG,LOG_WARNNING,##args)
#define ERROR(format,args...)  ll_log(LOG_ERROR,format,##args)

void ll_log(int level,char* format,...);

#endif