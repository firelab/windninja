/*************************************************
*  $Id$                                          *
*  EasyBMP Cross-Platform Windows Bitmap Library * 
*                                                *
*  Author: Paul Macklin                          *
*   email: pmacklin@math.uci.edu                 *
*                                                *
*    file: EasyBMP_Font.h                        *
*    date: 2-21-2005                             *
* version: 1.05.00                               *
*                                                *
*   License: BSD (revised)                       *
* Copyright: 2005-2006 by the EasyBMP Project    * 
*                                                *
* description: draw a simple font                *
*                                                *
*************************************************/

#include "EasyBMP.h"

#ifndef COPYRIGHT_SYMBOL
#define COPYRIGHT_SYMBOL -100 
#endif 

int PrintCopyright( BMP& Image, int TopLeftX, int TopLeftY , int Height , 
                  RGBApixel Color );
int PrintLetter( BMP& Image, char Letter , int TopLeftX, int TopLeftY, int Height , 
                 RGBApixel Color );
//changed to const char * --kss
int PrintString( BMP& Image, const char* String , int TopLeftX, int TopLeftY , int Height , 
                  RGBApixel Color );

//int PrintString( BMP& Image, char* String , int TopLeftX, int TopLeftY , int Height , 
//                  RGBApixel Color );
