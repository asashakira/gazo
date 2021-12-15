/**********************************************************
*							  *
*      Kyoto University Multi Images Handling System	  *
*							  *
*      file I/O routines				  *
*							  *
*   Written by Izuru Takaya, Thurs, June 29th, 1989	  *
*   Modified by Michihiko Minoh, Fri, Sept. 1st, 1989     *
*   Modified by Masayuki Mukunoki, Thu, Jun. 21, 1990	  *
*   Modified by Masayuki Mukunoki, 2007/08/16        	  *
*     TODO: �ƥ��Ȥ��Խ�ʬ                                *
*							  *
*   Copy right by Kyoto University, 1989		  *
*							  *
**********************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "kumi3.h"

typedef unsigned char uchar;

// �������ޥ�ɤȳ�ĥ�ҤȤ��б�(�Ǹ�� NULL)
struct {
    char	*ext;        // ��ĥ��
    char	*inpCmd;     // �����ѳ������ޥ��
    char	*outCmd;     // �����ѳ������ޥ��
} extcmd[] = {{".jpg", "djpeg", "cjpeg"},
	      {".png", "pngtopnm", "pnmtopng"},
	      {".gif", "giftopnm", "pnmtogif"},
	      {".gz",  "zcat", "gzip -c"},
	      {NULL, NULL, NULL}};

// pnm ������Ȥ��ե�����γ�ĥ��(�Ǹ�� NULL)
char *pnmext[] = {".jpg", ".png", ".gif", ".pnm", ".ppm", ".pgm", ".pbm", NULL};

int _k_debug  = 1;   // debug mode

#define	MAXLEN	2050  // popen ���Ϥ����ޥ�ɤκ���Ĺ

/****************************************************************/
/**** Prototype Declaration ****/
K_IMAGE *fio_allocImage(void);
int fio_freeImage(K_IMAGE *image);
int fio_allocBody(K_IMAGE *image);
int fio_freeBody(K_IMAGE *image);
int fio_setupParam(K_IMAGE *image);
int fio_kumiReadHeader(FILE *fp, K_IMAGE *image, char *magic);
int fio_kumiWriteHeader(FILE *fp, K_IMAGE *image);
int fio_pnmReadHeader(FILE *fp, K_IMAGE *image, char *magic);
int fio_pnmWriteHeader(FILE *fp, K_IMAGE *image);
static void _skipSP(FILE *fp, int skip_comment);
static int _getVal(FILE *fp);

/*********************************************************
*   k_open
*
*    read image from file to buffer
*
**********************************************************/
K_IMAGE *k_open( char *filename )
{
    FILE	*fp;
    K_IMAGE	*image;
    char	*cmd;
    char	pCommand[MAXLEN];
    int		i;
    int		x, y, m;

    char	magic[10];

    // ��¤�Τ��ΰ�����
    if ((image = fio_allocImage())==NULL) return(NULL);

    // ��ĥ�Ҥ˱��������ޥ������
    cmd = NULL;
    for(i = 0; extcmd[i].ext != NULL; i++)
    {
	if ( strstr(filename, extcmd[i].ext) != NULL)
	{
	    cmd = extcmd[i].inpCmd;
	    break;
	}
    }

    //
    // �ե������ open
    //
    if (filename == NULL || strstr(filename, K_STDFILE) != NULL)
    {
	fp = stdin;
    }else if (cmd != NULL){
	snprintf( pCommand, MAXLEN, "%s < %s", cmd, filename);
	fp = popen(pCommand, "r");
    }else{
	fp = fopen(filename, "r");
    }
    if (fp == NULL)
    {
	k_perror("Error: Can't open file for read(%s)!!\n", filename);
	return(NULL);
    }

    //
    // �إå����ɤ߹���
    //  magic ���ɤ߹��ߤ�Ƚ��
    //
    int dummy = fread(magic, 2, 1, fp);
    if (magic[0] == 'P' && magic[1] >= '4' && magic[1] <= '6')
    {
	if (fio_pnmReadHeader(fp, image, magic)) return(NULL);
    }else if (magic[0] == K_MAGIC[0] && magic[1] == K_MAGIC[1]){
	if (fio_kumiReadHeader(fp, image, magic)) return(NULL);
    }else{
	k_perror("Error: Unknown Magic(Not supported image / Can't find file)\n");
	return( NULL );
    }

    // �Ƽ�ѥ�᡼��������˷׻�
    if (fio_setupParam(image)) return(NULL);
    
    // �������ΤΥǡ����ΰ�����
    if (fio_allocBody(image)) return(NULL);

    //
    // ���Τ��ɤ߹���
    //
    if (k_pixeltype(image) == K_BIT)
    {
	// 8���Ǥ� 1byte �˥ѥå�����Ƥ���
	// 8���ܿ��ˤʤ�褦���ΰ���ݤ�ɬ�� (fio_allocBody)
	// �������˵ͤ����
	int		v, val;

	for(y = 0; y < k_ysize(image); y++)
	{
	    for(x = 0; x < k_xsize(image); x+= 8)
	    {
		for(m = 0; m < k_multi(image); m++)
		{
		    val = fgetc(fp);
		    for(i = 0; i < 8; i++)
		    {
			v = ((val & 0x80) != 0)? 1: 0;
			((uchar ***)k_data(image))[m][y][x+i] = v;
			val <<= 1;
		    }
		}
	    }
	}
    }else if (k_pixeltype(image) == K_UCHAR){
	// K_UCHAR �Ϥ褯�Ȥ��Τǡ����̤˾��ʬ��
	uchar	*buff;
	int	lineBytes;
	int	boffs;
	void	*dataPtr;

	lineBytes = k_xsize(image) * k_multi(image) * k_pixelbyte(image);
	buff = (uchar *)malloc(lineBytes);
	for(y = 0; y < k_ysize(image); y++)
	{
	    int dummy = fread(buff, lineBytes, 1, fp);
	    for(m = 0; m < k_multi(image); m++)
	    {
		dataPtr = (image->data)[m][y];
		boffs = m * k_pixelbyte(image);
		for(x = 0; x < k_xsize(image); x++)
		{
		    *(uchar *)(dataPtr + x * k_pixelbyte(image)) = buff[boffs];
		    boffs += k_multi(image) * k_pixelbyte(image);
		}
	    }
	}
	free(buff);
    }else{
	// �ե������� multi ͥ����¤�Ǥ��뤬
	// �ɤ߹��߸�� m, y, x �ν�
#if 0
	// ��ľ�ʼ����������٤�
	for(y = 0; y < k_ysize(image); y++)
	{
	    for(x = 0; x < k_xsize(image); x++)
	    {
		for(m = 0; m < k_multi(image); m++)
		{
		    int dummy = fread((image->data)[m][y]+x*k_pixelbyte(image), 
			  k_pixelbyte(image), 1, fp);
		}
	    }
	}
#endif
	// ¿����®��
	uchar	*buff;
	int	lineBytes;
	int	boffs;
	void	*dataPtr;

	lineBytes = k_xsize(image) * k_multi(image) * k_pixelbyte(image);
	buff = (uchar *)malloc(lineBytes);
	for(y = 0; y < k_ysize(image); y++)
	{
	    int dummy = fread(buff, lineBytes, 1, fp);
	    for(m = 0; m < k_multi(image); m++)
	    {
		boffs = m * k_pixelbyte(image);
		dataPtr = (image->data)[m][y];
		for(x = 0; x < k_xsize(image); x++)
		{
		    memcpy(dataPtr + x * k_pixelbyte(image), 
			   buff + boffs, k_pixelbyte(image));
		    boffs += k_multi(image) * k_pixelbyte(image);
		}
	    }
	}
	free(buff);
    }
      
    //
    // �ե������ close
    //
    if (filename == NULL || strstr(filename, K_STDFILE) != NULL)
    {
	; // Do Nothing 
    }else if (cmd != NULL){
	pclose(fp);
    }else{
	fclose(fp);
    }

    return( image );
}
/*********************************************************
*
*   k_create
*
*    allocate new image header area and copy it.
*    Note: This routine does not copy actual image bodies.
*
**********************************************************/
K_IMAGE *k_create(K_HEAD *header)
{
    K_IMAGE *image;

    // ��¤�Τ��ΰ�����
    if ((image = fio_allocImage()) == NULL) return( NULL );

    // �إå������Ƥ򥳥ԡ�
    *k_header(image) = *header;

    // �Ƽ�ѥ�᡼��������˷׻�
    if (fio_setupParam(image)) return(NULL);

    // �������ΤΥǡ����ΰ�����
    if (fio_allocBody(image)) return(NULL);

    return( image );
}
/*********************************************************
*
*   k_write
*
*   save an image in the buffer to a file  
*
**********************************************************/
int k_write(K_IMAGE *image, char *filename)
{
    FILE	*fp;
    char	*cmd;
    char	pCommand[MAXLEN];
    int		pnmFlag;
    int		i;
    int		x, y, m;

    // ��ĥ�Ҥ˱��������ޥ������
    cmd = NULL;
    for(i = 0; extcmd[i].ext != NULL; i++)
    {
	if ( strstr(filename, extcmd[i].ext) != NULL)
	{
	    cmd = extcmd[i].outCmd;
	    break;
	}
    }

    //
    // �ե������ open
    //
    if (filename == NULL || strstr(filename, K_STDFILE) != NULL)
    {
	fp = stdout;
    }else if (cmd != NULL){
	snprintf( pCommand, MAXLEN, "%s > %s", cmd, filename);
	fp = popen(pCommand, "w");
    }else{
	fp = fopen(filename, "w");
    }
    if (fp == NULL)
    {
	k_perror("Error: Can't create file for write (%s)!!\n", filename);
	return(-1);
    }
    
    //
    // �إå��ν񤭹���
    //

    // PNM������Ƚ��
    pnmFlag = 0;
    for(i = 0; pnmext[i] != NULL; i++)
    {
	if (strstr(filename, pnmext[i]) != NULL)
	{
	    pnmFlag = 1;
	    break;
	}
    }

    // ���줾��η����ǥإå���񤭹���
    if (pnmFlag == 1)
    {
	if (fio_pnmWriteHeader(fp, image)) return(-1);
    }else{
	if (fio_kumiWriteHeader(fp, image)) return(-1);
    }

    //
    // ���Τν���
    //
    if (k_pixeltype(image) == K_BIT)
    {   // K_BIT �ξ���8������� 1byte �˥ѥå�����
	int	val, v;

	for(y = 0; y < k_ysize(image); y++)
	{
	    for(x = 0; x < k_xsize(image); x+=8)
	    {
		for(m = 0; m < k_multi(image); m++)
		{
		    val = 0;
		    for(i = 0; i < 8; i++)
		    {
			val <<= 1;
			v = ((uchar ***)k_data(image))[m][y][x+i];
			val |= (v & 0x01);
		    }
		    fputc(val, fp);
		}
	    }
	}
    }else if (k_pixeltype(image) == K_UCHAR){
	// K_UCHAR �Ϥ褯�Ȥ��Τǡ����̤˾��ʬ��
	uchar	*buff;
	int	lineBytes;
	int	boffs;
	void	*dataPtr;

	lineBytes = k_xsize(image) * k_multi(image) * k_pixelbyte(image);
	buff = (uchar *)malloc(lineBytes);
	for(y = 0; y < k_ysize(image); y++)
	{
	    for(m = 0; m < k_multi(image); m++)
	    {
		dataPtr = (image->data)[m][y];
		boffs = m * k_pixelbyte(image);
		for(x = 0; x < k_xsize(image); x++)
		{
		    buff[boffs] = *(uchar *)(dataPtr + x * k_pixelbyte(image));
		    boffs += k_multi(image) * k_pixelbyte(image);
		}
	    }
	    fwrite(buff, lineBytes, 1, fp);
	}
	free(buff);

    }else{
#if 0
	// multi ���ʬ����Ƥ�������β����ǡ�����
	// multi ͥ��ǥե�����˽���
	// ��ľ�ʼ����������٤�
	for(y = 0; y < k_ysize(image); y++)
	{
	    for(x = 0; x < k_xsize(image); x++)
	    {
		for(m = 0; m < k_multi(image); m++)
		{
		    fwrite((image->data)[m][y]+x*k_pixelbyte(image), 
			   k_pixelbyte(image), 1, fp);
		}
	    }
	}
#endif
	// ¿����®��
	uchar	*buff;
	int	lineBytes;
	int	boffs;
	void	*dataPtr;

	lineBytes = k_xsize(image) * k_multi(image) * k_pixelbyte(image);
	buff = (uchar *)malloc(lineBytes);
	for(y = 0; y < k_ysize(image); y++)
	{
	    for(m = 0; m < k_multi(image); m++)
	    {
		boffs = m * k_pixelbyte(image);
		dataPtr = (image->data)[m][y];
		for(x = 0; x < k_xsize(image); x++)
		{
		    memcpy(buff + boffs,
			   dataPtr + x * k_pixelbyte(image), 
			   k_pixelbyte(image));
		    boffs += k_multi(image) * k_pixelbyte(image);
		}
	    }
	    fwrite(buff, lineBytes, 1, fp);
	}
	free(buff);
    }

    //
    // �ե������ close
    //
    if (filename == NULL || strstr(filename, K_STDFILE) != NULL)
    {
	; // Do Nothing
    }else if (cmd != NULL){
	pclose(fp);
    }else{
	fclose(fp);
    }

    return( 0 );
}
/**********************************************************
*
*   k_close
*
*    free image buffer
*    Note: This routine does not save image buffer.
*
***********************************************************/
int k_close( K_IMAGE *image )
{
    if (fio_freeBody( image )) return(-1);
    if (fio_freeImage( image )) return(-1);

    return( 0 );
}
/**********************************************************
*
*   k_open_frame
*
*    read image with the given frame number
*    fnhead is the printf format
*      ex. "image%06d.ppm"
*
***********************************************************/
K_IMAGE *k_open_frame(char *fnhead, int num)
{
    char fname[MAXLEN];

    snprintf(fname, MAXLEN, fnhead, num);
    return(k_open(fname));
}
/**********************************************************
*
*   k_write_frame
*
*    write image with the given frame number
*    fnhead is the printf format
*      ex. "image%06d.ppm"
*
***********************************************************/
int k_write_frame(K_IMAGE *image, char *fnhead, int num)
{
    char fname[MAXLEN];

    snprintf(fname, MAXLEN, fnhead, num);
    return(k_write(image, fname));
}
/****************************************************************/
// ��¤�Τ��ΰ�����
K_IMAGE *fio_allocImage()
{
    K_IMAGE *image;

    image = (K_IMAGE *)malloc(sizeof(K_IMAGE));
    if (image == NULL)
    {
	k_perror("Error: Can't malloc K_IMAGE!!\n");
	return( NULL );
    }

    image->header = (K_HEAD *)malloc(sizeof(K_HEAD));
    if (image->header == NULL)
    {
	k_perror("Error: Can't malloc K_HEAD!!\n");
	return( NULL );
    }
    return( image );
}

// ��¤�Τ��ΰ����
int fio_freeImage( K_IMAGE *image )
{
    free(image->header);
    free(image);
    return(0);
}


// �������ΤΥǡ����ΰ�����
int fio_allocBody(K_IMAGE *image)
{
    int		m, y;
    int		lineBytes;

    // width ������ 8���ܿ��ڤ�夲�β���ʬ�����ΰ�����( for K_BIT )
    lineBytes = (((k_xsize(image)+7)/8)*8)*k_pixelbyte(image);

    image->data = (void ***)malloc(k_multi(image) * sizeof(void **));
    if (image->data == NULL)
    {
	k_perror("Error: Can't malloc image body(1)!!\n");
	return(-1);
    }
    for(m = 0; m < k_multi(image); m++)
    {
	image->data[m] = (void **)malloc(k_ysize(image) * sizeof(void *));
	if (image->data[m] == NULL)
	{
	    k_perror("Error: Can't malloc image body(2)!!\n");
	    return(-1);
	}
	for(y = 0; y < k_ysize(image); y++)
	{
	    image->data[m][y] = (void *)malloc(lineBytes);
	    if (image->data[m][y] == NULL)
	    {
		k_perror("Error: Can't malloc image body(3)!!\n");
		return(-1);
	    }
	}
    }

    return( 0 );
}

// �������ΤΥǡ����ΰ����
int fio_freeBody(K_IMAGE *image)
{
    int		m, y;

    for(m = 0; m < k_multi(image); m++)
    {
	for(y = 0; y < k_ysize(image); y++)
	{
	    free(image->data[m][y]);
	}
	free(image->data[m]);
    }
    free(image->data);
    return(0);	    
}

// �Ƽ�ѥ�᡼��������˷׻�
int fio_setupParam(K_IMAGE *image)
{
    switch(k_pixeltype(image))
    {
    case K_UCHAR:  image->pixelbyte = 1; break;
    case K_USHORT: image->pixelbyte = 2; break;
    case K_INT:    image->pixelbyte = 4; break;
    case K_FLOAT:  image->pixelbyte = 4; break;
    case K_DOUBLE: image->pixelbyte = 8; break;
    case K_BIT:    image->pixelbyte = 1; break;
    default: k_perror("Error: Unknown Pixel Type (%d)!!\n", k_pixeltype(image));
	return(-1);
    }
    return(0);
}

// KUMI�����Υإå��ɤ߹���
int fio_kumiReadHeader(FILE *fp, K_IMAGE *image, char *magic)
{
    // Magic �λĤ��2ʸ�����ɤ߹���
    int dummy = fread(&magic[2], 2, 1, fp);
    if (magic[2] != K_MAGIC[2] || magic[3] != K_MAGIC[3])
    {
	magic[4] = '\0';
	k_perror("Error: Unknown Magic(%s)!!\n", magic);
	return(-1);
    }

    // �إå��λĤ���ɤ߹���
    int dummy2 = fread(image->header, K_INT_SIZE, K_HEADER_ITEMS, fp);
    return(0);
}

// KUMI�����Υإå�����
int fio_kumiWriteHeader(FILE *fp, K_IMAGE *image)
{
    // Magic �ν���
    fwrite(K_MAGIC, 4, 1, fp);

    // ��С������Ȥθߴ����Τ��������
    image->header->stype     = K_FILTER;
    image->header->imgtype   = 0;
    image->header->order     = 1;

    // �إå��λĤ�����
    fwrite(image->header, K_INT_SIZE, K_HEADER_ITEMS, fp);
    return(0);
}

// PNM�����Υإå��ɤ߹���(��Ƭ 2bytes ���ɤ߹��ߺѤ�)
int fio_pnmReadHeader(FILE *fp, K_IMAGE *image, char *magic)
{
    int		maxval;

    if (magic[1] == '4')
    { // pbm
	_skipSP(fp, 1);
	image->header->xsize = _getVal(fp);
	_skipSP(fp, 1);
	image->header->ysize = _getVal(fp);
	_skipSP(fp, 0);

	image->header->multi = 1;
	image->header->pixeltype = K_BIT;
	image->header->ebit = 8;
    } else if (magic[1] == '5' || magic[1] == '6') {
	// pgm, ppm
	_skipSP(fp, 1);
	image->header->xsize = _getVal(fp);
	_skipSP(fp, 1);
	image->header->ysize = _getVal(fp);
	_skipSP(fp, 1);
	maxval = _getVal(fp);
	_skipSP(fp, 0);

	image->header->pixeltype = K_UCHAR;
	if (magic[1] == '5')
	{      // pgm
	    image->header->multi = 1;
	}else{ // ppm
	    image->header->multi = 3;
	}
	image->header->ebit = ku_get_ebit(maxval);
    }else{
	magic[2] = '\0';
	k_perror("Error: Unkwon Magic netpbm-(%s)\n", magic);
	return(-1);
    }
    return(0);
}

// PNM�����Υإå������
int fio_pnmWriteHeader(FILE *fp, K_IMAGE *image)
{
    int		maxval;

    maxval = (1 << k_ebit(image))-1;
    if (k_pixeltype(image) == K_BIT && k_multi(image) == 1)
    {
        // PBM
	fprintf(fp, "P4\n");
	fprintf(fp, "%d %d\n", k_xsize(image), k_ysize(image));
    }else if (k_pixeltype(image) == K_UCHAR && k_multi(image) == 1){
	// PGM
	fprintf(fp, "P5\n");
	fprintf(fp, "%d %d\n", k_xsize(image), k_ysize(image));
	fprintf(fp, "%d\n", maxval);
    }else if (k_pixeltype(image) == K_UCHAR && k_multi(image) == 3){
        // PPM
	fprintf(fp, "P6\n");
	fprintf(fp, "%d %d\n", k_xsize(image), k_ysize(image));
	fprintf(fp, "%d\n", maxval);
    }else{
	k_perror("Error: Can't write in netpbm format\n\t pixeltype=%d, multi=%d\n",
		 k_pixeltype(image), k_multi(image));
	return(-1);
    }
    return(0);
}

// ���򡤥��֡����ԡ������Ȥ��ɤ����Ф�
//   skip_comment == 0 �ʤ�Х�����('#')�����Ф��ʤ�
//   (raster data �κǽ餬 '#' �ξ����н�
static void _skipSP(FILE *fp, int skip_comment)
{
    int		ch;

    while ((ch = fgetc(fp))!=EOF)
    {
	// �����̵��
	if (ch == ' ' || ch == '\t' || ch == '\n') continue;

	if (skip_comment == 1 && ch == '#')
	{
	    // �����ȤϹ����ޤ��ɤ����Ф�
	    while ((ch = fgetc(fp))!=EOF)
	    {
		if (ch == '\n')
		{
		    break;
		}
	    }
	    continue;
	}

	// ɬ�פʥǡ������ɤ�Ǥ��ޤä��Τ��᤹
	ungetc(ch, fp);
	return;
    }
}

// ���ͤ��ɤ߹��� (0�ʾ�������Τ�, int ��ɽ���Ǥ��ʤ��������)
static int _getVal(FILE *fp)
{
    int		ch;
    int		val;

    val = 0;
    while ((ch = fgetc(fp))!=EOF)
    {
	// ���Ͱʳ��򸫤Ĥ�����ȴ����
	if (ch < '0' || ch > '9') break;

	// ������䤷�ƿ��ͤ��ɲ�
	val = val * 10 + ch - '0';
    }
    return(val);
}

/****************************************************************
 *
 * access �ؿ�
 *   �ǡ�����ѥå������ޤȤޤ�Ȥ��ư���
 *   ���̤ϻȤ�ʤ� ( image.data �򥭥㥹�Ȥ��ƻȤ�����®�� )
 *
 ****************************************************************/
// k_pixelbyte ʬ�Υǡ����� buf �˳�Ǽ
int k_getpix(K_IMAGE *image, int x, int y, int m, void *buf)
{
    if (m < 0 || m >= k_multi(image) || x < 0 || x >= k_xsize(image) ||
	y < 0 || y >= k_ysize(image)) 
    {
	k_perror("Error: Out of Range in getpix %d %d %d\n", x, y, m);
	return(-1);
    }
	
    memcpy(buf,
	   (image->data)[m][y] + x * k_pixelbyte(image), 
	   k_pixelbyte(image));
    return(0);
}

// k_pixelbyte ʬ�Υǡ����� buf ���饳�ԡ�
int k_putpix(K_IMAGE *image, int x, int y, int m, void *buf)
{
    if (m < 0 || m >= k_multi(image) || x < 0 || x >= k_xsize(image) ||
	y < 0 || y >= k_ysize(image)) 
    {
	k_perror("Error: Out of Range in putpix %d %d %d\n", x, y, m);
	return(-1);
    }
	
    memcpy((image->data)[m][y] + x * k_pixelbyte(image), 
	   buf,
	   k_pixelbyte(image));
    return(0);
}
/****************************************************************
*
*	print error message
*
****************************************************************/
#include <stdarg.h>
void k_perror(const char *format, ...)
{
    if ( _k_debug )
    {
	va_list	ap;
	va_start(ap, format);
	vfprintf(stderr, format, ap);
	va_end(ap);
    }
}
/****************************************************************
 *
 *  Utility routines (ku_XXX)
 *
 ****************************************************************/
// val �� [0..255] ���ϰϤ˴ݤ��
int ku_crop_uchar(int val)
{
    if (val < 0)
    {
	val = 0;
    }else if (val > 255){
	val = 255;
    }
    return(val);
}

// maxval ��ɽ���Ǥ���2�ʿ��Ǥη�������
int ku_get_ebit(int maxval)
{
    int		c;

    c = 1;
    while( (maxval >>= 1) > 0)
    {
	c++;
    }
    return(c);
}

// ebit ��ɽ���Ǥ������������
int ku_get_maxval(int ebit)
{
    return((1 << ebit)-1);
}
