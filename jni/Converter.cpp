#include <Converter.h>

#define LOG_TAG "Converter"

struct ima_error_mgr {
    struct jpeg_error_mgr pub;  /* "public" fields */
    jmp_buf setjmp_buffer;      /* for return to caller */
    char* buffer;               /* error message <CSC>*/
};

typedef ima_error_mgr *ima_error_ptr;


/*
 * Here's the routine that will replace the standard error_exit method:
 */

//METHODDEF
void ima_jpeg_error_exit (j_common_ptr cinfo)
{
    // cinfo->err really points to a my_error_mgr struct, so coerce pointer
    ima_error_ptr myerr = (ima_error_ptr) cinfo->err;

    char buffer[JMSG_LENGTH_MAX];

    // Create the message
    myerr->pub.format_message (cinfo, myerr->buffer);

    // Send it to stderr, adding a newline
    //        AfxMessageBox(buffer);

    // Return control to the setjmp point
    longjmp(myerr->setjmp_buffer, 1);
}


IplImage* Converter::loadJPG(const char* filename)
{
    bool bGray = false;
  /* This struct contains the JPEG decompression parameters and pointers to
	* working space (which is allocated as needed by the JPEG library).
	*/
    struct jpeg_decompress_struct cinfo;
    /* We use our private extension JPEG error handler. */

    struct ima_error_mgr jerr;
    //  struct jpeg_error_mgr jerr;
    /* More stuff */
    FILE * infile;		/* source file */
    JSAMPARRAY buffer;		/* Output row buffer */
    int row_stride;		/* physical row width in output buffer */

    /* In this example we want to open the input file before doing anything else,
     * so that the setjmp() error recovery below can assume the file is open.
     * VERY IMPORTANT: use "b" option to fopen() if you are on a machine that
     * requires it in order to read binary files.
     */

    if ((infile = fopen(filename, "rb")) == NULL) {
        //fprintf(stderr, "can't open %s\n", filename);
        return 0;
    }

    /* Step 1: allocate and initialize JPEG decompression object */
    /* We set up the normal JPEG error routines, then override error_exit. */
    cinfo.err = jpeg_std_error(&jerr.pub);
    jerr.pub.error_exit = ima_jpeg_error_exit;

    /* Establish the setjmp return context for my_error_exit to use. */
    if (setjmp(jerr.setjmp_buffer)) {
        /* If we get here, the JPEG code has signaled an error.
         * We need to clean up the JPEG object, close the input file, and return.
         */
        jpeg_destroy_decompress(&cinfo);
        fclose(infile);
        return 0;
    }
    /* Now we can initialize the JPEG decompression object. */
    jpeg_create_decompress(&cinfo);

    /* Step 2: specify data source (eg, a file) */
    jpeg_stdio_src(&cinfo, infile);

    /* Step 3: read file parameters with jpeg_read_header() */
    (void) jpeg_read_header(&cinfo, true);

    /* Step 4: set parameters for decompression */
    //  printf("info %d %d %d CS %d ", cinfo.image_width, cinfo.image_height, cinfo.output_components, cinfo.jpeg_color_space);
    if (cinfo.jpeg_color_space!=JCS_GRAYSCALE) {
        cinfo.quantize_colors = TRUE;
        cinfo.desired_number_of_colors = 128;
    }
    /* Step 5: Start decompressor */
    jpeg_start_decompress(&cinfo);

    /* We may need to do some setup of our own at this point before reading
     * the data.  After jpeg_start_decompress() we have the correct scaled
     * output image dimensions available, as well as the output colormap
     * if we asked for color quantization.
     */

    IplImage* cvimg;
    if (cinfo.jpeg_color_space==JCS_GRAYSCALE)
    {
        bGray = true;
        //	  CreateImage(cinfo.image_width, cinfo.image_height, 8);
        //      Create(cinfo.image_width, cinfo.image_height, 8);
        cvimg = cvCreateImage(cvSize(cinfo.image_width,cinfo.image_height), 8, 1);
    }
    else {
        //CreateImage(cinfo.image_width, cinfo.image_height, 24);
        //Create(cinfo.image_width, cinfo.image_height, 24);
        cvimg = cvCreateImage(cvSize(cinfo.image_width,cinfo.image_height), 8, 3);
    }

    /* JSAMPLEs per row in output buffer */
    row_stride = cinfo.output_width * cinfo.output_components;
    //  byte* buf2 = new byte[row_stride];
    //  printf("NCMPS cmp [%d %d %d]", cinfo.output_components, cinfo.actual_number_of_colors,row_stride);

    /* Make a one-row-high sample array that will go away when done with image */
    buffer = (*cinfo.mem->alloc_sarray)
        ((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);

    /* Step 6: while (scan lines remain to be read) */
    /*           jpeg_read_scanlines(...); */

    /* Here we use the library's state variable cinfo.output_scanline as the
     * loop counter, so that we don't have to keep track ourselves.
     */
    int line, col;

    if(bGray)
    {
        BwImage ptr(cvimg);
        line = 0;
        while (cinfo.output_scanline < cinfo.output_height) 
        {
            (void) jpeg_read_scanlines(&cinfo, buffer, 1);
            /* Assume put_scanline_someplace wants a pointer and sample count. */
            memcpy(ptr[line++], buffer[0], row_stride);
        }
    }
    else 
    {
        RgbImage ptrColor(cvimg);
        line = 0;

        if(cinfo.output_components == 3)
        {
            while (cinfo.output_scanline < cinfo.output_height) 
            {
                (void) jpeg_read_scanlines(&cinfo, buffer, 1);
                /* Assume put_scanline_someplace wants a pointer and sample count. */
                memcpy(ptrColor[line++], buffer[0], row_stride);
            }
        }
        else
        {
            while (cinfo.output_scanline < cinfo.output_height) 
            {
                (void) jpeg_read_scanlines(&cinfo, buffer, 1);
                /* Assume put_scanline_someplace wants a pointer and sample count. */
                for(col=0 ; col<row_stride ; col++) 
                {
                    ptrColor[line][col].r = cinfo.colormap[0][buffer[0][col]];
                    ptrColor[line][col].g = cinfo.colormap[1][buffer[0][col]];
                    ptrColor[line][col].b = cinfo.colormap[2][buffer[0][col]];
                }
                line++;
            }
        }
    }


    /* Step 7: Finish decompression */
    (void) jpeg_finish_decompress(&cinfo);
    /* We can ignore the return value since suspension is not possible
     * with the stdio data source.
     */

    /* Step 8: Release JPEG decompression object */

    /* This is an important step since it will release a good deal of memory. */
    jpeg_destroy_decompress(&cinfo);

    /* After finish_decompress, we can close the input file.
     * Here we postpone it until after no more JPEG errors are possible,
     * so as to simplify the setjmp error logic above.  (Actually, I don't
     * think that jpeg_destroy can do an error exit, but why assume anything...)
     */
    fclose(infile);
    /* At this point you may want to check to see whether any corrupt-data
     * warnings occurred (test whether jerr.pub.num_warnings is nonzero).
     */

    /* And we're done! */
    return cvimg;
}

bool Converter::saveJPG(const char* filename, IplImage* cvimg)
{
    bool bGray = false;
    int col, i;

    struct jpeg_compress_struct cinfo;
    /* This struct represents a JPEG error handler.  It is declared separately
     * because applications often want to supply a specialized error handler
     * (see the second half of this file for an example).  But here we just
     * take the easy way out and use the standard error handler, which will
     * print a message on stderr and call exit() if compression fails.
     * Note that this struct must live as long as the main JPEG parameter
     * struct, to avoid dangling-pointer problems.
     */
    struct jpeg_error_mgr jerr;
    /* More stuff */
    FILE * outfile;		/* target file */
    int row_stride;		/* physical row width in image buffer */
    JSAMPARRAY buffer;		/* Output row buffer */

    IplImage* simg;

    /* Step 1: allocate and initialize JPEG compression object */

    /* We have to set up the error handler first, in case the initialization
     * step fails.  (Unlikely, but it could happen if you are out of memory.)
     * This routine fills in the contents of struct jerr, and returns jerr's
     * address which we place into the link field in cinfo.
     */
    cinfo.err = jpeg_std_error(&jerr);
    /* Now we can initialize the JPEG compression object. */
    jpeg_create_compress(&cinfo);

    /* Step 2: specify data destination (eg, a file) */
    /* Note: steps 2 and 3 can be done in either order. */

    /* Here we use the library-supplied code to send compressed data to a
     * stdio stream.  You can also write your own code to do something else.
     * VERY IMPORTANT: use "b" option to fopen() if you are on a machine that
     * requires it in order to write binary files.
     */
    if ((outfile = fopen(filename, "wb")) == NULL) {
        //	 fprintf(stderr, "can't open %s\n", filename);
        return FALSE;
    }
    jpeg_stdio_dest(&cinfo, outfile);

    /* Step 3: set parameters for compression */

    /* First we supply a description of the input image.
     * Four fields of the cinfo struct must be filled in:
     */
    cinfo.image_width = cvimg->width; 	// image width and height, in pixels
    cinfo.image_height = cvimg->height;

    if(cvimg->nChannels==1)
    {
        cinfo.input_components = 1; 	// # of color components per pixel
        cinfo.in_color_space = JCS_GRAYSCALE; 	/* colorspace of input image */
        bGray = true;
    } else {
        simg = cvCreateImage(cvGetSize(cvimg),8,3);
        if( cvimg->nChannels==3 ) {
            cvCopy(cvimg, simg);
        } else if( cvimg->nChannels==4 ) { 
            cvCvtColor(cvimg,simg,CV_RGBA2BGR);
        }
        cinfo.input_components = 3; 	// # of color components per pixel
        cinfo.in_color_space = JCS_RGB; 	/* colorspace of input image */
    }

    //  printf("info %d %d %d %d ", cinfo.image_width, cinfo.image_height, cinfo.input_components, cinfo.in_color_space);
    /* Now use the library's routine to set default compression parameters.
     * (You must set at least cinfo.in_color_space before calling this,
     * since the defaults depend on the source color space.)
     */
    jpeg_set_defaults(&cinfo);
    /* Now you can set any non-default parameters you wish to.
     * Here we just illustrate the use of quality (quantization table) scaling:
     */
    //jpeg_set_quality(&cinfo, 30, TRUE /* limit to baseline-JPEG values */);

    /* Step 4: Start compressor */

    /* TRUE ensures that we will write a complete interchange-JPEG file.
     * Pass TRUE unless you are very sure of what you're doing.
     */
    //  puts("begining");
    jpeg_start_compress(&cinfo, TRUE);

    /* Step 5: while (scan lines remain to be written) */
    /*           jpeg_write_scanlines(...); */

    /* Here we use the library's state variable cinfo.next_scanline as the
     * loop counter, so that we don't have to keep track ourselves.
     * To keep things simple, we pass one scanline per call; you can pass
     * more if you wish, though.
     */
    row_stride = cvimg->width*cinfo.input_components;	/* JSAMPLEs per row in image_buffer */

    buffer = (*cinfo.mem->alloc_sarray)
        ((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);

    int line = 0;
    if (bGray) 
    {
        BwImage ptr(cvimg);

        while (cinfo.next_scanline < cinfo.image_height) 
        {
            memcpy(buffer[0], ptr[line++], row_stride);
            (void) jpeg_write_scanlines(&cinfo, buffer, 1);
        }
    } else {
        RgbImage ptrColor(simg);

        while (cinfo.next_scanline < cinfo.image_height) {
            for(col=0, i=0 ; i<row_stride ; col++, i+=3) {
                buffer[0][i] = ptrColor[line][col].r;
                buffer[0][i+1] = ptrColor[line][col].g;
                buffer[0][i+2] = ptrColor[line][col].b;
            }
            line++;
            (void) jpeg_write_scanlines(&cinfo, buffer, 1);
        }

        cvReleaseImage(&simg);
    }

    /* Step 6: Finish compression */
    jpeg_finish_compress(&cinfo);
    /* After finish_compress, we can close the output file. */
    fclose(outfile);

    /* Step 7: release JPEG compression object */
    /* This is an important step since it will release a good deal of memory. */
    jpeg_destroy_compress(&cinfo);

    /* And we're done! */
    return true;
}

void Converter::saveCVIMG(const char* path, const IplImage* cvimg) {
    IplImage* img = cvCreateImage( cvGetSize(cvimg), 8, 4 );
    int nchs = cvimg->nChannels;
    LOGI("saveCVIMG:: %d channels", nchs);
    switch(nchs) {
        case 1:
            cvCvtColor(cvimg, img, CV_GRAY2RGBA);
            break;
        case 3:
            cvCvtColor(cvimg, img, CV_BGR2RGBA);
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


