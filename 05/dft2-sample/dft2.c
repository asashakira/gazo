/****************************************************************
 *
 *   2-D Discreate Fourie Translate program.
 *
 *   writer : M.Mukunoki
 *   date   : 1991/06/11
 *	     2016/12/16  rewrite for KUMI3
 *            2017/05/17
 *
 *   compile : gcc -O3 -funroll-loops -I. dft2.c kumi3.c -o dft2 -lm
 *   execute : ./dft2 sample01.pgm dft2_01.kumi
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

#define	DEF_SCALE	2.0

/**** Macros ****/

/**** Typedefs ****/
typedef unsigned char uchar;

/**** External Variables ****/

/**** Prototype Declaration ****/
int main(int argc, char *argv[]);
int usage(char *command);
int dft2(K_IMAGE *inp_img, K_IMAGE *out_img);

/****************************************************************/
int main(int argc, char *argv[]) {
  K_IMAGE	*inp_img, *out_img;
  K_HEAD	header;
  char	*inp_fname	= K_STDFILE;
  char	*out_fname	= K_STDFILE;
  int		inv = -1;

  int		c;
  int		errflg = 0;

  extern int	optind;
  extern char	*optarg;

  while((c = getopt(argc, argv, "i:o:rh")) != EOF) {
    switch(c) {
      case 'i':
        inp_fname = optarg;
        break;
      case 'o':
        out_fname = optarg;
        break;
      case 'r':
        inv = 1;
        break;
      case 'h':
        errflg++;
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
    inp_fname = argv[optind++];
  }
  if (optind < argc) {
    out_fname = argv[optind++];
  }
  if (optind != argc) {
    usage(argv[0]);
    exit(1);
  }

  if ((inp_img = k_open(inp_fname)) == NULL) return(-1);

  if (k_pixeltype(inp_img) != K_UCHAR) {
    fprintf(stderr, "Pixeltype of input image must be K_UCHAR!!\n");
    exit(1);
  }

  header = *k_header(inp_img);
  header.multi = 2;
  header.pixeltype = K_FLOAT;
  if ((out_img = k_create(&header)) == NULL) return(-1);

  dft2(inp_img, out_img);

  if (k_write(out_img, out_fname)) return(-1);
  k_close(out_img);
  k_close(inp_img);

  return 0;
}


int usage(char	*command) {
  fprintf(stderr, "2D Discrete Fourie Transform.\n");
  fprintf(stderr, "Usage: %s [-h] ", command);
  fprintf(stderr, "[[-i] input_file [[-o] output_file]]\n");
  fprintf(stderr, "\t -h : help\n");
  fprintf(stderr, "\t -i : input_file (you can omit '-i')\n");
  fprintf(stderr, "\t -o : output_file (you can omit '-o')\n");

  return 0;
}

int dft2(K_IMAGE *inp_img, K_IMAGE *out_img) {
  int	xsize = k_xsize(inp_img);
  int	ysize = k_ysize(inp_img);

  float	***optr = (float ***)k_data(out_img);
  uchar	**iptr  = (uchar **)k_data(inp_img)[0];

  for(int l = 0; l < ysize; l++) {
    for(int k = 0; k < xsize; k++) {
      optr[0][l][k] = optr[1][l][k] = 0.0;
      for(int n = 0; n < ysize; n++) {
        for(int m = 0; m < xsize; m++) {
          double mk = -2.0 * M_PI * m * k / xsize; 
          double nl = -2.0 * M_PI * n * l / ysize;

          optr[0][l][k] += iptr[n][m] * (cos(mk)*cos(nl) - sin(mk)*sin(nl));
          optr[1][l][k] += iptr[n][m] * (cos(mk)*sin(nl) + sin(mk)*cos(nl));
        }
      }
    }
  }

  return 0;
}
