/****************************************************************
 *
 *   Smoothing filter
 *
 *   writer  :	Akira Itai
 *   date    :	2021/10/27
 *   compile :
 gcc -I. smooth3.c kumi3.c -o smooth3 -lm
 *   execute :
 ./smooth3 sample01.pgm out01.pgm
 *
 ****************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <getopt.h>

#include <kumi3.h>

/**** Defines ****/
#define	B 5

/**** Macros ****/

/**** Typedefs ****/
typedef unsigned char uchar;

/**** External Variables ****/

/**** Prototype Declaration ****/
int main(int argc, char *argv[]);
int usage(char *command);
int smooth3(K_IMAGE *inp_img, K_IMAGE *out_img, int mask[2 *B +1 ][2 *B +1 ]);

/**** Routines ****/
int main(int argc, char *argv[]) {
  K_IMAGE	*inp_img, *out_img;
  char *inp_fname	= K_STDFILE;
  char *out_fname	= K_STDFILE;

  int		mask[2*B+1][2*B+1] = {
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1},
    {1, 2, 3, 3, 3, 3, 3, 3, 3, 2, 1},
    {1, 2, 3, 4, 4, 4, 4, 4, 3, 2, 1},
    {1, 2, 3, 4, 5, 5, 5, 4, 3, 2, 1},
    {1, 2, 3, 4, 5, 6, 5, 4, 3, 2, 1},
    {1, 2, 3, 4, 5, 5, 5, 4, 3, 2, 1},
    {1, 2, 3, 4, 4, 4, 4, 4, 3, 2, 1},
    {1, 2, 3, 3, 3, 3, 3, 3, 3, 2, 1},
    {1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1},
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}
  };
  int c;
  int errflg = 0;

  extern int	optind;
  extern char	*optarg;

  while((c = getopt(argc, argv, "i:o:Ph")) != EOF) {
    switch(c) {
      case 'i':
        inp_fname = optarg;
        break;
      case 'o':
        out_fname = optarg;
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

  if ((inp_img = k_open(inp_fname)) == NULL) {
    return(-1);
  }
  if (k_pixeltype(inp_img) != K_UCHAR) {
    fprintf(stderr, "Pixeltype of input image must be K_UCHAR!!\n");
    exit(1);
  }

  if ((out_img = k_create(k_header(inp_img))) == NULL) {
    return(-1);
  }

  smooth3(inp_img, out_img, mask);

  k_write(out_img, out_fname);

  k_close(inp_img);
  k_close(out_img);

  return 0;
}

int usage(char	*command) {
  fprintf(stderr, "smoothing filter.\n");
  fprintf(stderr, "Usage: %s [-hP] [-a <6 affine parameters>] ", command);
  fprintf(stderr, "[-s out_xsize out_ysize] ");
  fprintf(stderr, "[[-i] input_file [[-o] output_file]]\n");
  fprintf(stderr, "\t -h : help\n");
  fprintf(stderr, "\t -P : output with NETPBM format\n");
  fprintf(stderr, "\t -i : input_file (you can omit '-i')\n");
  fprintf(stderr, "\t -o : output_file (you can omit '-o')\n");

  return 0;
}

int smooth3(K_IMAGE *inp_img, K_IMAGE *out_img, int mask[2*B+1][2*B+1]) {
  for(int m = 0; m < k_multi(inp_img); m++) {
    uchar	**iptr = (uchar **)k_data(inp_img)[m];
    uchar	**optr = (uchar **)k_data(out_img)[m];

    for(int y = 0; y < k_ysize(inp_img); y++) {
      for(int x = 0; x < k_xsize(inp_img); x++) {
        int	sum = 0, c = 0;
        for (int i = -B; i <= B; i++) {
          for (int j = -B; j <= B; j++) {
            if (x+j < 0 || y+i < 0 || x+j >= k_xsize(inp_img) || y+i >= k_ysize(inp_img)) continue;
            sum += iptr[y+i][x+j] * mask[i+B][j+B];
            c += mask[i+B][j+B];
          }
        }
        optr[y][x] = sum / c;
      }
    }
  }
  return 0;
}
