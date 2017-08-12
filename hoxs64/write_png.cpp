#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <zlib.h>
#include <png.h>
#include <tchar.h>
#include "write_png.h"


static void writepng_error_handler(png_structp png_ptr, png_const_charp msg);

WritePng::WritePng()
{
	file = NULL;
	mainprog_ptr = NULL;
}

WritePng::~WritePng()
{
	writepng_cleanup();
	CloseFile();
}

FILE *WritePng::OpenFile(const TCHAR *filename)
{	
	FILE *f = _tfopen(filename, TEXT("w+b"));
	if (f!=NULL)
	{
		CloseFile();
		file = f;
	}
	return f;
}

void WritePng::CloseFile()
{
	if (file != NULL)
	{
		fclose(file);
		file = NULL;
	}
}

int WritePng::writepng_init(png_mainprog_info *mainprog_ptr)
{
    png_structp  png_ptr;       /* note:  temporary variables! */
    png_infop  info_ptr;
    int color_type, interlace_type;


    /* could also replace libpng warning-handler (final NULL), but no need: */

    png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, mainprog_ptr, writepng_error_handler, NULL);
    if (!png_ptr)
        return 4;   /* out of memory */

    info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) {
        png_destroy_write_struct(&png_ptr, NULL);
        return 4;   /* out of memory */
    }


    /* setjmp() must be called in every function that calls a PNG-writing
     * libpng function, unless an alternate error handler was installed--
     * but compatible error handlers must either use longjmp() themselves
     * (as in this program) or exit immediately, so here we go: */

    if (setjmp(mainprog_ptr->jmpbuf)) {
        png_destroy_write_struct(&png_ptr, &info_ptr);
        return 2;
    }

	this->mainprog_ptr = mainprog_ptr;

    /* make sure outfile is (re)opened in BINARY mode */
    png_init_io(png_ptr, mainprog_ptr->outfile);


    /* set the compression levels--in general, always want to leave filtering
     * turned on (except for palette images) and allow all of the filters,
     * which is the default; want 32K zlib window, unless entire image buffer
     * is 16K or smaller (unknown here)--also the default; usually want max
     * compression (NOT the default); and remaining compression flags should
     * be left alone */

    png_set_compression_level(png_ptr, Z_BEST_COMPRESSION);
/*
    >> this is default for no filtering; Z_FILTERED is default otherwise:
    png_set_compression_strategy(png_ptr, Z_DEFAULT_STRATEGY);
    >> these are all defaults:
    png_set_compression_mem_level(png_ptr, 8);
    png_set_compression_window_bits(png_ptr, 15);
    png_set_compression_method(png_ptr, 8);
 */


    /* set the image parameters appropriately */

	/*
    if (mainprog_ptr->pnmtype == 5)
        color_type = PNG_COLOR_TYPE_GRAY;
    else if (mainprog_ptr->pnmtype == 6)
        color_type = PNG_COLOR_TYPE_RGB;
    else if (mainprog_ptr->pnmtype == 8)
        color_type = PNG_COLOR_TYPE_RGB_ALPHA;
    else {
        png_destroy_write_struct(&png_ptr, &info_ptr);
        return 11;
    }
	*/

	if (mainprog_ptr->pnmtype == 0)
	{
		color_type = PNG_COLOR_TYPE_PALETTE;
	}
	else
	{
        png_destroy_write_struct(&png_ptr, &info_ptr);
        return 11;
	}
	

    interlace_type = mainprog_ptr->interlaced? PNG_INTERLACE_ADAM7 :
                                               PNG_INTERLACE_NONE;

    png_set_IHDR(png_ptr, info_ptr, mainprog_ptr->width, mainprog_ptr->height,
      mainprog_ptr->sample_depth, color_type, interlace_type,
      PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

	if (mainprog_ptr->pallet != NULL && mainprog_ptr->pallet_colour_count > 0)
	{
		png_set_PLTE(png_ptr, info_ptr, mainprog_ptr->pallet, mainprog_ptr->pallet_colour_count);
	}

    if (mainprog_ptr->gamma > 0.0)
        png_set_gAMA(png_ptr, info_ptr, mainprog_ptr->gamma);

    if (mainprog_ptr->have_bg) {   /* we know it's RGBA, not gray+alpha */
        png_color_16  background;

        background.red = mainprog_ptr->bg_red;
        background.green = mainprog_ptr->bg_green;
        background.blue = mainprog_ptr->bg_blue;
        png_set_bKGD(png_ptr, info_ptr, &background);
    }

    if (mainprog_ptr->have_time) {
        png_time  modtime;

        png_convert_from_time_t(&modtime, mainprog_ptr->modtime);
        png_set_tIME(png_ptr, info_ptr, &modtime);
    }

    if (mainprog_ptr->have_text) {
        png_text  text[6];
        int  num_text = 0;

        if (mainprog_ptr->have_text & RWPNG_TEXT_TITLE) {
            text[num_text].compression = PNG_TEXT_COMPRESSION_NONE;
            text[num_text].key = "Title";
            text[num_text].text = mainprog_ptr->title;
            ++num_text;
        }
        if (mainprog_ptr->have_text & RWPNG_TEXT_AUTHOR) {
            text[num_text].compression = PNG_TEXT_COMPRESSION_NONE;
            text[num_text].key = "Author";
            text[num_text].text = mainprog_ptr->author;
            ++num_text;
        }
        if (mainprog_ptr->have_text & RWPNG_TEXT_DESC) {
            text[num_text].compression = PNG_TEXT_COMPRESSION_NONE;
            text[num_text].key = "Description";
            text[num_text].text = mainprog_ptr->desc;
            ++num_text;
        }
        if (mainprog_ptr->have_text & RWPNG_TEXT_COPY) {
            text[num_text].compression = PNG_TEXT_COMPRESSION_NONE;
            text[num_text].key = "Copyright";
            text[num_text].text = mainprog_ptr->copyright;
            ++num_text;
        }
        if (mainprog_ptr->have_text & RWPNG_TEXT_EMAIL) {
            text[num_text].compression = PNG_TEXT_COMPRESSION_NONE;
            text[num_text].key = "E-mail";
            text[num_text].text = mainprog_ptr->email;
            ++num_text;
        }
        if (mainprog_ptr->have_text & RWPNG_TEXT_URL) {
            text[num_text].compression = PNG_TEXT_COMPRESSION_NONE;
            text[num_text].key = "URL";
            text[num_text].text = mainprog_ptr->url;
            ++num_text;
        }
        png_set_text(png_ptr, info_ptr, text, num_text);
    }


    /* write all chunks up to (but not including) first IDAT */

    png_write_info(png_ptr, info_ptr);


    /* if we wanted to write any more text info *after* the image data, we
     * would set up text struct(s) here and call png_set_text() again, with
     * just the new data; png_set_tIME() could also go here, but it would
     * have no effect since we already called it above (only one tIME chunk
     * allowed) */


    /* set up the transformations:  for now, just pack low-bit-depth pixels
     * into bytes (one, two or four pixels per byte) */

    png_set_packing(png_ptr);
/*  png_set_shift(png_ptr, &sig_bit);  to scale low-bit-depth values */


    /* make sure we save our pointers for use in writepng_encode_image() */

    mainprog_ptr->png_ptr = png_ptr;
    mainprog_ptr->info_ptr = info_ptr;	

    /* OK, that's all we need to do for now; return happy */

    return 0;
}

int WritePng::writepng_encode_image()
{
    png_structp png_ptr = (png_structp)mainprog_ptr->png_ptr;
    png_infop info_ptr = (png_infop)mainprog_ptr->info_ptr;


    /* as always, setjmp() must be called in every function that calls a
     * PNG-writing libpng function */

    if (setjmp(mainprog_ptr->jmpbuf)) {
        png_destroy_write_struct(&png_ptr, &info_ptr);
        mainprog_ptr->png_ptr = NULL;
        mainprog_ptr->info_ptr = NULL;
        return 2;
    }


    /* and now we just write the whole image; libpng takes care of interlacing
     * for us */

    png_write_image(png_ptr, mainprog_ptr->row_pointers);


    /* since that's it, we also close out the end of the PNG file now--if we
     * had any text or time info to write after the IDATs, second argument
     * would be info_ptr, but we optimize slightly by sending NULL pointer: */

    png_write_end(png_ptr, NULL);

    return 0;
}

int WritePng::writepng_encode_row()  /* NON-interlaced only! */
{
    png_structp png_ptr = (png_structp)mainprog_ptr->png_ptr;
    png_infop info_ptr = (png_infop)mainprog_ptr->info_ptr;


    /* as always, setjmp() must be called in every function that calls a
     * PNG-writing libpng function */

    if (setjmp(mainprog_ptr->jmpbuf)) {
        png_destroy_write_struct(&png_ptr, &info_ptr);
        mainprog_ptr->png_ptr = NULL;
        mainprog_ptr->info_ptr = NULL;
        return 2;
    }


    /* image_data points at our one row of image data */

    png_write_row(png_ptr, mainprog_ptr->image_data);

    return 0;
}

int WritePng::writepng_encode_finish()
{
    png_structp png_ptr = (png_structp)mainprog_ptr->png_ptr;
    png_infop info_ptr = (png_infop)mainprog_ptr->info_ptr;


    /* as always, setjmp() must be called in every function that calls a
     * PNG-writing libpng function */

    if (setjmp(mainprog_ptr->jmpbuf)) {
        png_destroy_write_struct(&png_ptr, &info_ptr);
        mainprog_ptr->png_ptr = NULL;
        mainprog_ptr->info_ptr = NULL;
        return 2;
    }


    /* close out PNG file; if we had any text or time info to write after
     * the IDATs, second argument would be info_ptr: */

    png_write_end(png_ptr, NULL);

    return 0;
}

void WritePng::writepng_cleanup()
{
	if (mainprog_ptr != NULL)
	{
		png_structp png_ptr = (png_structp)mainprog_ptr->png_ptr;
		png_infop info_ptr = (png_infop)mainprog_ptr->info_ptr;

		if (png_ptr && info_ptr)
		{
			png_destroy_write_struct(&png_ptr, &info_ptr);
		}
		mainprog_ptr = NULL;
	}
}

static void writepng_error_handler(png_structp png_ptr, png_const_charp msg)
{
    png_mainprog_info  *mainprog_ptr;

    /* This function, aside from the extra step of retrieving the "error
     * pointer" (below) and the fact that it exists within the application
     * rather than within libpng, is essentially identical to libpng's
     * default error handler.  The second point is critical:  since both
     * setjmp() and longjmp() are called from the same code, they are
     * guaranteed to have compatible notions of how big a jmp_buf is,
     * regardless of whether _BSD_SOURCE or anything else has (or has not)
     * been defined. */

    fprintf(stderr, "writepng libpng error: %s\n", msg);
    fflush(stderr);

    mainprog_ptr = (png_mainprog_info*)png_get_error_ptr(png_ptr);
    if (mainprog_ptr == NULL) {         /* we are completely hosed now */
        fprintf(stderr,
          "writepng severe error:  jmpbuf not recoverable; terminating.\n");
        fflush(stderr);
        exit(99);
    }

    longjmp(mainprog_ptr->jmpbuf, 1);
}
