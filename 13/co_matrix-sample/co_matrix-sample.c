/****************************************************************
 *
 *   Calculate Co-occurrence Matrix
 *
 *   writer	:   M. Mukunoki
 *   date	:   2020/12/22
 *
 ****************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

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
int calc_co_matrix(K_IMAGE *inp_img, int **co_matrix, int level, int dx, int dy);

/**** Routines ****/
int main(int argc, char *argv[])
{
  K_IMAGE	*inp_img;
  char	*inp_fname	= K_STDFILE;

  int		c;
  int		errflg = 0;

  extern int	optind;
  extern char	*optarg;

  while((c = getopt(argc, argv, "i:hv")) != EOF)
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
  if (k_multi(inp_img) != 1)
  {
    fprintf(stderr, "Input image must be a GRAY image!!\n");
    exit(1);
  }

  ////////////////////////////////////////////////////////////////
  int		level = 4;
  int		**co_matrix;
  int		dx = 1;
  int		dy = 0;

  co_matrix = (int **)malloc(level * sizeof(int *));
  for(int i = 0; i < level; i++)
  {
    co_matrix[i] = (int *)calloc(level, sizeof(int));
  }

  calc_co_matrix(inp_img, co_matrix, level, dx, dy);

  for(int i = 0; i < level; i++)
  {
    for(int j = 0; j < level; j++)
    {
      printf("%6d ", co_matrix[i][j]);
    }
    printf("\n");
  }

  k_close(inp_img);

  exit(0);
}


int usage(char *command)
{
  fprintf(stderr, "Histgram equalizer\n");
  fprintf(stderr, "Usage: %s [-hP] [-1|-8]", command);
  fprintf(stderr, " [[-i] input_file [[-o] output_file]]\n");
  fprintf(stderr, "\t -h : help\n");
  fprintf(stderr, "\t -t : threshold value\n");
  fprintf(stderr, "\t -P : output with NETPBM format\n");
  fprintf(stderr, "\t -i : input_file (you can omit '-i')\n");
  fprintf(stderr, "\t -o : output_file (you can omit '-o')\n");

  return(0);
}

int calc_co_matrix(K_IMAGE *inp_img, int **co_matrix, int level, int dx, int dy)
{
  uchar	**iptr = (uchar **)k_data(inp_img)[0];
  int		xsize = k_xsize(inp_img);
  int		ysize = k_ysize(inp_img);

  for(int y1 = 0; y1 < ysize; y1++)
  {
    for(int x1 = 0; x1 < xsize; x1++)
    {
      int	x2 = x1 + dx;
      int	y2 = y1 + dy;

      if (x2 < 0 || x2 >= xsize || y2 < 0 || y2 >= ysize) continue;

      int now = iptr[y1][x1] * level / 256;
      int next = iptr[y2][x2] * level / 256;
      co_matrix[now][next]++;
      co_matrix[next][now]++;
    }
  }
  return 0;
}
