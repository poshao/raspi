MAKE            ?=   make
CC              ?=   gcc
CFLAGS          :=   -Wall

INCLUDE  = -I ./src/include/ \
		   -I $(BUILD_DIR)/include/ \
		   -I $(BUILD_DIR)/include/freetype2/

LIBPATH  = -L $(BUILD_DIR)/lib/
LIBVAR   = -Wl,-dy -lfreetype -lc -lz -lm -lc

TARGET   =   test

ROOT_DIR :=   $(shell pwd)
BUILD_DIR   = $(ROOT_DIR)/build
OBJS_DIR    = $(BUILD_DIR)/objs


VPATH := src

SRCS =  display.c \
        log.c \
        bmp.c \
        http.c http_parser.c krypton.c cJSON.c api.c

# SRCS += test.c main.c
SRCS +=  time_weather.c 

OBJS = $(SRCS:%.c=$(OBJS_DIR)/%.o)


all:$(OBJS)
	$(CC) $^ -o $(BUILD_DIR)/$(TARGET) $(INCLUDE) $(LIBPATH) $(LIBVAR)

$(OBJS):$(OBJS_DIR)/%.o:%.c
	[ -d ${OBJS_DIR} ] || mkdir -p ${OBJS_DIR}
	$(CC) -c $(CFLAGS) $< -o $@ $(INCLUDE)

dev:all
	$(BUILD_DIR)/$(TARGET)

run:
	nohup $(BUILD_DIR)/$(TARGET) > /dev/null 2>&1 &

#伪目标
.PHONY:clean all

clean:
	-rm -rf $(OBJS_DIR)
	-rm -f $(BUILD_DIR)/$(TARGET)

# 下载编译freetype
install_freetype:
	[ -d ./dl ] || mkdir ./dl
	[ -d ./packages ] || mkdir ./packages
	[ -f ./dl/freetype-2.10.2.tar.gz ] || wget -4 -O ./dl/freetype-2.10.2.tar.gz  https://download.savannah.gnu.org/releases/freetype/freetype-2.10.2.tar.gz
	[ -d ./packages/freetype ] || tar zxf ./dl/freetype-2.10.2.tar.gz -C ./packages/ && mv ./packages/freetype-2.10.2 ./packages/freetype
	[ -e ${BUILD_DIR}/lib/libfreetype.so ] || cd ./packages/freetype && ./autogen.sh && ./configure --prefix=$(BUILD_DIR) && $(MAKE) && $(MAKE) install

install_jpeg:
	[ -d ./dl ] || mkdir ./dl
	[ -d ./packages ] || mkdir ./packages
	[ -f ./dl/jpegsrc.v9d.tar.gz ]
	[ -d ./packages/libjpeg ] || tar zxf ./dl/jpegsrc.v9d.tar.gz -C ./packages && mv ./packages/jpeg-9d ./packages/libjpeg
	cd ./packages/libjpeg && ./configure --prefix=$(BUILD_DIR) && $(MAKE) && $(MAKE) install

# 编译安装zlib
install_zlib:
	[ -d ./dl ] || mkdir ./dl
	[ -d ./packages ] || mkdir ./packages
	[ -f ./dl/zlib-1.2.11.tar.gz ] || wget -4 -O ./dl/zlib-1.2.11.tar.gz  https://zlib.net/zlib-1.2.11.tar.gz
	[ -d ./packages/zlib ] || tar zxf ./dl/zlib-1.2.11.tar.gz -C ./packages && mv ./packages/zlib-1.2.11 ./packages/zlib
	cd ./packages/zlib && ./configure --prefix=$(BUILD_DIR) && $(MAKE) test && $(MAKE) install