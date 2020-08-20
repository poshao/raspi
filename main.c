#include "display.h"
#include "log.h"
#include <unistd.h>

#include "test.h"

int main(int argc, char **argv)
{
    DEBUG("======Input args======\n");
    for (int i = 0; i < argc; i++)
    {
        DEBUG("(%d) [%s]\n", i, argv[i]);
    }
    DEBUG("======Input args======\n");
    // test();
    // test_Convert2BitBMP();
    // test_Convert2BitBMP_Folder();
    // test_getWeather();
    // return 0;

    dp_open("/dev/fb1");
    // dp_getScreenInfo();
    dp_ScreenInfo();

    // dp_beginDraw();

    dp_fillScreen(COLOR_DARK);
    

    // // 一堆点
    // for(int i=0;i<64;i+=1){
    //     dp_point(0xffff,i*2,i);
    // }

    // // 一堆线
    // for(int i=0;i<64;i+=2){
    //     dp_line(COLOR_LIGHT,0,i,127-i,i);
    // }

    // // 一堆方框
    // for(int i=0;i<32;i+=2){
    //     dp_rectangle(COLOR_LIGHT,i,i,127-i,63-i);
    // }

    // // 一堆圆圈
    // for(int i=0;i<128;i+=6){
    //     dp_circle(COLOR_LIGHT,64,32,i);
    // }

    // dp_text(L"hel0lo \n123测试",1,0,0,);

    test_showBadApple();

    // dp_endDraw();
    dp_refresh();

    // usleep(100000);
    dp_close();
}
