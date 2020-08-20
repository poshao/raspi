#include <stdio.h>
#include <stdlib.h>

#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#include <linux/fb.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

#include <math.h>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H
#include <wchar.h>

#include "display.h"
#include "log.h"

// 屏幕相关变量
static int _fdev = 0;
static struct fb_var_screeninfo _vinfo;
static struct fb_fix_screeninfo _finfo;

static char *_fbuff = 0;
static char *_buff = 0;
static size_t _buff_size = 0;

// 字体相关变量
static FT_Library _ft_lib = NULL;
static FT_Face _ft_face = NULL;
static FT_Error _ft_error;

void dp_close()
{
    if (_ft_lib != NULL)
    {
        FT_Done_FreeType(_ft_lib);
        _ft_lib = NULL;
    }
    dp_endDraw();
    if (_fdev != 0)
    {
        close(_fdev);
    }
    _fdev = 0;
}

int dp_open(char *fb_dev)
{
    int rs;
    dp_close();
    rs = open(fb_dev, O_RDWR);
    if (_fdev < 0)
    {
        ERROR("open [%s] failed! (%d)", fb_dev, rs);
    }
    _fdev = rs;
    dp_getScreenInfo();
    dp_beginDraw();
    // 初始化字体
    _ft_error = FT_Init_FreeType(&_ft_lib);
    if (_ft_error)
        ERROR("init font library failed(%d)!", _ft_error);

    return _fdev;
}

int dp_getScreenInfo()
{
    int rs;
    rs = ioctl(_fdev, FBIOGET_FSCREENINFO, &_finfo);
    if (rs)
    {
        ERROR("read fixed information failed!(%d)", rs);
        return -1;
    }

    rs = ioctl(_fdev, FBIOGET_VSCREENINFO, &_vinfo);
    if (rs)
    {
        ERROR("read variable information failed!(%d)", rs);
        return -1;
    }
    return 0;
}

void dp_ScreenInfo()
{
    INFO("===screen information===");
    INFO("name: %s", _finfo.id);
    INFO("mem size(byte): %d", _finfo.smem_len);
    INFO("line length(byte): %d", _finfo.line_length);

    INFO("visual: =>");
    switch (_finfo.visual)
    {
    case FB_VISUAL_DIRECTCOLOR:
        INFO("DIRECTCOLOR");
        break;
    case FB_VISUAL_FOURCC:
        INFO("FOURCC");
        break;
    case FB_VISUAL_MONO01:
        INFO("MONO01");
        break;
    case FB_VISUAL_MONO10:
        INFO("MONO10");
        break;
    case FB_VISUAL_PSEUDOCOLOR:
        INFO("PSEUDOCOLOR");
        break;
    case FB_VISUAL_STATIC_PSEUDOCOLOR:
        INFO("STATIC_PSEUDOCOLOR");
        break;
    case FB_VISUAL_TRUECOLOR:
        INFO("TRUECOLOR");
        break;
    default:
        break;
    }

    INFO("width(pixel): %d", _vinfo.xres);
    INFO("height(pixel): %d", _vinfo.yres);
    INFO("bits of per pixel(bit): %d", _vinfo.bits_per_pixel);

    INFO("===screen information===");
}

int dp_endDraw()
{
    int rs;
    if (_fbuff != 0)
    {
        rs = munmap(_fbuff, _finfo.smem_len);
        if (rs == -1)
        {
            ERROR("endDraw munmap failed(%d)", rs);
            return -1;
        }
        _fbuff = 0;
    }

    if (_buff != 0)
    {
        free(_buff);
        _buff = 0;
        _buff_size = 0;
    }

    return 0;
}

int dp_beginDraw()
{
    if (_fdev == 0)
    {
        return -1;
    }
    if (_fbuff != 0)
    {
        dp_endDraw();
    }

    _buff_size = _finfo.smem_len;

    _fbuff = (char *)mmap(0, _buff_size, PROT_READ | PROT_WRITE, MAP_SHARED, _fdev, 0);
    if (((int)_fbuff) == -1)
    {
        ERROR("map framebuffer device to memory failed!");
        return -1;
    }

    _buff = (char *)malloc(_buff_size);
    return _buff_size;
}

int dp_refresh()
{
    int rs;
    memcpy(_fbuff, _buff, _buff_size);

    // rs=ioctl(_fdev,FBIOGET_VSCREENINFO,&_vinfo);

    // INFO("refresh %d",rs);

    rs = msync(_fbuff, _buff_size, MS_SYNC);
    // // rs = msync(_fbuff, _buff_size, MS_INVALIDATE);
    if (rs)
    {
        ERROR("dp_refresh failed(%d)!", rs);
        return rs;
    }
    return 0;
}

int dp_fillScreen(int color)
{
    if (_vinfo.bits_per_pixel == 1)
    {
        memset(_buff, color == 0 ? 0 : 0xff, _buff_size);
    }
    else
    {
        int b = _vinfo.bits_per_pixel / 8;
        for (int i = 0; i < _vinfo.xres; i++)
        {
            memcpy(_buff + b * i, &color, b);
        }

        for (int i = 1; i < _vinfo.yres; i++)
        {
            memcpy(_buff + _finfo.line_length * i, _buff, _finfo.line_length);
        }
    }

    return 0;
}

int dp_point(int color, int x, int y)
{
    char *p;
    unsigned char mask;
    if (color)
    {
        color = 0xffffffff;
    }

    if (x >= _vinfo.xres || y >= _vinfo.yres)
    {
        ERROR("dp_point out of range, screen(%d,%d) point(%d,%d)", _vinfo.xres, _vinfo.yres, x, y);
        return -1;
    }

    // 位图
    if (_vinfo.bits_per_pixel == 1)
    {
        p = _buff + y * _finfo.line_length + x / 8;
        mask = 1 << (x % 8);
        if (color)
        {
            *p |= mask;
        }
        else
        {
            *p &= ~mask;
        }
    }
    else
    {
        p = _buff + y * _finfo.line_length + x * _vinfo.bits_per_pixel / 8;
        memcpy((void *)p, &color, _vinfo.bits_per_pixel / 8);
    }

    return 0;
}

int dp_line(int color, int x1, int y1, int x2, int y2)
{
    float r;
    int step;
    float x, y;

    // DEBUG("line (%d,%d)-(%d,%d)", x1, y1, x2, y2);

    step = x1 < x2 ? 1 : -1;
    r = 1.0 * (y2 - y1) / (x2 - x1);

    y = y1;
    for (x = x1; x != x2 + step; x += step, y += r)
    {
        dp_point(color, x, y);
    }

    step = y1 < y2 ? 1 : -1;
    r = 1.0 * (x2 - x1) / (y2 - y1);

    x = x1;
    for (y = y1; y != y2 + step; y += step, x += r)
    {
        dp_point(color, x, y);
    }
    return 0;
}

int dp_rectangle(int color, int x1, int y1, int x2, int y2)
{
    // DEBUG("rect (%d,%d)-(%d,%d)", x1, y1, x2, y2);
    dp_line(color, x1, y1, x2, y1);
    dp_line(color, x2, y1, x2, y2);
    dp_line(color, x2, y2, x1, y2);
    dp_line(color, x1, y2, x1, y1);
    return 0;
}

int dp_circle(int color, int x, int y, int r)
{
    int cnt;
    double step, sita = 0;

    cnt = r * 4;

    INFO("circle pos: (%d,%d) r: %d  count of point: %d", x, y, r, cnt);

    step = 2 * M_PI / cnt;

    while (cnt-- > 0)
    {
        dp_point(color, x + r * sin(sita), y + r * cos(sita));
        sita += step;
    }
    return 0;
}

void dp_fontInfo(FT_Face face)
{
    INFO("===font information===");
    INFO("name:%s", face->family_name);
    INFO("style:%s", face->style_name);
    INFO("face id: %ld, count: %ld", face->face_index, face->num_faces);
    INFO("unit per EM: %d", face->units_per_EM);
    INFO("count glyph: %ld, fix size: %d,charmap: %d", face->num_glyphs, face->num_fixed_sizes, face->num_charmaps);

    INFO("charmaps===");
    INFO("platformid: 0=>apple, 1=>macintosh, 2=>iso, 3=>microsoft, 4=>custom, 7=>adobe");
    for (int i = 0; i < face->num_charmaps; i++)
    {
        FT_CharMap charmap = face->charmaps[i];
        uint code = charmap->encoding;
        INFO("charmap: %c%c%c%c platform(%d) encoding(%d)", code >> 24, code >> 16, code >> 8, code, charmap->platform_id, charmap->encoding_id);
    }

    INFO("available sizes===");
    for (int i = 0; i < face->num_fixed_sizes; i++)
    {
        FT_Bitmap_Size bitmap = face->available_sizes[i];
        INFO("size %d x %d", bitmap.width, bitmap.height);
    }

    INFO("===font information===");
}

void dp_freeFont(FT_Face font)
{
    FT_Done_Face(font);
}

FT_Face dp_loadFont(char *fontFile)
{
    FT_Face face;

    // face = (FT_Face *)malloc(sizeof(face));
    _ft_error = FT_New_Face(_ft_lib, fontFile, 0, &face);
    if (_ft_error)
        ERROR("create font face failed(%d)!", _ft_error);

    _ft_error = FT_Select_Charmap(face, FT_ENCODING_UNICODE);

    dp_fontInfo(face);

    if (_ft_face == NULL)
    {
        _ft_face = face;
    }
    return face;
}

int dp_text(wchar_t *text, int color, int x, int y, FT_Face font, int fontsize)
{

    // _ft_error = FT_Init_FreeType(&_ft_lib);
    // if (_ft_error)
    // {
    //     ERROR("init freeType Library failed(%d)!", _ft_error);
    //     return -1;
    // }

    // _ft_error = FT_New_Face(_ft_lib,"/usr/share/fonts/truetype/moe/MoeStandardSong.ttf",0,&_ft_face);
    // _ft_error = FT_New_Face(_ft_lib, "resources/fonts/msyh.ttc", 0, &_ft_face);

    // _ft_error = FT_Select_Charmap(_ft_face, FT_ENCODING_UNICODE);
    _ft_error = FT_Set_Pixel_Sizes(font, 0, fontsize);

    // FT_Matrix matrix;
    // FT_Vector pen;

    // double a=M_PI/180*30;
    // matrix.xx = (FT_Fixed)( cos(a) * 0x10000L);
    // matrix.xy = (FT_Fixed)(-sin(a) * 0x10000L);
    // matrix.yx = (FT_Fixed)( sin(a) * 0x10000L);
    // matrix.yy = (FT_Fixed)( cos(a) * 0x10000L);

    // pen.x = x * 64;
    // pen.y = (64 - y) * 64;

    // FT_Set_Transform(_ft_face,&matrix,&pen);

    for (int i = 0; i < wcslen(text); i++)
    {
        _ft_error = FT_Load_Char(font, text[i], FT_LOAD_RENDER | FT_LOAD_MONOCHROME);
        // _ft_error = FT_Load_Char(_ft_face,L'A',FT_LOAD_RENDER | FT_LOAD_MONOCHROME);

        FT_GlyphSlot g = font->glyph;
        FT_Bitmap bitmap = g->bitmap;

        // INFO( "pixel mode: %d",bitmap.pixel_mode);
        // INFO( "width: %d heght: %d", bitmap.width, bitmap.rows);
        // INFO( "bytes of line: %d", bitmap.pitch);

        // INFO( "advance %d,%d", g->advance.x>>6, g->advance.y>>6);
        // INFO( "metrix %d,%d",g->metrics.horiAdvance/64, g->metrics.vertAdvance/64);
        // INFO( "padding: l=>%d t=>%d",g->bitmap_left,g->bitmap_top);
        // INFO( "metrix2 %d,%d",)
        // INFO( "H %d V %d", g->linearHoriAdvance/64,g->linearVertAdvance/64);

        // FT_BBox box;
        // FT_Glyph gg;
        // FT_Get_Glyph(g,&gg);

        // FT_Glyph_Get_CBox(gg,FT_GLYPH_BBOX_PIXELS,&box);
        // INFO( "(%d.%d)-(%d,%d)",box.xMin,box.yMin,box.xMax,box.yMax);
        // FT_Glyph_Get_CBox(gg,FT_GLYPH_BBOX_TRUNCATE,&box);
        // INFO( "(%d.%d)-(%d,%d)",box.xMin,box.yMin,box.xMax,box.yMax);
        // FT_Done_Glyph(gg);

        // 计算基于基线的偏差
        int offset_x, offset_y;
        int draw_color;
        offset_x = 0;
        offset_y = (g->metrics.vertAdvance >> 6) - g->bitmap_top;

        for (int r = 0; r < bitmap.rows; r++)
        {
            for (int c = 0; c < bitmap.width; c++)
            {

                if (bitmap.buffer[r * bitmap.pitch + c / 8] & (0x80 >> c % 8))
                {
                    draw_color = color;
                }
                else
                {
                    draw_color = ~color;
                }
                dp_point(draw_color, x + c + offset_x, y + r + offset_y);
            }
        }

        x += g->advance.x >> 6;
    }

    // _ft_error = FT_Done_Face(_ft_face);
    // _ft_error = FT_Done_FreeType(_ft_lib);
    return 0;
}

// #include <jpeglib.h>

// struct my_error_mgr {
//   struct jpeg_error_mgr pub;	/* "public" fields */

//   jmp_buf setjmp_buffer;	/* for return to caller */
// };
// typedef struct my_error_mgr * my_error_ptr;

// METHODDEF(void)
// my_error_exit (j_common_ptr cinfo)
// {
//   /* cinfo->err really points to a my_error_mgr struct, so coerce pointer */
//   my_error_ptr myerr = (my_error_ptr) cinfo->err;

//   /* Always display the message. */
//   /* We could postpone this until after returning, if we chose. */
//   (*cinfo->err->output_message) (cinfo);

//   /* Return control to the setjmp point */
//   longjmp(myerr->setjmp_buffer, 1);
// }

// int dp_image(char* file){
//     struct jpeg_decompress_struct cinfo;
//     FILE *infile;
//     struct my_error_mgr jerr;

//     if((infile = fopen(file,"rb")) == NULL){
//         ERROR( "can't open %s file",file);
//         return -1;
//     }

//     cinfo.err = jpeg_std_error(&jerr.pub);
//     jerr.pub.error_exit = my_error_exit;

//     if (setjmp(jerr.setjmp_buffer)) {
//     /* If we get here, the JPEG code has signaled an error.
//      * We need to clean up the JPEG object, close the input file, and return.
//      */
//         jpeg_destroy_decompress(&cinfo);
//         fclose(infile);
//         return 0;
//     }
//     jpeg_create_decompress(&cinfo);
//     jpeg_stdio_src(&cinfo,infile);

//     jpeg_read_header(&cinfo,TRUE);
//     jpeg_start_decompress(&cinfo);

//     // 读取数据

//     int row_stride = cinfo.output_width * cinfo.output_components;
//     // printf("output size: %d x %d",cinfo.output_width,cinfo.output_height);
//     // printf("output_components : %d",cinfo.output_components);

//   /* Make a one-row-high sample array that will go away when done with image */
//     JSAMPARRAY buffer = (*cinfo.mem->alloc_sarray)
// 		((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);
//     int line=0;
//     while (cinfo.output_scanline < cinfo.output_height) {
//         /* jpeg_read_scanlines expects an array of pointers to scanlines.
//         * Here the array is only one element long, but you could ask for
//         * more than one scanline at a time if that's more convenient.
//         */
//         (void) jpeg_read_scanlines(&cinfo, buffer, 1);
//         /* Assume put_scanline_someplace wants a pointer and sample count. */
//         for(int i=0;i<128;i++){
//             // if(buffer[0][i*3]+buffer[0][i*3+1]+buffer[0][i*3+2]){
//             if(buffer[0][i*3+1]){
//                 dp_point(1,i,line);
//                 // printf("x");
//             }else{
//                 dp_point(0,i,line);
//                 // printf(" ");
//             }
//             // printf("%x ",buffer[0][i*3]);
//         }
//         line++;
//         // printf("\n");
//         // printf("data: 0x%x 0x%x 0x%x\n",buffer[0][])
//         // put_scanline_someplace(buffer[0], row_stride);
//     }

//     jpeg_finish_decompress(&cinfo);
//     jpeg_destroy_decompress(&cinfo);
//     fclose(infile);
//     return 0;

// }

#include "bmp.h"

int dp_image_bmp(int drawcolor, char *file, int x, int y)
{
    FILE *infile;

    if ((infile = fopen(file, "rb")) == NULL)
    {
        ERROR("can't open %s file", file);
        return -1;
    }

    bmp_header_t bmpHeader;
    bmp_color_table_entry_t *pPalette = NULL;
    int8_t *pData = NULL, *pPixel;
    bmp_color_table_entry_t color, *pColor;

    fread(&bmpHeader, sizeof(bmpHeader), 1, infile);
    if (bmpHeader.data_offset > 54)
    {
        pPalette = (bmp_color_table_entry_t *)malloc(bmpHeader.data_offset - 54);
        fread(pPalette, bmpHeader.data_offset - 54, 1, infile);
    }

    pData = (int8_t *)malloc(bmpHeader.file_size - bmpHeader.data_offset);
    fread(pData, bmpHeader.file_size - bmpHeader.data_offset, 1, infile);
    fclose(infile);

    for (int r = 0; r < bmpHeader.height; r++)
    {
        for (int c = 0; c < bmpHeader.width; c++)
        {
            pPixel = pData + (c + (bmpHeader.height - 1 - r) * bmpHeader.width) * bmpHeader.bit_count / 8;
            // printf("%x ",pPixel);
            switch (bmpHeader.bit_count)
            {
            case 1:
                // printf("%x",(*pPixel >> (7 - c % 8)) & 0x1);
                pColor = &pPalette[(*pPixel >> (7 - c % 8)) & 0x1];
                memcpy(&color, pColor, sizeof(color));
                break;
            case 4:
                pColor = &pPalette[(*pPixel >> 4 * (1 - c % 2)) & 0xf];
                memcpy(&color, pColor, sizeof(color));
                break;
            case 8:
                pColor = &pPalette[*pPixel];
                memcpy(&color, pColor, sizeof(color));
                break;
            case 16:
                pColor = &pPalette[*pPixel + *(pPixel + 1) * 256];
                memcpy(&color, pColor, sizeof(color));
                break;
            case 24:
                color.blue = pPixel[0];
                color.green = pPixel[1];
                color.red = pPixel[2];
                color.reserved = 0;
                break;
            case 32:
                color.blue = pPixel[0];
                color.green = pPixel[1];
                color.red = pPixel[2];
                color.reserved = pPixel[3];
                break;
            }

            if ((color.red | color.green | color.blue) == 0)
            // if (color.red)
            {
                dp_point(~drawcolor, c + x, r + y);
                // printf(" ");
            }
            else
            {
                dp_point(drawcolor, c + x, r + y);
                // printf("x");
            }
        }
        // printf("\n");
    }

    if (pPalette != NULL)
    {
        free(pPalette);
        pPalette = NULL;
    }
    free(pData);
    return 0;
}

int dp_processbar_LR(int color, int x, int y, int len, float percent)
{
    dp_rectangle(color, x, y, x + len - 1, y + 4);
    if ((len - 4) * percent >= 1)
    {
        dp_line(color, x + 2, y + 2, x + 1 + (len - 4) * percent, y + 2);
    }
    return 0;
}

int dp_processbar_BT(int color, int x, int y, int len, float percent)
{
    dp_rectangle(color, x, y, x + 4, y - len + 1);
    if ((len - 4) * percent >= 1)
    {
        dp_line(color, x + 2, y - 2, x + 2, y -2 - (len - 4) * percent +1);
    }
    return 0;
}