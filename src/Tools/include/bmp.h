#ifndef LOADBMP_H_INCLUDED

#define LOADBMP_H_INCLUDED



typedef unsigned char BYTE;

typedef unsigned short WORD;

typedef unsigned long DWORD;

//typedef unsigned int DWORD; //64bit os


#pragma pack(1)

typedef struct {

    /*BITMAPFILEHEADER*/

    WORD bfType;

    DWORD bfSize;

    WORD bfReserved1;

    WORD bfReserved2;

    DWORD bfOffBits;

}BITMAPFILEHEADER;



#define FILEHEADSIZE 14



#define MAX(a,b) a>b?a:b

#define MIN(a,b) a<b?a:b



typedef struct {

    /*BITMAPINFOHEADER*/

    DWORD biSize;

    DWORD biWidth;

    DWORD biHeight;

    WORD biPlanes;

    WORD biBitCount;

    DWORD biCompression;

    DWORD biSizeImage;

    DWORD biXPelsPerMeter;

    DWORD biYPelsPerMeter;

    DWORD biClrUsed;

    DWORD biClrImportant;

}BITMAPINFOHEADER;



#define INFOHEADSIZE 40



typedef struct {

    BYTE rgbBlue;

    BYTE rgbGreen;

    BYTE rgbRed;

    BYTE rgbReserved;

}RGBQUAD;



typedef struct {

    BITMAPINFOHEADER bmiHeader;

    RGBQUAD bmiColors[1];

}BITMAPINFO;



BYTE *get_data (const char *name, int *width, int *height,int *bitc);
BYTE *encode_bmp(char *buff, int *size,int width, int height);
void release_bmp(BYTE *bmp_ptr);
int RawToBmp(BYTE *pIn,BYTE *pOut,int width, int height);


#endif // LOADBMP_H_INCLUDED