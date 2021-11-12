/****************************************************************
*
*   Copy Image
*
*   writer  :	M.Mukunoki
*   date    :	2016/12/08
*   compile :
*    gcc -I. copy_img.c kumi3.c -o copy_img
*   execute :
*    ./copy_img sample01.pgm out01.pgm
*
****************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <kumi3.h>

/**** Defines ****/
#define	DEF_THRES	128

/**** Macros ****/

/**** Typedefs ****/
typedef unsigned char uchar;

/**** External Variables ****/

/**** Prototype Declaration ****/
int main(int argc, char *argv[]);
int usage(char *command);
int copy_img(K_IMAGE *inp_img, K_IMAGE *out_img);

/**** Routines ****/
int main(int argc, char *argv[])
{
    K_IMAGE	*inp_img, *out_img;
    char	*inp_fname	= K_STDFILE;
    char	*out_fname	= K_STDFILE;

    int		c;
    int		errflg = 0;
    int		thres = DEF_THRES;

    extern int	optind;
    extern char	*optarg;
	
    while((c = getopt(argc, argv, "i:o:Pt:h")) != EOF)
    {
	switch(c)
	{
	case 'i':
	    inp_fname = optarg;
	    break;
	case 'o':
	    out_fname = optarg;
	    break;
	case 't':
	    thres = atoi(optarg);
	    break;
	case 'P':
	    out_fname = K_STDPNM;
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

    if ((inp_img = k_open(inp_fname)) == NULL)
    {
	return(-1);
    }
    if (k_pixeltype(inp_img) != K_UCHAR)
    {
	fprintf(stderr, "Pixeltype of input image must be K_UCHAR!!\n");
	exit(1);
    }

    if ((out_img = k_create(k_header(inp_img))) == NULL)
    {
	return(-1);
    }

    copy_img(inp_img, out_img);

    k_write(out_img, out_fname);

    k_close(inp_img);
    k_close(out_img);

    return 0;
}

int usage(char	*command)
{
    fprintf(stderr, "Copy Image.\n");
    fprintf(stderr, "Usage: %s [-hP] [-t thres] ", command);
    fprintf(stderr, "[[-i] input_file [[-o] output_file]]\n");
    fprintf(stderr, "\t -h : help\n");
    fprintf(stderr, "\t -t : threshold value\n");
    fprintf(stderr, "\t -P : output with NETPBM format\n");
    fprintf(stderr, "\t -i : input_file (you can omit '-i')\n");
    fprintf(stderr, "\t -o : output_file (you can omit '-o')\n");

    return(0);
}

int copy_img(K_IMAGE *inp_img, K_IMAGE *out_img)
{
    uchar	***iptr = (uchar ***)k_data(inp_img);
    uchar	***optr = (uchar ***)k_data(out_img);

    for(int m = 0; m < k_multi(inp_img); m++)
    {
	for(int y = 0; y < k_ysize(inp_img); y++)
	{
	    for(int x = 0; x < k_xsize(inp_img); x++)
	    {
		optr[m][y][x] = iptr[m][y][x];
	    }
	}
    }

    return 0;
}
