/****************************************************************
*
*   Calculate Average Pixel Value for RGB Image
*
*   filename	:	average_rgb.c
*   writer	:	M.Mukunoki
*   date	:	2016/08/25
*
*   compile     :
*    gcc -O3 -funroll-loops average_rgb.c kumi3.c -o average_rgb
*
****************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <kumi3.h>

/**** Defines ****/

/**** Macros ****/

/**** Typedefs ****/
typedef unsigned char uchar;

/**** External Variables ****/

/**** Prototype Declaration ****/
int main(int argc, char *argv[]);
int usage(char *command);
int average_rgb(K_IMAGE *img, float *ave);

/**** Routines ****/
int main(int argc, char *argv[])
{
    K_IMAGE	*inp_img;
    char	*inp_fname	= K_STDFILE;

    int		c;
    int		errflg = 0;

    extern int	optind;
    extern char	*optarg;
	
    while((c = getopt(argc, argv, "i:h")) != EOF)
    {
	switch(c)
	{
	case 'i':
	    inp_fname = optarg;
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
    if (optind != argc)
    {
	usage(argv[0]);
	exit(1);
    }

    if ((inp_img = k_open(inp_fname)) == NULL)
    {
	return(-1);
    }

    if (k_pixeltype(inp_img) != K_UCHAR)
    {
	fprintf(stderr, "Pixeltype of input image must be K_UCHAR!!\n");
	exit(1);
    }

    float	ave[k_multi(inp_img)];

    average_rgb(inp_img, ave);
    for(int m = 0; m < k_multi(inp_img); m++)
    {
	printf("%d %f\n", m, ave[m]);
    }

    k_close(inp_img);

    exit(0);
}

int usage(char	*command)
{
    fprintf(stderr, "Calculate Average Pixel Value for RGB Image.\n");
    fprintf(stderr, "Usage: %s [-h] ", command);
    fprintf(stderr, "[[-i] input_file\n");
    fprintf(stderr, "\t -h : help\n");
    fprintf(stderr, "\t -i : input_file (you can omit '-i')\n");

    return(0);
}

int average_rgb(K_IMAGE *img, float *ave)
{
    uchar	***iptr = (uchar ***)k_data(img);

    for(int m = 0; m < k_multi(img); m++)
    {
	int		sum = 0;
	for(int y = 0; y < k_ysize(img); y++)
	{
	    for(int x = 0; x < k_xsize(img); x++)
	    {
		sum += iptr[m][y][x];
	    }
	}
	ave[m] = (float)sum/(k_xsize(img) * k_ysize(img));
    }

    return 0;
}
