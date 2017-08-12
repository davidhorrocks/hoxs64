#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <tchar.h>


#define RWPNG_TEXT_TITLE    0x01
#define RWPNG_TEXT_AUTHOR   0x02
#define RWPNG_TEXT_DESC     0x04
#define RWPNG_TEXT_COPY     0x08
#define RWPNG_TEXT_EMAIL    0x10
#define RWPNG_TEXT_URL      0x20

#define RWPNG_TEXT_TITLE_OFFSET        0
#define RWPNG_TEXT_AUTHOR_OFFSET      72
#define RWPNG_TEXT_COPY_OFFSET     (2*72)
#define RWPNG_TEXT_EMAIL_OFFSET    (3*72)
#define RWPNG_TEXT_URL_OFFSET      (4*72)
#define RWPNG_TEXT_DESC_OFFSET     (5*72)
typedef unsigned char   uch;
typedef unsigned short  ush;
typedef unsigned long   ulg;

typedef struct _png_mainprog_info {
    double gamma;
    long width;
    long height;
    time_t modtime;
    FILE *infile;
    FILE *outfile;
    void *png_ptr;
    void *info_ptr;
    uch *image_data;
    uch **row_pointers;
    char *title;
    char *author;
    char *desc;
    char *copyright;
    char *email;
    char *url;
    int filter;    /* command-line-filter flag, not PNG row filter! */
    int pnmtype;
    int sample_depth;
    bool interlaced;
    int have_bg;
    int have_time;
    int have_text;
    jmp_buf jmpbuf;
    uch bg_red;
    uch bg_green;
    uch bg_blue;
	const png_color *pallet;
	int pallet_colour_count;
} png_mainprog_info;


class WritePng
{
public:
	WritePng();
	~WritePng();
	FILE *OpenFile(const TCHAR *filename);
	void CloseFile();
	int writepng_init(png_mainprog_info *mainprog_ptr);
	int writepng_encode_image();
	int writepng_encode_row();
	int writepng_encode_finish();
	void writepng_cleanup();

	FILE *file;
	png_mainprog_info *mainprog_ptr;
};