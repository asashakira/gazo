/**********************************************************
*                                                         *
*      Kyoto University Multi Images Handling System      *
*                                                         *
*      Heading for Kumi Image Handling system             *
*                                                         *
*       Written by Michihiko Minoh, Fri., Aug.25th, 1989  *
*                                                         *
*	Copy right by Kyoto University, 1989              *
*                                                         *
***********************************************************/
#ifndef _KUMI
#define _KUMI

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

/* basic constant */
#define K_STDFILE  "/-"
#define K_STDPNM   "/-.pnm"

#define K_MAGIC	      "100K"    // Magic Number

/* Code for the pixel type of the header */
#define K_UCHAR  0
#define K_USHORT 1
#define K_INT    2
#define K_FLOAT  3
#define K_DOUBLE 4
#define K_BIT    5

/* Code for the store type of the header */
#define K_FILTER	(0x00000002)

/**** Data Structure for KUMI header file ****/
#define	K_HEADER_ITEMS	8
#define	K_INT_SIZE	4
typedef struct
{
    int	stype;			/* store type */
    int	xsize;			/* horizontal size of the image */
    int	ysize;			/* vertical size of the image */
    int	pixeltype;		/* pixel type */
    int	ebit;			/* effective bit */
    int	imgtype;		/* image type (dummy) */
    int	multi;			/* multiply of frames */
    int	order;			/* number of frames */
} K_HEAD;

/**** Data Structure for image data ****/
typedef struct
{
    K_HEAD *header;		/* header information */
    int	   pixelbyte;		/* number of bytes for a pixel */
    void   ***data;		/* pointer to the buffer for images */
} K_IMAGE;

/* external variables */
extern int _k_debug;	/* debug mode */

/* Functions */
K_IMAGE *k_open(char *filename);
K_IMAGE *k_create(K_HEAD *header);
int k_write(K_IMAGE *image, char *filename);
int k_close(K_IMAGE *image);
int k_getpix(K_IMAGE *image, int x, int y, int m, void *buf);
int k_putpix(K_IMAGE *image, int x, int y, int m, void *buf);
void k_perror(const char *format, ...);
	
K_IMAGE *k_open_frame(char *fnhead, int num);
int k_write_frame(K_IMAGE *image, char *fnhead, int num);

/* utility routines */
int ku_crop_uchar(int val);
int ku_get_ebit(int maxval);
int ku_get_maxval(int ebit);

/* Macros */
//#define	k_stype(image) 	    	((image)->header->stype)
#define k_xsize(image)		((image)->header->xsize)
#define k_ysize(image)		((image)->header->ysize)
#define k_ebit(image)		((image)->header->ebit)
#define k_pixeltype(image)	((image)->header->pixeltype)
#define	k_multi(image)		((image)->header->multi)
#define k_header(image)		((image)->header)
#define k_data(image)		((image)->data)

#define k_pixelbyte(image)	((image)->pixelbyte)

/****************************************************************/
#ifdef __cplusplus
}
#endif

#endif
