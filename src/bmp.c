#include <linux/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "log.h"
#include "bmp.h"

void showBmpHeaderInfo(bmp_header_t * hbmp){
    printf("char %c%c\n",hbmp->signature[0],hbmp->signature[1]);
    printf("size: %d offset: %d\n",hbmp->file_size,hbmp->data_offset);
    printf("screen %d x %d\n",hbmp->width,hbmp->height);
    printf("planes: %d bit_count: %d\n",hbmp->planes,hbmp->bit_count);
    printf("compress: %d\n",hbmp->compression);
    printf("data size: %d\n",hbmp->image_size);
    printf("pixel per M: %d x %d\n",hbmp->x_pixels_per_m,hbmp->y_pixels_per_m);
    printf("used: %d import: %d\n",hbmp->colors_used,hbmp->colors_important);
}

void showBmpInfo(char* bmpfile){
    FILE *infile;

    if((infile = fopen(bmpfile,"rb")) == NULL){
        ERROR( "can't open %s file\n",bmpfile);
        return ;
    }

    bmp_image_t bmp;
    bmp_header_t *hbmp;

    memset(&bmp,0,sizeof(bmp));
    hbmp = &bmp.header;
    // printf("struct size: %d\n",sizeof(bmp.header));
    fread(hbmp,sizeof(bmp.header),1,infile);

    showBmpHeaderInfo(hbmp);
    // printf("char %c%c\n",hbmp->signature[0],hbmp->signature[1]);
    // printf("size: %d offset: %d size2: %d\n",hbmp->file_size,hbmp->data_offset,hbmp->size);
    // printf("screen %d x %d\n",hbmp->width,hbmp->height);
    // printf("planes: %d bit_count: %d\n",hbmp->planes,hbmp->bit_count);
    // printf("compress: %d\n",hbmp->compression);
    // printf("data size: %d\n",hbmp->image_size);
    // printf("pixel per M: %d x %d\n",hbmp->x_pixels_per_m,hbmp->y_pixels_per_m);
    // printf("used: %d import: %d\n",hbmp->colors_used,hbmp->colors_important);

    // 调色板信息
    int planeBytes;
    bmp_color_table_entry_t *plane;

    planeBytes=hbmp->data_offset-sizeof(bmp.header);
    plane = (bmp_color_table_entry_t*)malloc(planeBytes);

    fread(plane,sizeof(bmp_color_table_entry_t),planeBytes/sizeof(bmp_color_table_entry_t),infile);
    for(int i=0;i<planeBytes/sizeof(bmp_color_table_entry_t);i++){
        printf("%d : %x,%x,%x,%x\n",i,plane[i].red,plane[i].green,plane[i].blue,plane[i].reserved);
    }

    int s=hbmp->file_size-hbmp->data_offset;
    char *pdata;
    pdata = (char *)malloc(s);
    fread(pdata,s,1,infile);

    // char *p;
    // p=pdata;

    // for(int r=0;r<hbmp->height;r++){
    //     for(int c=0;c<hbmp->width;c++){
    //         p=pdata+(r*16)+c/8;

    //         // p=pdata+hbmp->bit_count/8*(c+(hbmp->height-r-1)*hbmp->width);
    //         if(*p & (1<< c%8)){
    //             // dp_point(1,c,r);
    //             printf("xx");
    //         }else{
    //             // dp_point(0,c,r);
    //             printf("  ");
    //         }
    //         // p+=hbmp->bit_count/8;
    //     }
    //     printf("\n");
    // }

    free(plane);
    free(pdata);
    fclose(infile);
}

int convert2bitPic(char* srcFile,char* destFile){
    FILE *fsrc=NULL,*fdest=NULL;

    if ((fsrc = fopen(srcFile,"rb"))==NULL){
        ERROR("can't open file [%s] (%d)\n",srcFile,fsrc);
        return -1;
    }

    if((fdest = fopen(destFile,"wr+"))==NULL){
        ERROR("can't write file [%s]\n",destFile);
        return -1;
    }

    bmp_header_t srcHeader;
    bmp_color_table_entry_t * srcPalette=NULL;
    __u8 * srcData=NULL,*pSrcPixel;
    __u8 * destData=NULL,*pDestPixel;

    bmp_image_t * destImg;

    fread(&srcHeader,sizeof(srcHeader),1,fsrc);
    if(srcHeader.data_offset > 54){
        // printf("read palette(%d)\n",srcHeader.data_offset-54);
        srcPalette=(bmp_color_table_entry_t *)malloc(srcHeader.data_offset-54);
        fread(srcPalette,srcHeader.data_offset-54,1,fsrc);
    }

    srcData = (__u8 *) malloc(srcHeader.file_size-srcHeader.data_offset);
    fread(srcData,srcHeader.file_size-srcHeader.data_offset,1,fsrc);
    fclose(fsrc);

    destImg = (bmp_image_t *) malloc(sizeof(bmp_image_t)+2*sizeof(bmp_color_table_entry_t));
    memset(destImg,0,sizeof(bmp_image_t)+2*sizeof(bmp_color_table_entry_t));

    destImg->header.signature[0]='B';
    destImg->header.signature[1]='M';
    destImg->header.size=40;
    destImg->header.width=srcHeader.width;
    destImg->header.height=srcHeader.height;
    destImg->header.planes=1;
    destImg->header.bit_count=1;
    // destImg->header.compression=0;
    // destImg->header.image_size=0;
    // destImg->header.x_pixels_per_m=0;
    // destImg->header.y_pixels_per_m=0;
    // destImg->header.colors_used=0;
    // destImg->header.colors_important=0;
    destImg->header.data_offset=54+8;
    destImg->header.file_size=destImg->header.data_offset+ceil(destImg->header.width*destImg->header.bit_count * destImg->header.height / 32)*4;;

    destImg->color_table[0].blue=0;
    destImg->color_table[0].green=0;
    destImg->color_table[0].red=0;
    
    destImg->color_table[1].blue=0xff;
    destImg->color_table[1].green=0xff;
    destImg->color_table[1].red=0xff;

    destData = (__u8 *) malloc(destImg->header.file_size-destImg->header.data_offset);

    // showBmpHeaderInfo(&srcHeader);

    for(int r=0;r<srcHeader.height;r++){
        for(int c=0;c<srcHeader.width;c++){
            pSrcPixel=srcData+(srcHeader.width * r + c) * srcHeader.bit_count /8;
            pDestPixel=destData+(srcHeader.width * r + c) /8;

            bmp_color_table_entry_t * pcolor;
            bmp_color_table_entry_t color;
            if(srcHeader.bit_count == 1){
                pcolor = &srcPalette[(*pSrcPixel >> (7-c%8)) & 0x1];
                // pcolor = &srcPalette[*pSrcPixel & (0x80 >> c%8) ? 1 : 0];
                memcpy(&color,pcolor,sizeof(color));
            }else if(srcHeader.bit_count == 4){
                pcolor = &srcPalette[(*pSrcPixel >> 4*(1-c%2)) & 0xf];
                memcpy(&color,pcolor,sizeof(color));
            }else if(srcHeader.bit_count == 8){
                pcolor = &srcPalette[*pSrcPixel];
                memcpy(&color,pcolor,sizeof(color));
            }else if(srcHeader.bit_count == 16){
                // printf("%x %x_",*pSrcPixel,*(pSrcPixel+1));
                pcolor = &srcPalette[*pSrcPixel + *(pSrcPixel+1)*256];
                memcpy(&color,pcolor,sizeof(color));
            }else if(srcHeader.bit_count == 24){
                color.blue=pSrcPixel[0];
                color.green=pSrcPixel[1];
                color.red=pSrcPixel[2];
                color.reserved=0;
            }else if(srcHeader.bit_count == 32){
                color.blue=pSrcPixel[0];
                color.green=pSrcPixel[1];
                color.red=pSrcPixel[2];
                color.reserved=pSrcPixel[3];
            }

            if(color.red){
                // printf("x");
                (*pDestPixel) |=0x80 >> c%8;
            }else{
                // printf(" ");
                (*pDestPixel) &=~(0x80 >> c%8);
            }
        }
        // printf("\n");
    }

    fwrite(destImg,sizeof(bmp_image_t)+2*sizeof(bmp_color_table_entry_t),1,fdest);
    fwrite(destData,destImg->header.file_size-destImg->header.data_offset,1,fdest);
    fclose(fdest);

    if(srcPalette != NULL){
        free(srcPalette);
        srcPalette=NULL;
    }
    free(srcData);
    free(destData);
    free(destImg);

    return 0;
}




