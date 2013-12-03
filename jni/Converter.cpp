#include <Converter.h>

#define LOG_TAG "Converter"

void Converter::saveCVIMG(const char* path, const IplImage* cvimg) {
    IplImage* img = cvCreateImage( cvGetSize(cvimg), 8, 4 );
    int nchs = cvimg->nChannels;
    LOGI("saveCVIMG:: %d channels", nchs);
    switch(nchs) {
        case 1:
            cvCvtColor(cvimg, img, CV_GRAY2BGRA);
            break;
        case 3:
            cvCvtColor(cvimg, img, CV_BGR2BGRA);
            break;
        case 4:
            cvCopy(cvimg, img);
            break;
        default:
            LOGE("CVIMG has different channels!! It's not saved correctly");
            break;
    }

    SkBitmap* bitmap = new SkBitmap;
    bitmap->setConfig(SkBitmap::kARGB_8888_Config, img->width, img->height);
    char *p;
    if( bitmap->allocPixels() ) {
        p = (char *)bitmap->getPixels();
        memcpy( p, img->imageData, img->imageSize );
    }

/*
    FILE* fp = fopen("sdcard/_bmp.txt", "wb"); 
    for(int i=0; i<img->imageSize; i++) {
        fputc(img->imageData[i],fp);
    }
    fflush(fp);
    fclose(fp);
*/
    saveSkBitmapToBMPFile(*bitmap, path);

    delete bitmap;

    cvReleaseImage(&img);
}

void Converter::saveSkBitmapToBMPFile(const SkBitmap& skBitmap, const char* path) {
    typedef unsigned char UINT8; 
    typedef signed char SINT8; 
    typedef unsigned short UINT16; 
    typedef signed short SINT16; 
    typedef unsigned int UINT32; 
    typedef signed int SINT32; 

    struct BMP_FILEHDR // BMP file header 
    { 
        UINT32 bfSize; // size of file 
        UINT16 bfReserved1; 
        UINT16 bfReserved2; 
        UINT32 bfOffBits; // pointer to the pixmap bits 
    }; 

    struct BMP_INFOHDR // BMP information header 
    { 
        UINT32 biSize; // size of this struct 
        UINT32 biWidth; // pixmap width 
        UINT32 biHeight; // pixmap height 
        UINT16 biPlanes; // should be 1 
        UINT16 biBitCount; // number of bits per pixel 
        UINT32 biCompression; // compression method 
        UINT32 biSizeImage; // size of image 
        UINT32 biXPelsPerMeter; // horizontal resolution 
        UINT32 biYPelsPerMeter; // vertical resolution 
        UINT32 biClrUsed; // number of colors used 
        UINT32 biClrImportant; // number of important colors 
    }; 

    int bmpWidth = skBitmap.width(); 
    int bmpHeight = skBitmap.height();
    int stride = skBitmap.rowBytes(); 
    char* m_pmap = (char*)skBitmap.getPixels(); 
    //virtual PixelFormat& GetPixelFormat() =0; //assume pf is ARGB; 
    FILE* fp = fopen(path, "wb"); 
    if(!fp){ 
        LOGE("saveSkBitmapToBMPFile: fopen %s Error!\n", path); 
    } 
    SINT32 bpl=bmpWidth*4; 
    // BMP file header. 
    BMP_FILEHDR fhdr; 
    fputc('B', fp); 
    fputc('M', fp); 
    fhdr.bfReserved1=fhdr.bfReserved2=0; 
    fhdr.bfOffBits=14+40; // File header size + header size. 
    fhdr.bfSize=fhdr.bfOffBits+bpl*bmpHeight; 
    fwrite(&fhdr, 1, 12, fp); 

    // BMP header. 
    BMP_INFOHDR bhdr; 
    bhdr.biSize=40; 
    bhdr.biBitCount=32; 
    bhdr.biCompression=0; // RGB Format. 
    bhdr.biPlanes=1; 
    bhdr.biWidth=bmpWidth; 
    bhdr.biHeight=bmpHeight; 
    bhdr.biClrImportant=0; 
    bhdr.biClrUsed=0; 
    bhdr.biXPelsPerMeter=2384; 
    bhdr.biYPelsPerMeter=2384; 
    bhdr.biSizeImage=bpl*bmpHeight; 
    fwrite(&bhdr, 1, 40, fp); 

    // BMP data. 
    //for(UINT32 y=0; y<m_height; y++) 
    for(SINT32 y=bmpHeight-1; y>=0; y--) 
    { 
        SINT32 base=y*stride; 
        for(SINT32 x=0; x<(SINT32)bmpWidth; x++) 
        { 
            UINT32 i=base+x*4;
            UINT32 pixelData = *(UINT32*)(m_pmap+i);
            UINT8 b1=BitmapColorGetB(pixelData);
            UINT8 g1=BitmapColorGetG(pixelData);
            UINT8 r1=BitmapColorGetR(pixelData);
            UINT8 a1=BitmapColorGetA(pixelData);
            r1=r1*a1/255; 
            g1=g1*a1/255; 
            b1=b1*a1/255; 
            UINT32 temp=(a1<<24)|(r1<<16)|(g1<<8)|b1;//a bmp pixel in little endian is B¡¢G¡¢R¡¢A 
            fwrite(&temp, 4, 1, fp); 
        } 
    } 
    fflush(fp); 
    fclose(fp);
}
