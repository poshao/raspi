# 佛系Linux学习

### 硬件材料

* 树莓派(zero w/3B+/4)
* 0.96'OLED屏幕(ssd1306)

### 依赖类库

* [FreeType](https://www.freetype.org/) 字体渲染引擎
* [zlib](https://zlib.net/) 压缩/解压缩库
* cJSON

### 运行方法

#### 树莓派加载屏幕驱动
``` sh
# IIC通讯
sudo dtoverlay ssd1306

# SPI通讯
#sudo dtoverlay ssd1306-spi
```

#### 编译
`make dev` 查看效果
`make run` 后台运行
`make install_freetype` 手动安装freetype字体渲染引擎
`make install_zlib` 手动安装zlib库

### 支持功能
* 显示日期时间
* 显示天气情况([和风天气](https://www.heweather.com/))