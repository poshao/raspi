#include <linux/types.h>
#include <unistd.h>
#include <stdio.h>
#include <zlib.h>

#include "bmp.h"
#include "display.h"

// 播放BadApple视频
void test_showBadApple()
{
    char picDir[100] = "resources/test/badapple";
    char filename[200] = {0};

    for (int i = 1; i < 6566; i++)
    {
        sprintf(filename, "%s/%08d.bmp", picDir, i);
        // printf("%s\n",f);
        dp_image_bmp(0xffffff, filename, 0, 0);
        dp_refresh();
        usleep(33000);
    }
    printf("play finished!\n");
}

// 转化成二值位图
void test_Convert2BitBMP()
{
    char srcFile[100] = "resources/test/24bit.bmp";
    char destFile[100] = "temp/test_bit.bmp";

    convert2bitPic(srcFile, destFile);

    printf("convert finished! [%s]\n", destFile);
}

#include <dirent.h>
#include <sys/stat.h>
// 批量转换二值图
void test_Convert2BitBMP_Folder()
{
    char srcFolder[100] = "/home/shaozi/下载/WeatherIcon-master/weather-icon-S1/32/";
    char destFolder[100] = "/home/shaozi/下载/WeatherIcon-master/weather-icon-S1/32_bit/";
    char srcPath[200] = {0}, destPath[200] = {0};

    DIR *d = NULL;
    struct dirent *dp = NULL; /* readdir函数的返回值就存放在这个结构体中 */
    struct stat st;
    char p[200] = {0};
    if (!(d = opendir(srcFolder)))
    {
        printf("opendir[%s] error: %m\n", srcFolder);
        return;
    }

    while ((dp = readdir(d)) != NULL)
    {
        /* 把当前目录.，上一级目录..及隐藏文件都去掉，避免死循环遍历目录 */
        if ((!strncmp(dp->d_name, ".", 1)) || (!strncmp(dp->d_name, "..", 2)))
            continue;

        snprintf(p, sizeof(p) - 1, "%s/%s", srcFolder, dp->d_name);
        stat(p, &st);
        if (!S_ISDIR(st.st_mode))
        {

            sprintf(srcPath, "%s/%s", srcFolder, dp->d_name);
            sprintf(destPath, "%s/%s", destFolder, dp->d_name);
            convert2bitPic(srcPath, destPath);
            printf("%s\n", destPath);
            // } else {
            // printf("%s/\n", dp->d_name);
            // trave_dir(p);
        }
    }
    closedir(d);
}

#include "http.h"
#include "cJSON.h"
void test_getWeather()
{
    // char url[200] = "https://devapi.heweather.net/v7/weather/now?gzip=n&location=101280112&key=cdc1ea6006344312b59a92273adfab45";
    char url[100]="https://devapi.heweather.net/v7/weather/now";

    struct ft_http_client_t *client;
    char *body;

    cJSON *root, *now, *item;

    ft_http_init();
    client = ft_http_new();
    ft_http_set_timeout(client, 5000);
    body = ft_http_sync_request(client, url, M_GET);

    printf("recv body: %s\n", body);
    // root = cJSON_Parse(body);
    // now = cJSON_GetObjectItem(root, "now");

    // item = cJSON_GetObjectItem(now, "temp");
    // printf("温度: %s\n", item->valuestring);

    // item = cJSON_GetObjectItem(now, "humidity");
    // printf("湿度: %s\n", item->valuestring);
    // item = cJSON_GetObjectItem(now, "pressure");
    // printf("压强: %s\n", item->valuestring);

    ft_http_destroy(client);
    ft_http_exit(client);
    ft_http_deinit();
}

// unsigned int decompress(char *data,int len){
//     uncompress();
//     unsigned int ret;
//     z_stream strm;
    
//     strm.zalloc = Z_NULL;
//     strm.zfree = Z_NULL;
//     strm.opaque = Z_NULL;
//     strm.avail_in = 0;
//     strm.next_in = Z_NULL;
//     ret = inflateInit(&strm);
//     if (ret != Z_OK)
//         return ret;
    
//     do{
//         strm.avail_in=*len;
//         strm.next_in=data;

//         do{
//             strm.avail_out = CHUNK;
//             strm.next_out = out;
//             ret=inflate(&strm, Z_NO_FLUSH);
//         }while (strm.avail_out == 0);
//     }while (ret != Z_STREAM_END);

//     (void)inflateEnd(&strm);
//     return ret == Z_STREAM_END ? Z_OK : Z_DATA_ERROR;
// }