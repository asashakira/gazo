/****************************************************************
 *
 *   filename	:	flatten.c
 *
 *   compile     :
gcc -I. flatten.c kumi3.c -o flatten
 *
 ****************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <kumi3.h>

/**** Defines ****/
#define	GRAY_LEVELS	256

/**** Macros ****/

/**** Typedefs ****/
typedef unsigned char uchar;

/**** External Variables ****/

/**** Prototype Declaration ****/
int main(int argc, char *argv[]);
int usage(char *command);
void calc_hist(K_IMAGE *img, int hist[GRAY_LEVELS]);

/**** Routines ****/
int main(int argc, char *argv[])
{
  K_IMAGE	*inp_img;
  char	*inp_fname	= K_STDFILE;
  char	*out_fname	= K_STDFILE;
  int		hist[GRAY_LEVELS];

  int		c;
  int		errflg = 0;

  extern int	optind;
  extern char	*optarg;

  while((c = getopt(argc, argv, "i:o:t:h")) != EOF) {
    switch(c) {
      case 'i':
        inp_fname = optarg;
        break;
      case 'o':
        out_fname = optarg;
        break;
      case 't':
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

  calc_hist(inp_img, hist);
  flatten(inp_img, hist);

  k_write(out_img, out_fname);

  k_close(inp_img);
  k_close(out_img);

  exit(0);
}

int usage(char	*command) {
  fprintf(stderr, "Calculate Average Pixel Value for RGB Image.\n");
  fprintf(stderr, "Usage: %s [-h] ", command);
  fprintf(stderr, "[[-i] input_file\n");
  fprintf(stderr, "\t -h : help\n");
  fprintf(stderr, "\t -i : input_file (you can omit '-i')\n");

  return(0);
}

void calc_hist(K_IMAGE *img, int hist[GRAY_LEVELS]) {
  uchar	**iptr = (uchar **)k_data(img)[0];

  for(int t = 0; t < GRAY_LEVELS; t++) {
    hist[t] = 0;
  }

  for(int y = 0; y < k_ysize(img); y++) {
    for(int x = 0; x < k_xsize(img); x++) {
      hist[iptr[y][x]]++;
    }
  }
  return;
}

void flatten(K_IMAGE *inp_img, K_IMAGE *out_img, int hist[GRAY_LEVELS]) {
  uchar	**iptr = (uchar **)k_data(inp_img)[0];
  uchar	**optr = (uchar **)k_data(out_img)[0];
  for (int y = 0; y < k_ysize(inp_img); y++) {
    for (int x = 0; x < k_xsize(inp_img); x++) {
    }
  }
}

