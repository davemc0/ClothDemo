/**********************************************************************

  imagetype 
  
  This is a set of classes for reading and writing images.
  The base class, ImageType, can have a variable number of elements
  (typically 1 for grayscale image, or 3 for RGB images).
  Functions are provided for getting and setting pixel values,
  as well as for computing the array index of a given pixel
  given its x and y coordinate.
  Two image formats are currently supported: PPM and TIFF.  PPM is
  a very simple format, while TIFF provides lossless LZW compression.
  
  Functions:
    read()
    write()
    set_pixel()
    get_pixel()
    get_index()
    
   --------------------------------------------------------------------

  5/13/97 Paul Rademacher  (rademach@cs.unc.edu)

**********************************************************************/

#ifndef _IMAGETYPE_H_
#define _IMAGETYPE_H_

#include <string.h>
#include <stdlib.h>
#include "stdinc.h"
#include "tiffio.h"
#include "tiff.h"


/***************************************** class ImageType ***************/

class ImageType
{
public:
  Byte  *pixels;
  int    x_size, y_size, num_elements;
  char   name[ 80 ];
  Bool   valid;         /* Check this variable to see if memory
			   allocation was successful            */

  /* Functions to get and set pixel values */
  void set_pixel( int x, int y, Byte r, Byte g, Byte b );
  void set_pixel( int x, int y, Byte intensity );
  void get_pixel( int x, int y, int r, int g, int b ); /* implement */

  void flip_h( void );   /* Flip image horizontally */
  void flip_v( void );   /* Flip image vertically   */

  /* Functions to get the array index of a particular pixel */
  int get_index( int x, int y, int element ) {
    return ( y * x_size * num_elements ) + ( x * num_elements ) + element;
  };
  int get_index( int x, int y ) {
    return ( y * x_size * num_elements ) + ( x * num_elements );
  };

  /* Constructors */

  ImageType( int w, int h, int num_elements );

  ImageType( void ) {
    x_size  = 0;
    y_size  = 0;
    valid   = false;
    pixels  = NULL;
    strcmp( name, "<none>" );
  }

  /* Destructor */

  ~ImageType() {
    if ( valid == true AND pixels != NULL ) {
      free( pixels );
    }
  }
};


/**************************************** class ImagePPM *******************/

class ImagePPM : public ImageType
{
public:
  int read( char *name );
  int write( char *name, int binary );

  ImagePPM( int w, int h, int num_elements ) : ImageType(w,h,num_elements) {};
  ImagePPM( void ) : ImageType()    {};
};


/**************************************** class ImageTIFF ******************/

class ImageTIFF : public ImageType
{
private:
  void set_tiff_fields( TIFF *tif );

public:
  int read( char *name );
  int write( char *name );

  ImageTIFF( int w, int h ) : ImageType(w,h,3) {};
  ImageTIFF( void ) : ImageType()              {};
};

#endif
