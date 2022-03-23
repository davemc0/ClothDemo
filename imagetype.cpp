/**********************************************************************

  imagetype - a generic class for loading and dealing with images

  5/13/97 Paul Rademacher

**********************************************************************/

#include "imagetype.h"
#include "stdinc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int big_endian() {
  short a = 0x0001;
  return (int)*((unsigned char*) (&a));
}

/*************************************************************************
  ImageType::ImageType()
*************************************************************************/

ImageType::ImageType( int w, int h, int num_elements )
{
  pixels = (Byte *) calloc(sizeof( Byte), w * h * num_elements );

  if ( pixels != NULL )
    valid = true;

  x_size             = w;
  y_size             = h;
  this->num_elements = num_elements;
}


/****************************************************************************
  ImageType::set_pixel()
****************************************************************************/

void ImageType::set_pixel( int x, int y, Byte r, Byte g, Byte b )
{
  int   index;
  Byte *p;
  
  index  = get_index(x,y);
  p      = &pixels[index];
  *p	 = r;
  *(p+1) = g;
  *(p+2) = b;
}


/****************************************************************************
  ImageType::set_pixel()
****************************************************************************/

void ImageType::set_pixel( int x, int y, Byte intensity )
{
  int   index;
  Byte *p;

  index = get_index(x,y,0);
  p     = &pixels[index];
  *p	= intensity;
}


/****************************************************************************
  ImageType::flip_h()
****************************************************************************/

void ImageType::flip_h( void )
{
  Byte t;
  int i, j, index1, index2, k;

  for( j=0; j<y_size; j++ ) {
    for( i=0; i<x_size/2; i++ ) {
      index1 = get_index( i, j );
      index2 = get_index( x_size-i-1, j );

      for( k=0; k<num_elements; k++ ) {
	SWAP2( pixels[index1+k], pixels[index2+k], t );
      }
    }
  }
}


/****************************************************************************
  ImageType::flip_v()
****************************************************************************/

void ImageType::flip_v( void )
{
  Byte t;
  int i, j, index1, index2, k;

  for( i=0; i<x_size; i++ ) {
    for( j=0; j<y_size/2; j++ ) {
      index1 = get_index( i, j );
      index2 = get_index( i, y_size-j-1 );

      for( k=0; k<num_elements; k++ ) {
	SWAP2( pixels[index1+k], pixels[index2+k], t );
      }
    }
  }
}

/****************************************************************************
  ImagePPM::read()
****************************************************************************/

int ImagePPM::read( char *name )
{
  FILE    *file;
  int      i, j, r, g, b;
  char     buf[ 1001 ];
  char     type[20];
  int      count;
  int      scale;
  int      buf_size;
  int      binary, index;

  this->valid = false;

  strcpy( this->name, name );

  file = fopen( name, "rb" );
  if ( file == NULL ) {
    fprintf( stderr, "image file '%s' not found\n", name );
    return false;
  }

  /* Now we read the file */
  fgets( buf, 1000, file ); /* Header */
  sscanf( buf, "%s", type );

  if ( !strcmp( type, "P2" )) {
    binary   = false;
    num_elements = 1;
  }
  else if ( !strcmp( type, "P3" )) {
    binary   = false;
    num_elements = 3;
  }
  else if ( !strcmp( type, "P5" )) {
    binary   = true;
    num_elements = 1;
  }
  else if ( !strcmp( type, "P6" )) {
    binary   = true;
    num_elements = 3;
  }

  fgets( buf, 1000, file ); /* Skip over comments */
  while( buf[0] == '#' )
    fgets( buf, 1000, file );

  sscanf( buf, "%d %d", &x_size, &y_size );
  fprintf( stderr, "image: %s    x_size: %d   y_size: %d\n", 
	   name, x_size, y_size );

  fgets( buf, 1000, file );
  sscanf( buf, "%d", &scale );
  
  buf_size  = y_size * x_size * num_elements;
  pixels = (Byte *) calloc( sizeof( Byte ) * buf_size, 1 );
  if ( pixels == NULL )
    return false;

  count = 0;
  for( j = 0; j < y_size; j++ )  {
    //for( j = y_size - 1; j >=0; j-- ) {
    for( i = 0; i < x_size; i++ )   {
      index = get_index( i, j, 0 );

      if ( binary == false )     {
	if ( num_elements == 3 ) {
	  fscanf( file, "%d %d %d", &r, &g, &b );

	  pixels[index+0] = (Byte) r;
	  pixels[index+1] = (Byte) g;
	  pixels[index+2] = (Byte) b;
	}
	else /* num_elements == 1 */ {
	  fscanf( file, "%d", &r );
	  pixels[index] = (Byte) r;
	}
      }
      else /* binary == true */ {
	if ( num_elements == 1 ) {
	  fread(&pixels[index], sizeof(Byte), 1, file ); 
	}
	else /* num_elements == 3 */ {
	  fread(&pixels[index+0], sizeof(Byte), 1, file ); 
	  fread(&pixels[index+1], sizeof(Byte), 1, file ); 
	  fread(&pixels[index+2], sizeof(Byte), 1, file ); 
	}
      }
    }
  }

  fclose( file );
  valid = true;
  return true;
}

/*************************************************************************
  ImagePPM::write( char *name, int binary )
*************************************************************************/

int ImagePPM::write( char *name, int binary )
{
  FILE *file;

  if ( binary )
    file = fopen( name, "wb" );
  else
    file = fopen( name, "w" );
  
  if ( file == NULL ) {
    fprintf( stderr, "ERROR: Could not open file '%s' for writing\n", name );
    return false;
  }
  
  if ( binary AND num_elements == 1 )
    fprintf( file, "P5\n" );
  else if ( binary AND num_elements == 3 )
    fprintf( file, "P6\n" );
  else if ( NOT binary AND num_elements == 1 )
    fprintf( file, "P2\n" );
  else if ( NOT binary AND num_elements == 3 )
    fprintf( file, "P3\n" );
  
  fprintf( file, "%d %d\n255\n", x_size, y_size );
  
  if ( binary ) {
    fwrite( pixels, sizeof(Byte), num_elements * x_size * y_size, file );
  }
  else {
    int i;

    for( i=0; i < x_size * y_size * num_elements; i++ )
      fprintf( file, "%d ", (char) pixels[i] );
  }

  fclose( file );
  return true;	
}


/*****************************************************************************
  ImageTIFF::set_tiff_fields()
*****************************************************************************/

void ImageTIFF::set_tiff_fields( TIFF *tif )
{
  TIFFSetField(tif, TIFFTAG_IMAGEWIDTH, x_size );
  TIFFSetField(tif, TIFFTAG_IMAGELENGTH, y_size );
  TIFFSetField(tif, TIFFTAG_COMPRESSION, COMPRESSION_LZW );
  TIFFSetField(tif, TIFFTAG_PREDICTOR, 2);
  TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);
  TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, 3 );   /* RGB */
  TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, 8);      /* 8-bit */
  TIFFSetField(tif, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
  TIFFSetField(tif, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
  TIFFSetField(tif, TIFFTAG_XRESOLUTION, 600.0);
  TIFFSetField(tif, TIFFTAG_YRESOLUTION, 600.0);
  TIFFSetField(tif, TIFFTAG_RESOLUTIONUNIT, 2);
  TIFFSetField(tif, TIFFTAG_ROWSPERSTRIP, 1);
}

/*****************************************************************************
  ImageTIFF::write()
*****************************************************************************/

int ImageTIFF::write( char *filename )
{
  TIFF *tif;
  unsigned char	*p;
  int            i;
  
  if ( pixels == NULL )		/*** No color info to write ***/
    return false;
  
  tif = TIFFOpen(filename, "w");
  if (tif == NULL)
    return false;
  
  set_tiff_fields(tif);
  
  p = pixels;
	
  /***   write one row of the image at a time   ****/
  for (i = 0; i < y_size; i++) {
    if (TIFFWriteScanline(tif, p, i, 0) < 0){
      TIFFClose(tif);
      return 0;
    }
    p += x_size*3;
  }
  
  TIFFClose(tif);
  return true;
}


/*****************************************************************************
  ImageTIFF::read()
*****************************************************************************/

int ImageTIFF::read( char *filename )
{
  TIFF *tif;
  Byte *tmp_pixels;
  int   index, k, j;

  valid        = false;
  num_elements = 3;

  tif  = TIFFOpen(filename, "r");
  if ( NOT tif ) {
    return false;
  }

  TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &x_size);
  TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &y_size);

  tmp_pixels = (Byte *) malloc(x_size * y_size * 4 * sizeof(Byte));

  if (tmp_pixels == NULL) {
    TIFFClose( tif );
    return false;
  }
	
  if (TIFFReadRGBAImage(tif, x_size, y_size, (unsigned long *) tmp_pixels, 
			0) == NULL) {
    free( tmp_pixels );
    TIFFClose( tif );
    return false;
  }
		
  /*    copy image data to users buffer
        IMPORTANT: TIFFReadRGBAImage return ABGR pixels packed into 
        32-bit words. Also, the rows of file are returned in the
        the reversed order (i.e., ABGR points to the column 0
        of the last row). Thus, the last row is followed by the 
        one before last, ans so on. Inside a row, the order of
        the columns is preserved. 
	*/
        
  pixels = (Byte*) malloc( x_size * y_size * 3 * sizeof(Byte));
  if (pixels == NULL) {
    free( tmp_pixels );
    TIFFClose( tif );
    return false;
  }

  strcpy( this->name, filename );

  unsigned char r, g, b;
  long *a = (long *) tmp_pixels;
  for (k=y_size-1; k>=0; k--) {
    for (j=0; j<x_size; j++) {
      r = (*a & 0x0000FF) >> 0;
      g = (*a & 0x00FF00) >> 8;
      b = (*a & 0xFF0000) >> 16;

      set_pixel( j, k, r, g, b );
      
      a++;
    } 
  }

  fprintf( stderr, "image: %s    x_size: %d   y_size: %d\n", 
	   filename, x_size, y_size );

  valid        = true;

  free( tmp_pixels );
  TIFFClose( tif );

  return true;
}


