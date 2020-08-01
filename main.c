#include "display.h"
#include "log.h"

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
    test_getWeather();
    return 0;

    dp_open("/dev/fb1");
    // dp_getScreenInfo();
    dp_ScreenInfo();

    // dp_beginDraw();

    // dp_fillScreen(0);

    // // 一堆点
    // for(int i=0;i<64;i+=1){
    //     dp_point(0xffff,i*2,i);
    // }

    // // 一堆线
    // for(int i=0;i<64;i+=2){
    //     dp_line(1,0,i,127-i,i);
    // }

    // // 一堆方框
    // for(int i=0;i<32;i+=2){
    //     dp_rect(1,i,i,127-i,63-i);
    // }

    // // 一堆圆圈
    // for(int i=0;i<128;i+=6){
    //     dp_circle(1,64,32,i);
    // }

    // dp_text(0xffff,10,10,L"hel0lo \n123测试");

    test_showBadApple();

    // dp_endDraw();
    dp_close();
}
