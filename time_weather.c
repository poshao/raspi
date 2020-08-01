#include <signal.h>
#include <string.h>
#include <wchar.h>
#include <locale.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#include <ifaddrs.h>
#include <arpa/inet.h>
#include <stdio.h>

#include <pthread.h>

#include "display.h"
#include "log.h"

#define MAX_LENGTH 200

FT_Face _font = NULL;

#include "http.h"
#include "cJSON.h"

typedef struct WeatherInfo
{
    float temp;
    float hum;
    float press;
    int iconid;
    char text[10];
} WeatherInfo;

// 获取天气信息
void getWeatherInfo(WeatherInfo *info)
{
    char url[MAX_LENGTH] = "https://devapi.heweather.net/v7/weather/now?location=101280112&key=cdc1ea6006344312b59a92273adfab45";
    struct ft_http_client_t *client;
    char *body;

    cJSON *root, *now, *item;

    ft_http_init();
    client = ft_http_new();
    ft_http_set_timeout(client, 5000);

    body = ft_http_sync_request(client, url, M_GET);

    DEBUG("recv body: %s", body);
    root = cJSON_Parse(body);
    now = cJSON_GetObjectItem(root, "now");

    item = cJSON_GetObjectItem(now, "temp");
    DEBUG("温度: %s", item->valuestring);
    sscanf(item->valuestring, "%f", &info->temp);

    item = cJSON_GetObjectItem(now, "humidity");
    DEBUG("湿度: %s", item->valuestring);
    sscanf(item->valuestring, "%f", &info->hum);

    item = cJSON_GetObjectItem(now, "pressure");
    DEBUG("压强: %s", item->valuestring);
    sscanf(item->valuestring, "%f", &info->press);

    item = cJSON_GetObjectItem(now, "icon");
    DEBUG("ICON: %s", item->valuestring);
    sscanf(item->valuestring, "%d", &info->iconid);

    item = cJSON_GetObjectItem(now, "text");
    DEBUG("TEXT: %s", item->valuestring);
    strcpy(info->text, item->valuestring);

    ft_http_destroy(client);
    ft_http_exit(client);
    ft_http_deinit();
}

// 获取内存信息
void getMemInfo(double *totalGb, double *usagePercent)
{
    FILE *fd;
    char name1[20], unit[10];
    double mem_total, mem_free;
    char buff[256];

    fd = fopen("/proc/meminfo", "r");

    fgets(buff, sizeof(buff), fd);
    sscanf(buff, "%s %lf %s\n", name1, &mem_total, unit);
    fgets(buff, sizeof(buff), fd);
    // sscanf(buff, "%s %ld %s\n", name1, &mem_free,unit);
    fgets(buff, sizeof(buff), fd);
    sscanf(buff, "%s %lf %s\n", name1, &mem_free, unit);
    fclose(fd);

    *usagePercent = 1.0 * (mem_total - mem_free) / mem_total;
    *totalGb = mem_total / 1024 / 1024.0;
    // DEBUG("meminfo total : %.02fGB usage: %.2f%\n",mem_total,100*mem_used_rate);
}

// 获取cpu温度
float getCpuTemperate()
{
    FILE *fd;
    int temp;
    char buff[256];

    fd = fopen("/sys/class/thermal/thermal_zone0/temp", "r");
    fgets(buff, sizeof(buff), fd);
    sscanf(buff, "%d", &temp);
    fclose(fd);

    // DEBUG("cpu temp: %0.2f℃\n",temp/1000.0);
    return temp / 1000.0;
}

// 获取IP地址
void getIPs()
{
    struct ifaddrs *ifAddrStruct = NULL;
    void *tmpAddrPtr = NULL;

    if (getifaddrs(&ifAddrStruct) != 0)
    {
        ERROR("get ip failed");
    }

    struct ifaddrs *iter = ifAddrStruct;

    while (iter != NULL)
    {
        if (iter->ifa_addr->sa_family == AF_INET)
        { //if ip4
            // is a valid IP4 Address
            char addressBuffer[INET_ADDRSTRLEN];
            tmpAddrPtr = &((struct sockaddr_in *)iter->ifa_addr)->sin_addr;
            inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);
            if (strcmp(iter->ifa_name, "lo") != 0)
            {
                DEBUG("%5s: %s", iter->ifa_name, addressBuffer);
            }
        }
        //else if (ifaddrstruct->ifa_addr->sa_family == af_inet6) { // check it is ip6

        /* deal ip6 addr */
        //    tmpaddrptr = &((struct sockaddr_in *)ifaddrstruct->ifa_addr)->sin_addr;
        //    char addressbuffer[inet6_addrstrlen];
        //    inet_ntop(af_inet6, tmpaddrptr, addressbuffer, inet6_addrstrlen);

        //}
        iter = iter->ifa_next;
    }
    //releas the struct
    freeifaddrs(ifAddrStruct);
}

// 运行状态帧
void statusFrame()
{
    wchar_t buffer[MAX_LENGTH];
    double mem_total, mem_usage;

    int y = 64 - 18;

    // float cpuTemp;
    dp_fillScreen(0);

    // CPU信息绘制
    swprintf(buffer, MAX_LENGTH, L"CPU: %.2f℃", getCpuTemperate());
    // dp_text(buffer, 1, 0, 0, _font, 16);
    y = 0;
    dp_text(L"C", 1, 3, y - 2, _font, 18);
    dp_rectangle(1, 0, y, 17, y + 17);
    dp_text(buffer, 1, 24, y - 2, _font, 16);

    // 内存信息绘制
    getMemInfo(&mem_total, &mem_usage);
    swprintf(buffer, MAX_LENGTH, L"%.2fGB %.2f%%", mem_total, mem_usage * 100);

    y = 20;
    // 内存信息块,width:128px height:18px
    dp_text(L"M", 1, 2, y - 2, _font, 18);
    dp_rectangle(1, 0, y, 17, y + 17);
    dp_processbar_LR(1, 24, y + 13, 104, mem_usage);
    dp_text(buffer, 1, 24, y - 2, _font, 14);

    dp_refresh();
    // dp_text(L"CPU: 44.39℃ ");
}

// 天气帧
WeatherInfo weather;

void weatherFrame(int color)
{
    wchar_t buffer[MAX_LENGTH] = {0};
    char iconPath[100] = {0};

    dp_fillScreen(~color);

    // getWeatherInfo(&weather);

    sprintf(iconPath, "resources/weather/32_bit/%d.bmp", weather.iconid);
    dp_image_bmp(color, iconPath, 0, 0);

    swprintf(buffer, MAX_LENGTH, L"%.0f℃", weather.temp);
    dp_text(buffer, color, 1, 27, _font, 22);

    mbstowcs(buffer, weather.text, MAX_LENGTH);
    dp_text(buffer, color, 30, 0, _font, 14);

    swprintf(buffer, MAX_LENGTH, L"%.0f%%", weather.hum);
    dp_text(buffer, color, 30, 16, _font, 12);
    swprintf(buffer, MAX_LENGTH, L"%.0fhPa", weather.press);
    dp_text(buffer, color, 1, 50, _font, 12);

    time_t t;
    struct tm *lt;
    time(&t);           //获取Unix时间戳。
    lt = localtime(&t); //转为时间结构。

    // swprintf(buffer, MAX_LENGTH, L"%4d", lt->tm_year + 1900);
    // dp_text(buffer, color, 58, 40, _font, 18);
    // // dp_line(color,68,20,110,20);
    swprintf(buffer, MAX_LENGTH, L"%02d月%02d日", lt->tm_mon + 1, lt->tm_mday);
    dp_text(buffer, color, 56, 0, _font, 16);
    wchar_t week[] = L"日一二三四五六";

    swprintf(buffer, MAX_LENGTH, L"星期%lc %02d", week[lt->tm_wday], lt->tm_sec);
    dp_text(buffer, color, 58, 20, _font, 14);

    // swprintf(buffer, MAX_LENGTH, L"%02d", lt->tm_mday);
    // dp_text(buffer, color, 69, 30, _font, 18);

    // dp_text(L"07月13日",color,58,28,_font,16);

    swprintf(buffer, MAX_LENGTH, L"%02d:%02d", lt->tm_hour, lt->tm_min);
    dp_text(buffer, color, 50, 32, _font, 28);

    dp_processbar_BT(color, 122, 63, 64, lt->tm_sec / 60.0);

    dp_refresh();
}

//定时器更新
void timer_update()
{
    static int weather_update_tick = 0;
    static int color_change_tick = 0;
    static int color = ~0;

    if (weather_update_tick++ > 300)
    {
        getWeatherInfo(&weather);
        weather_update_tick = 0;
    }

    if (color_change_tick++ > 8)
    {
        color = ~color;
        color_change_tick = 0;
    }
    // DEBUG("timer update:  %d\n",weather_update_tick);
    weatherFrame(color);
}

int main(int argc, char **argv)
{
    DEBUG("======Input args======");
    for (int i = 0; i < argc; i++)
    {
        DEBUG("(%d) [%s]", i, argv[i]);
    }
    DEBUG("======Input args======");

    // 无需屏幕的代码

    // getIPs();
    // getCpuTemperate();
    // getMemUsage();

    // return 0;

    setlocale(LC_CTYPE, "");

    dp_open("/dev/fb1");
    dp_ScreenInfo();
    _font = dp_loadFont("resources/fonts/msyh.ttc");
    getWeatherInfo(&weather);

    // weatherFrame();
    // timeFrame();
    // statusFrame();

    struct itimerval timer;
    memset(&timer, 0, sizeof(timer));
    timer.it_value.tv_sec = 1;
    timer.it_value.tv_usec = 0;
    timer.it_interval.tv_sec = 1;
    timer.it_interval.tv_usec = 0;

    signal(SIGALRM, timer_update);
    if (0 > setitimer(ITIMER_REAL, &timer, NULL))
    {
        perror("set timer failed");
        exit(-1);
    }

    while (1)
    {

        pause();
    }

    dp_freeFont(_font);
    dp_close();

    return 0;
}