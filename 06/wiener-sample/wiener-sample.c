/****************************************************************
 *
 *   2D Wiener Filter
 *
 *   date   : 1991/06/11
 *	     2016/12/19  rewrite for KUMI3
 *            2017/05/23
 *
 *   compile : gcc -O3 -funroll-loops -I. -std=gnu99 wiener-sample.c kumi3.c -o wiener -lm
 *   execute : ./wiener -g 0.001 sample02_dft.kumi psf_dft.kumi out.kumi
 *
 ****************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <getopt.h>

#include <kumi3.h>

/**** Defines ****/
#define	DEF_GAMMA	0.001

/**** Macros ****/
#define sqr(x)	((x)*(x))

/**** Typedefs ****/
typedef unsigned char uchar;

/**** External Variables ****/

/**** Prototype Declaration ****/
int main(int argc, char *argv[]);
int usage(char *command);
void wiener_filter(K_IMAGE *fimg, K_IMAGE *gimg, K_IMAGE *oimg, float gamma);

/****************************************************************/
int main(int argc, char *argv[]) {
  K_IMAGE	*fimg, *gimg, *oimg;
  char	*fimg_fname	= K_STDFILE;
  char	*gimg_fname	= K_STDFILE;
  char	*oimg_fname	= K_STDFILE;
  float	gamma = DEF_GAMMA;

  int		c;
  int		errflg = 0;

  extern int	optind;
  extern char	*optarg;

  while((c = getopt(argc, argv, "rg:h")) != EOF) {
    switch(c) {
      case 'h':
        errflg++;
        break;
      case 'g':
        gamma = atof(optarg);
        break;
      default:
        errflg++;
        break;
    }
    if (errflg) {
      usage(argv[0]);
      exit(1);
    }
  }/* end of while */
  if (optind < argc) {
    fimg_fname = argv[optind++];
  }
  if (optind < argc) {
    gimg_fname = argv[optind++];
  }
  if (optind < argc) {
    oimg_fname = argv[optind++];
  }
  if (optind != argc) {
    usage(argv[0]);
    exit(1);
  }

  if ((fimg = k_open(fimg_fname)) == NULL) return(-1);
  if (k_pixeltype(fimg) != K_FLOAT || k_multi(fimg) != 2) {
    fprintf(stderr, "Input image must be COMPLEX image!!\n");
    exit(1);
  }

  if ((gimg = k_open(gimg_fname)) == NULL) return(-1);
  if (k_pixeltype(gimg) != K_FLOAT || k_multi(gimg) != 2) {
    fprintf(stderr, "Input image must be COMPLEX image!!\n");
    exit(1);
  }

  if ((oimg = k_create(k_header(fimg))) == NULL) return(-1);

  wiener_filter(fimg, gimg, oimg, gamma);

  if (k_write(oimg, oimg_fname)) return(-1);
  k_close(oimg);
  k_close(gimg);
  k_close(fimg);

  return 0;
}


int usage(char	*command) {
  fprintf(stderr, "2D Winner Filter.\n");
  fprintf(stderr, "Usage: %s [-h] ", command);
  fprintf(stderr, "<input_file> <filter_file> <output_file>\n");
  fprintf(stderr, "\t -h : help\n");

  return(0);
}

void wiener_filter(K_IMAGE *fimg, K_IMAGE *gimg, K_IMAGE *oimg, float gamma) {
  int	xsize = k_xsize(fimg);
  int	ysize = k_ysize(fimg);

  float ***fptr = (float ***)k_data(fimg);
  float ***gptr = (float ***)k_data(gimg);
  float ***optr = (float ***)k_data(oimg);

  int	x, y;
  for(y = 0; y < ysize; y++) {
    for(x = 0; x < xsize; x++) {
      float a = gptr[0][y][x];
      float b = gptr[1][y][x];
      float c = fptr[0][y][x];
      float d = fptr[1][y][x];

      optr[0][y][x] = (a*c + b*d) / (a*a + b*b + gamma);
      optr[1][y][x] = (a*d - b*c) / (a*a + b*b + gamma);
    }
  }
}
