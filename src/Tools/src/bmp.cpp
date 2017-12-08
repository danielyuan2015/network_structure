#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <unistd.h>
#include "bmp.h"

static int ReadFileHeader(char *filepath, BITMAPFILEHEADER *bmfh);
static int ReadInfoHeader(char *filepath, BITMAPINFOHEADER *bmih);
static DWORD GetLineBytes(int width, int bitcount);
static BYTE *ReadPixelData(char *filepath,int *img_width,int *img_height,int *bitc);

DWORD GetLineBytes(int width, int bitcount)
{
    if (bitcount < 8)
        return (width+7)/8;
    else
        return (width*bitcount+31)/32*4;
}

int ReadFileHeader(char *filepath, BITMAPFILEHEADER *bmfh)
{
    FILE *fp;

    fp = fopen(filepath, "rb");
    if (!fp)
    {
        printf("Can't open file:%s\n", filepath);
        return -1;
    }

    fread(&bmfh->bfType,sizeof(WORD),1,fp);
    fread(&bmfh->bfSize,sizeof(DWORD),1,fp);
    fread(&bmfh->bfReserved1,sizeof(WORD),1,fp);
    fread(&bmfh->bfReserved2,sizeof(WORD),1,fp);
    fread(&bmfh->bfOffBits,sizeof(DWORD),1,fp);

    //printf("sizeof : %d\n", bmfh->bfSize);
    //printf("offbits : %d\n",bmfh->bfOffBits);

    fclose(fp);
    return 0;
}

int ReadInfoHeader(char *filepath, BITMAPINFOHEADER *bmih)
{
    FILE *fp;

    fp = fopen(filepath, "rb");
    if (!fp)
    {
        printf("Can't open file:%s\n", filepath);
        return -1;
    }

    fseek(fp,14,SEEK_SET);

    fread(&bmih->biSize,sizeof(DWORD),1,fp);
    fread(&bmih->biWidth,sizeof(DWORD),1,fp);
    fread(&bmih->biHeight,sizeof(DWORD),1,fp);
    fread(&bmih->biPlanes,sizeof(WORD),1,fp);
    fread(&bmih->biBitCount,sizeof(WORD),1,fp);
    fread(&bmih->biCompression,sizeof(DWORD),1,fp);
    fread(&bmih->biSizeImage,sizeof(DWORD),1,fp);
    fread(&bmih->biXPelsPerMeter,sizeof(DWORD),1,fp);
    fread(&bmih->biYPelsPerMeter,sizeof(DWORD),1,fp);
    fread(&bmih->biClrUsed,sizeof(DWORD),1,fp);
    fread(&bmih->biClrImportant,sizeof(DWORD),1,fp);

//    printf("size : %d\n", bmih->biSize);
//    printf("width : %d\n", bmih->biWidth);
//    printf("height : %d\n", bmih->biHeight);
//    printf("planes : %d\n", bmih->biPlanes);
//    printf("bitcount : %d\n", bmih->biBitCount);
//    printf("compression : %d\n", bmih->biCompression);
//    printf("sizeimage : %d\n", bmih->biSizeImage);
//    printf("XPelsPerMeter : %d\n", bmih->biXPelsPerMeter);
//    printf("YPelsPerMeter : %d\n", bmih->biYPelsPerMeter);
//    printf("ClrUsed : %d\n", bmih->biClrUsed);
//    printf("ClrImportant : %d\n", bmih->biClrImportant);

    fclose(fp);
    return 0;
}

BYTE *ReadPixelData(char *filepath,int *img_width,int *img_height,int *bitc)
{
    BITMAPFILEHEADER bmfh;
    BITMAPINFOHEADER bmih;
    BYTE *imgdata;
    FILE *fp;
    int n;
    int width;
    int height;
    int bitCount;
    int w,h;
    int count = 0;
	int bitcnt = 0;
    //long biSizeImage;
    DWORD dwLineBytes;
    BYTE *pRowAddr;
    BYTE *raw;
    BYTE pixel;

    ReadFileHeader(filepath,&bmfh);
    ReadInfoHeader(filepath,&bmih);

    width = bmih.biWidth;
    height = bmih.biHeight;
    *img_width = width;
    *img_height = height;

    bitCount = bmih.biBitCount;
	*bitc = bitCount;
    //biSizeImage = bmih.biSizeImage;
    dwLineBytes = GetLineBytes(width,bitCount);
    //printf("dwLineBytes : %d\n", dwLineBytes);

    fp = fopen(filepath, "rb");
    if (!fp)
    {
        printf("Can't open file:%s\n", filepath);
        return NULL;
    }

    fseek(fp,bmfh.bfOffBits,SEEK_SET);

    imgdata = (BYTE *)malloc(dwLineBytes*height*sizeof(BYTE));
    if (!imgdata)
    {
        printf("Can't allocate memory for the pixel data.\n");
        return NULL;
    }

    n=fread(imgdata,dwLineBytes*height*sizeof(BYTE),1,fp);
    if (n == 0)
    {
        if (ferror(fp))
        {
            printf("Can't read the pixel data.\n");
            fclose(fp);
            return NULL;
        }
    }

    raw = (BYTE *)malloc(width*height*sizeof(BYTE));
    if (bitCount == 1)
    {
        for(h=0; h<height; h++)
        {
            pRowAddr = imgdata+dwLineBytes*(height-1-h);
            for(w=0; w<dwLineBytes; w++)
            {
                pixel = *(pRowAddr+w);
                BYTE shiftbits;
                shiftbits = 8;
                if ((dwLineBytes<<3) != width)
                {
                    if (w == (dwLineBytes-1))
                    {
                        shiftbits = width-(((dwLineBytes-1)<<3));
                    }
                }

                BYTE i;
                for(i=0; i<shiftbits; i++)
                {
                    if((pixel>>(shiftbits-1-i))&0x1)
                        raw[count++] = 0xff;
                    else
                        raw[count++] = 0x0;
                }
            }
        }
        //printf("PixelsCount = %d\n",count);
    }
    else
    {
        for(h=0;h<height;h++)
        {
            pRowAddr = imgdata+dwLineBytes*(height-1-h);
            for(w=0;w<width;w++)
            {
                raw[count++] = *(pRowAddr+(w<<2));
            }
        }

    }

    fclose(fp);
    free(imgdata);

    return raw;
}

BYTE *get_data (const char *name,
                      int *width, int *height,int *bitc)
{
    //BITMAPFILEHEADER BitmapFileHeader;
    //BITMAPINFOHEADER BitmapInfoHeader;

    char filepath[20] = {0};
    memset(filepath, 0, sizeof(filepath));
    memcpy(filepath, name, strlen(name));
    //ReadFileHeader(filepath, &BitmapFileHeader);
    //ReadInfoHeader(filepath, &BitmapInfoHeader);
    BYTE *raw = ReadPixelData(filepath,width,height,bitc);

    return raw;
}

void WriteFileHeader(BITMAPFILEHEADER *bmfh, int width, int height)
{
	int image_size = width*height;
	int clr_table_size = 256*4;
	
	bmfh->bfType = 0x4D42;
	bmfh->bfSize = FILEHEADSIZE+INFOHEADSIZE+clr_table_size+image_size;
	bmfh->bfReserved1 = 0;
	bmfh->bfReserved2 = 0;
	bmfh->bfOffBits = FILEHEADSIZE+INFOHEADSIZE+clr_table_size;
}

void WriteInfoHeader(BITMAPINFOHEADER *bmih, int width, int height)
{
	bmih->biSize = INFOHEADSIZE;
	bmih->biWidth = width;
	bmih->biHeight = height;
	bmih->biPlanes = 1;
	bmih->biBitCount = 8;
	
	bmih->biCompression = 0;
	bmih->biSizeImage = 0;
	bmih->biXPelsPerMeter = 0;
	bmih->biYPelsPerMeter = 0;
	bmih->biClrUsed = 0;
	bmih->biClrImportant = 0;
}

void release_bmp(BYTE *bmp_ptr)
{
	if(bmp_ptr)
		free(bmp_ptr);
}

BYTE *encode_bmp(char *buff, int *size,int width, int height)
{
	BYTE *bmp_ptr;
	BYTE *ptr;
	BITMAPFILEHEADER bmfh;
	BITMAPINFOHEADER bmih;
	int i,j;
	int curr_line_addr;
	int clr_table_size = 256*4;
	RGBQUAD clr_table[256];
	
	WriteFileHeader(&bmfh, width, height);
	WriteInfoHeader(&bmih, width, height);
	
	if (bmfh.bfSize) {
		ptr = bmp_ptr = (BYTE *)malloc(bmfh.bfSize);
		*size = bmfh.bfSize;
		if (bmp_ptr < 0)
			return NULL;
	}
	
	for(i=0;i<256;i++)
	{
		clr_table[i].rgbBlue = i;
		clr_table[i].rgbGreen = i;
		clr_table[i].rgbRed = i;
		clr_table[i].rgbReserved = 0;
	}
	
	memcpy(bmp_ptr,&bmfh,FILEHEADSIZE);
	bmp_ptr += FILEHEADSIZE;
	memcpy(bmp_ptr,&bmih,INFOHEADSIZE);
	bmp_ptr += INFOHEADSIZE;
	memcpy(bmp_ptr,clr_table,clr_table_size);
	bmp_ptr += clr_table_size;
	
	for (i = height-1;i >= 0; i--)
	{
		curr_line_addr = i*width;
		for (j = 0; j < width; j++)
		{
			*(bmp_ptr++) = buff[curr_line_addr+j];
		}
	}
	
	return ptr;
}

int RawToBmp(BYTE *pIn,BYTE *pOut,int width, int height)
{
	BYTE *bmp_ptr = NULL;
	//char *ptr;
	int size = 0;
	int curr_line_addr;
	int clr_table_size = 256*4;
	RGBQUAD clr_table[256];
	int i,j;

	BITMAPFILEHEADER bmfh;
	BITMAPINFOHEADER bmih;

	if((NULL == pIn)&&(NULL == pOut))
		return -1;

	bmp_ptr = pOut;

	WriteFileHeader(&bmfh, width, height);
	WriteInfoHeader(&bmih, width, height);

	if (bmfh.bfSize) {
		size = bmfh.bfSize;
	}

	for(i=0;i<256;i++) {
		clr_table[i].rgbBlue = i;
		clr_table[i].rgbGreen = i;
		clr_table[i].rgbRed = i;
		clr_table[i].rgbReserved = 0;
	}

	memcpy(bmp_ptr,&bmfh,FILEHEADSIZE);
	bmp_ptr += FILEHEADSIZE;
	memcpy(bmp_ptr,&bmih,INFOHEADSIZE);
	bmp_ptr += INFOHEADSIZE;
	memcpy(bmp_ptr,clr_table,clr_table_size);
	bmp_ptr += clr_table_size;

	for (i = height-1;i >= 0; i--)
	{
		curr_line_addr = i*width;
		for (j = 0; j < width; j++)
		{
			*(bmp_ptr++) = pIn[curr_line_addr+j];
		}
	}
	return size;
}
