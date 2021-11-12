/****************************************************************
*
*   2-D Discreate Inverse Fourie Translate program.
*
*   writer : M.Mukunoki
*   date   : 1991/06/11
*	     2016/12/16  rewrite for KUMI3
*            2017/05/17
*
*   compile : gcc -O3 -funroll-loops -I. dft2r.c kumi3.c -o dft2r -lm
*   execute : ./dft2r dft2_01.kumi out01.pgm
*
****************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <getopt.h>

#include <kumi3.h>

/**** Defines ****/
#ifndef M_PI
#define	M_PI		3.1415927
#endif

/**** Macros ****/

/**** Typedefs ****/
typedef unsigned char uchar;

/**** External Variables ****/

/**** Prototype Declaration ****/
int main(int argc, char *argv[]);
int usage(char *command);
int dft2r(K_IMAGE *inp_img, K_IMAGE *out_img);

/****************************************************************/
int main(int argc, char *argv[])
{
    K_IMAGE	*inp_img, *out_img;
    K_HEAD	header;
    char	*inp_fname	= K_STDFILE;
    char	*out_fname	= K_STDFILE;

    int		c;
    int		errflg = 0;

    extern int	optind;
    extern char	*optarg;
	
    while((c = getopt(argc, argv, "i:o:h")) != EOF)
    {
	switch(c)
	{
	case 'i':
	    inp_fname = optarg;
	    break;
	case 'o':
	    out_fname = optarg;
	    break;
	case 'h':
	    errflg++;
	    break;
	default:
	    errflg++;
	    break;
	}
	if (errflg)
	{
	    usage(argv[0]);
	    exit(1);
	}
    }/* end of while */
    if (optind < argc)
    {
	inp_fname = argv[optind++];
    }
    if (optind < argc)
    {
	out_fname = argv[optind++];
    }
    if (optind != argc)
    {
	usage(argv[0]);
	exit(1);
    }

    if ((inp_img = k_open(inp_fname)) == NULL) return(-1);

    if (k_pixeltype(inp_img) != K_FLOAT)
    {
	fprintf(stderr, "Pixeltype of input image must be K_FLOAT!!\n");
	exit(1);
    }

    if (k_multi(inp_img) != 2)
    {
	fprintf(stderr, "Multi of input image must be 2!!\n");
	exit(1);
    }

    header = *k_header(inp_img);
    header.multi = 1;
    header.pixeltype = K_UCHAR;
    if ((out_img = k_create(&header)) == NULL) return(-1);
	
    dft2r(inp_img, out_img);
	
    if (k_write(out_img, out_fname)) return(-1);
    k_close(out_img);
    k_close(inp_img);

    return 0;
}


int usage(char	*command)
{
    fprintf(stderr, "2D Discrete Inverse Fourie Transform.\n");
    fprintf(stderr, "Usage: %s [-h] ", command);
    fprintf(stderr, "[[-i] input_file [[-o] output_file]]\n");
    fprintf(stderr, "\t -h : help\n");
    fprintf(stderr, "\t -i : input_file (you can omit '-i')\n");
    fprintf(stderr, "\t -o : output_file (you can omit '-o')\n");

    return(0);
}

int dft2r(K_IMAGE *inp_img, K_IMAGE *out_img)
{
    int	xsize = k_xsize(inp_img);
    int	ysize = k_ysize(inp_img);

    float	***iptr = (float ***)k_data(inp_img);
    uchar	**optr  = (uchar **)k_data(out_img)[0];
	
    for(int l = 0; l < ysize; l++)
    {
	fprintf(stderr, "%d / %d\n", l, ysize);
	for(int k = 0; k < xsize; k++)
	{
	    double	val = 0.0;
	    for(int n = 0; n < ysize; n++)
	    {
		for(int m = 0; m < xsize; m++)
		{
		    double mk = 2.0 * M_PI * m * k / xsize; 
		    double nl = 2.0 * M_PI * n * l / ysize;

		    val += ; // この行に式を追加
			
		}
	    }
	    val /= xsize * ysize;
	    if ( val<0.0   ) val=0.0;
	    if ( val>255.0 ) val=255.0;
	    optr[l][k] = (int)val;
	}
    }
    
    return(0);
}
