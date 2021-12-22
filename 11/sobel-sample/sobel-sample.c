/****************************************************************
 *
 *   Sobel Filter
 *
 *   filename	:	sobel.c
 *   writer	:	M.MUKUNOKI
 *   date	:       2017/06/21
 *
 ****************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <math.h>

#include <kumi3.h>

/**** Defines ****/

/**** Macros ****/

/**** Typedefs ****/
typedef unsigned char uchar;

/**** External Variables ****/

/**** Prototype Declaration ****/
int main(int argc, char *argv[]);
int usage(char *command);
void set_img(K_IMAGE *img, int val);
void sobel(K_IMAGE *inp_img, K_IMAGE *out_img);

/****************************************************************/
int main(int argc, char *argv[]) {
  K_IMAGE	*inp_img, *out_img;
  char	*inp_fname	= K_STDFILE;
  char	*out_fname	= K_STDFILE;

  int 	c;
  int 	errflg = 0;

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
    fprintf(stderr, "Can't open image for read %s\n", inp_fname);
    exit(1);
  }
  if (k_pixeltype(inp_img) != K_UCHAR) {
    fprintf(stderr, "Pixeltype of input image must be K_BIT!!\n");
    exit(1);
  }

  if ((out_img = k_create(k_header(inp_img))) == NULL) {
    fprintf(stderr, "Can't create image for write\n");
    exit(1);
  }

  set_img(out_img, 0);
  sobel( inp_img, out_img);

  if (k_write(out_img, out_fname)) {
    fprintf(stderr, "Can't write image %s\n", out_fname);
    exit(1);
  }
  k_close(inp_img);
  k_close(out_img);

  exit(0);
}


int usage(char	*command) {
  fprintf(stderr, "Laplacian Filter\n");
  fprintf(stderr, "Usage: %s [-hP] ", command);
  fprintf(stderr, "[[-i] input_file [[-o] output_file]]\n");
  fprintf(stderr, "\t -h : help\n");
  fprintf(stderr, "\t -P : output with PBMPLUS mode\n");
  fprintf(stderr, "\t -i : input_file (you can omit '-i')\n");
  fprintf(stderr, "\t -o : output_file (you can omit '-o')\n");

  return 0;
}

void set_img(K_IMAGE *img, int val) {
  int		x, y, m;
  uchar	***optr = (uchar ***)k_data(img);
  for(m = 0; m < k_multi(img); m++) {
    for(y = 0; y < k_ysize(img); y++) {
      for(x = 0; x < k_xsize(img); x++) {
        optr[m][y][x] = val;
      }
    }
  }
  return;
}

void sobel(K_IMAGE *inp_img, K_IMAGE *out_img) {
  uchar **iptr = (uchar **)k_data(inp_img)[0];
  uchar **optr = (uchar **)k_data(out_img)[0];

  for (int y = 1; y < k_ysize(inp_img)-1; y++) {
    for (int x = 1; x < k_xsize(inp_img)-1; x++) {
      int fx = -iptr[y-1][x-1] - 2*iptr[y][x-1] - iptr[y+1][x-1]
             +  iptr[y-1][x+1] + 2*iptr[y][x+1] + iptr[y+1][x+1];
      int fy = -iptr[y-1][x-1] - 2*iptr[y-1][x] - iptr[y-1][x+1]
             +  iptr[y+1][x-1] + 2*iptr[y+1][x] + iptr[y+1][x+1];
      int val = sqrt(fx*fx + fy*fy);
      optr[y][x] = val < 0 ? 0 : val > 255 ? 255 : val;
    }
  }
  return;
}
