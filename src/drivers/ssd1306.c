// #include <linux/init.h>
#include <linux/module.h>
#include <linux/fb.h>
// #include <linux/i2c.h>
#include <linux/spi/spi.h>
#include <linux/gpio.h>
// #include <linux/proc_fs.h>
#include <linux/of_device.h>
// #include <linux/kernel.h>

#define DEV_NAME "ssd1306_driver"

struct ssd1306fb_par
{
    u32 width;
    u32 height;
};

static const struct fb_fix_screeninfo ssd1306fb_fix = {
    .id = "ssd1306fb",
    .type = FB_TYPE_PACKED_PIXELS,
    .visual = FB_VISUAL_MONO10,
    .xpanstep = 0,
    .ypanstep = 0,
    .ywrapstep = 0,
    .accel = FB_ACCEL_NONE,
};

static const struct fb_var_screeninfo ssd1306fb_var = {
    .bits_per_pixel = 1,
    .red = {.length = 1},
    .green = {.length = 1},
    .blue = {.length = 1},
};

struct ssd1306_deviceinfo
{
    char name[10];
};

static struct fb_ops ssd1306fb_ops = {
    .owner = THIS_MODULE,
    .fb_read = fb_sys_read,
    // .fb_write = ssd1307fb_write,
    // .fb_blank = ssd1307fb_blank,
    // .fb_fillrect = ssd1307fb_fillrect,
    // .fb_copyarea = ssd1307fb_copyarea,
    // .fb_imageblit = ssd1307fb_imageblit
};

static int ssd1306_write_cmd(struct spi_device *spi,u8 cmd){
    
    spi_write(spi,&cmd,1);
}

static int ssd1306_write_data(struct spi_device *spi,u8 data){
    spi_write(spi,&data,1);
}

// 初始化设备
static int ssd1306_init(){
    devm_gpiod_get_from_of_node
    gpiod_set_value()
}

static struct fb_info *info = NULL;

static int ssd1306_probe(struct spi_device *dev)
{
    struct device_node *node = dev->dev.of_node;
    struct ssd1306fb_par *par = NULL;
    int lRet;

    // struct ssd1306_deviceinfo *device_info = NULL;

    dev_info(&dev->dev,"ssd1306 driver probe\n");

    if (!node)
    {
        dev_err(&dev->dev, "device tree not found\n");
        return -EINVAL;
    }

    info = framebuffer_alloc(sizeof(struct ssd1306fb_par), &dev->dev);
    if (!info)
    {
        return -ENOMEM;
    }

    par = info->par;

    // 从设备树读取信息
    of_property_read_u32(node, "solomon,width", &par->width);
    of_property_read_u32(node, "solomon,height", &par->height);

    dev_info(&dev->dev,"screen size:%dx%d\n", par->width, par->height);

    info->fbops = &ssd1306fb_ops;
    info->fix = ssd1306fb_fix;
    info->fix.line_length = DIV_ROUND_UP(par->width,8); // 128/8
    
    // info->fbdefio = ssd1306fb_defio; // 定时刷新

    info->var = ssd1306fb_var;
    info->var.width = info->var.xres = par->width;
    info->var.height = info->var.yres = par->height;

    // 初始化设备


    // 注册FrameBuffer设备
    lRet = register_framebuffer(info);
    return 0;
}

static int ssd1306_remove(struct spi_device *dev)
{
    printk(KERN_INFO "ssd1306 remove\n");
    unregister_framebuffer(info);
    framebuffer_release(info);
    return 0;
}

static struct of_device_id ssd1306_of_match[] = {
    {.compatible = "solomon,ssd1306-spi",
     .data = (void *)&((struct ssd1306_deviceinfo){
         .name = "hello"})},
    {}};
MODULE_DEVICE_TABLE(of, ssd1306_of_match);

static struct spi_driver ssd1306_driver = {
    .probe = ssd1306_probe,
    .remove = ssd1306_remove,
    .driver = {
        .name = DEV_NAME,
        .of_match_table = ssd1306_of_match}};

module_spi_driver(ssd1306_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Byron Gong<1032066879@qq.com>");