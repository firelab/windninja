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

#include "EasyBMP_Geometry.h"
#include "EasyBMP_Font.h"

//int PrintString( BMP& Image, char* String , int TopLeftX, int TopLeftY , int Height , 
//                  RGBApixel Color )

int PrintString( BMP& Image, const char* String , int TopLeftX, int TopLeftY , int Height , 
                  RGBApixel Color )
{
 int CharNumber = 0;
 int StartX = TopLeftX;
 int Spacing = (int) ebmpRound( 0.2*Height );
 if( Spacing < 3 )
 { Spacing = 3; }
 
 for( CharNumber = 0 ; CharNumber < strlen( String ) ; CharNumber++ )
 {
  int ReturnPosition = PrintLetter( Image , String[CharNumber] , StartX , TopLeftY , Height , Color );
  StartX = ReturnPosition;
  StartX += Spacing;
 }
 return StartX;
}

int PrintLetter( BMP& Image, char Letter , int TopLeftX, int TopLeftY, int Height , 
                  RGBApixel Color )
{
 int Width = (int) floor( 0.6*Height);
 if( Width % 2 != 0 ){ Width++; }
 int Center = (Width)/2;
 
 RGBApixel TempColor;
 TempColor.Red = 0;
 TempColor.Green = 255;
 TempColor.Blue = 0;
 
 double pi = 3.14159265358979;

// if( isalpha(Letter) )
// { Letter = toupper(Letter); }

 if( Letter == COPYRIGHT_SYMBOL )
 {
  return PrintCopyright( Image, TopLeftX, TopLeftY, Height, Color );
 }

 if( Letter == 'a'  )
 {
  double x1 = TopLeftX + 0.25*Height;
  double y1 = TopLeftY + 0.75*Height;
  
  int x2 = ebmpRound(TopLeftX+0.5*Height);
  int y2 = ebmpRound(TopLeftY+0.5*Height);
  
  int x3 = x2;
  int y3 = TopLeftY+Height;
  
  DrawArc(Image,x1,y1,0.25*Height,0,2*pi,Color);
  DrawLine(Image,x2,y2,x3,y3,Color);
 
  return ebmpRound( TopLeftX + 0.5*Height );
 }
 
 if( Letter == 'b' )
 {
  double x1 = TopLeftX + 0.25*Height;
  double y1 = TopLeftY + 0.75*Height;
  
  int x2 = TopLeftX; // ebmpRound(TopLeftX+0.5*Height);
  int y2 = TopLeftY; // ebmpRound(TopLeftY+0.5*Height);
  
  int x3 = x2;
  int y3 = TopLeftY+Height;
  
  DrawArc(Image,x1,y1,0.25*Height,0,2*pi,Color);
  DrawLine(Image,x2,y2,x3,y3,Color);
 
  return ebmpRound( TopLeftX + 0.5*Height );
 } 
 
 if( Letter == 'c' )
 {
  double x1 = TopLeftX + 0.25*Height;
  double y1 = TopLeftY + 0.75*Height;

  DrawArc(Image,x1,y1,0.25*Height,0.25*pi,-0.25*pi,Color);
  return ebmpRound( TopLeftX+0.5*Height);
 }

 if( Letter == 'd' )
 {
  double x1 = TopLeftX + 0.25*Height;
  double y1 = TopLeftY + 0.75*Height;
  
  int x2 = ebmpRound(TopLeftX+0.5*Height);
  int y2 = TopLeftY; // ebmpRound(TopLeftY+0.5*Height);
  
  int x3 = x2;
  int y3 = TopLeftY+Height;
  
  DrawArc(Image,x1,y1,0.25*Height,0,2*pi,Color);
  DrawLine(Image,x2,y2,x3,y3,Color);
 
  return ebmpRound( TopLeftX + 0.5*Height );
 }  
 
 if( Letter == 'e' )
 {
  double x1 = TopLeftX + 0.25*Height;
  double y1 = TopLeftY + 0.75*Height;
  
  int x2 = TopLeftX;
  int y2 = ebmpRound(TopLeftY + 0.75*Height);
  
  int x3 = ebmpRound( TopLeftX+0.5*Height);
  int y3 = y2;

  DrawArc(Image,x1,y1,0.25*Height,0.25*pi,0,Color);
  DrawLine(Image,x2,y2,x3,y3,Color);
  
  return x3;
 } 
 
 if( Letter == 'f' )
 {
  int x1 = ebmpRound( TopLeftX + 0.25*Height);
  int y1 = TopLeftY + Center;
  
  int x2 = x1;
  int y2 = TopLeftY + Height;
  
  int x3 = TopLeftX;
  int y3 = ebmpRound( TopLeftY + 0.5*Height);
  
  int x4 = x1 + (x1-x3);
  int y4 = y3;
  
  double x5 = TopLeftX+0.5*Height;
  double y5 = TopLeftY+0.25*Height;
  
  if( Height % 4 == 3 )
  { x5 -= 1; }
  
  DrawLine(Image,x1,y1,x2,y2,Color);
  DrawLine(Image,x3,y3,x4,y4,Color);
  
  DrawArc(Image,x5,y5,0.25*Height,7*pi/8    ,2*pi,Color);

  return x4;  
 }
 
 if( Letter == 'g' )
 {
  double x1 = TopLeftX+0.25*Height;
  double y1 = TopLeftY+0.75*Height;
 
  int x2 = ebmpRound(TopLeftX+0.5*Height);
  int y2 = ebmpRound(TopLeftY+0.5*Height);
  
  int x3 = x2;
  int y3 = ebmpRound(TopLeftY+1.25*Height);
  
  double x4 = x1; 
  double y4 = TopLeftY+1.25*Height;
 
  DrawArc(Image,x1,y1,0.25*Height,0,2*pi,Color);
  DrawArc(Image,x4,y4,0.25*Height,0,pi,Color);
  DrawLine(Image,x2,y2,x3,y3,Color);
 
  return ebmpRound(TopLeftX+0.5*Height);
 }
 
 if( Letter == 'h' )
 {
  double x1 = TopLeftX + 0.25*Height;
  double y1 = TopLeftY + 0.75*Height;
  
  int x3 = TopLeftX;
  int y3 = TopLeftY; // ebmpRound(TopLeftY+0.5*Height);
  
  int x4 = x3;
  int y4 = TopLeftY + Height;
  
  int x5 = ebmpRound(TopLeftX+0.5*Height);
  int y5 = ebmpRound(TopLeftY+0.75*Height);
  
  int x6 = x5;
  int y6 = y4;
  
  DrawArc(Image,x1,y1,0.25*Height,pi,2*pi,Color);
  
  DrawLine(Image,x3,y3,x4,y4,Color);
  DrawLine(Image,x5,y5,x6,y6,Color);
 
  return x5;
 }  
 
 if( Letter == 'i' )
 {
  int x1 = TopLeftX;
  int y1 = ebmpRound(TopLeftY+0.5*Height);
   
  int x2 = x1;
  int y2 = TopLeftY+Height;
  
  double x3 = x1;
  double y3 = y1 - 2.5;
 
  DrawArc( Image, x3, y3, 0.75 , 0 , 6.3 , Color );
  DrawLine(Image,x1,y1,x2,y2,Color);
  
  return ebmpRound(x1+1.25);
 }
 
 if( Letter == 'j' )
 {
  int x1 = ebmpRound(TopLeftX+0.25*Height);
  int y1 = ebmpRound(TopLeftY+0.5*Height);
   
  int x2 = x1;
  int y2 = ebmpRound(TopLeftY+1.25*Height);
  
  double x3 = x1;
  double y3 = y1 - 2.5;
  
  double x4 = TopLeftX;
  double y4 = TopLeftY+1.25*Height;
 
  DrawArc( Image, x3, y3, 0.75 , 0 , 6.3 , Color );
  DrawLine(Image,x1,y1,x2,y2,Color);
  
  DrawArc(Image,x4,y4,0.25*Height,0,pi,Color);
  
  return x1;
 } 
 
 if( Letter == 'k' )
 {
  int x1 = TopLeftX;
  int y1 = TopLeftY;
  
  int x2 = x1;
  int y2 = TopLeftY+Height;
  
  int x3 = x1;
  int y3 = ebmpRound(TopLeftY+0.75*Height);
  
  int x4 = ebmpRound(TopLeftX+0.3*Height);
  int y4 = ebmpRound(TopLeftY+0.5*Height);
  
  int x5 = x4;
  int y5 = y2;
  
  DrawLine(Image,x1,y1,x2,y2,Color);
  DrawLine(Image,x3,y3,x4,y4,Color);
  DrawLine(Image,x3,y3,x5,y5,Color);
 
  return x5;
 }
 
 if( Letter == 'm' )
 {
  double x1 = TopLeftX + 0.25*Height;
  double y1 = TopLeftY + 0.75*Height;

  double x2 = TopLeftX + 0.75*Height;
  double y2 = y1;  
  
  int x3 = TopLeftX;
  int y3 = ebmpRound(TopLeftY+0.5*Height);
  
  int x4 = x3;
  int y4 = TopLeftY + Height;
  
  int x5 = ebmpRound(TopLeftX+0.5*Height);
  int y5 = ebmpRound(TopLeftY+0.75*Height);
  
  int x6 = x5;
  int y6 = y4;
  
  int x7 = ebmpRound(TopLeftX+Height);
  if( x7 - x5 > x5 - x3 )
  { x7--; }
  
  int y7 = y5;
  
  int x8 = x7;
  int y8 = y4;
  
  
  DrawArc(Image,x1,y1,0.25*Height,pi,2*pi,Color);
  DrawArc(Image,x2,y2,0.25*Height,pi,2*pi,Color);
  
  DrawLine(Image,x3,y3,x4,y4,Color);
  DrawLine(Image,x5,y5,x6,y6,Color);
  DrawLine(Image,x7,y7,x8,y8,Color);
 
  return x7;
 }
 
 if( Letter == 'n' )
 {
  double x1 = TopLeftX + 0.25*Height;
  double y1 = TopLeftY + 0.75*Height;
  
  int x3 = TopLeftX;
  int y3 = ebmpRound(TopLeftY+0.5*Height);
  
  int x4 = x3;
  int y4 = TopLeftY + Height;
  
  int x5 = ebmpRound(TopLeftX+0.5*Height);
  int y5 = ebmpRound(TopLeftY+0.75*Height);
  
  int x6 = x5;
  int y6 = y4;
  
  DrawArc(Image,x1,y1,0.25*Height,pi,2*pi,Color);
  
  DrawLine(Image,x3,y3,x4,y4,Color);
  DrawLine(Image,x5,y5,x6,y6,Color);
 
  return x5;
 } 
 
 if( Letter == 'o' )
 {
  double x1 = TopLeftX + 0.25*Height;
  double y1 = TopLeftY + 0.75*Height;
  
  DrawArc(Image,x1,y1,0.25*Height,0,2*pi,Color);
 
  return ebmpRound(TopLeftX + 0.5*Height);
 }   
 
 if( Letter == 'p' )
 {
  double x1 = TopLeftX + 0.25*Height;
  double y1 = TopLeftY + 0.75*Height;
  
  int x2 = TopLeftX;
  int y2 = ebmpRound(TopLeftY + 0.5*Height);
  
  int x3 = x2;
  int y3 = ebmpRound(TopLeftY + 1.5*Height);

  DrawArc(Image,x1,y1,0.25*Height,0,2*pi,Color);
  DrawLine(Image,x2,y2,x3,y3,Color);

  return ebmpRound( TopLeftX + 0.5*Height );
 }
 
 if( Letter == 'q' )
 {
  double x1 = TopLeftX+0.25*Height;
  double y1 = TopLeftY+0.75*Height;
 
  int x2 = ebmpRound(TopLeftX+0.5*Height);
  int y2 = ebmpRound(TopLeftY+0.5*Height);
  
  int x3 = x2;
  int y3 = ebmpRound(TopLeftY+1.5*Height);

  int x4 = ebmpRound(x3+0.2*Height);
  int y4 = ebmpRound(y3-0.2*Height);
 
  DrawArc(Image,x1,y1,0.25*Height,0,2*pi,Color);
  DrawLine(Image,x2,y2,x3,y3,Color);
  DrawLine(Image,x3,y3,x4,y4,Color);
 
  return ebmpRound(TopLeftX+0.5*Height);
 }
 
 if( Letter == 'r' )
 {
  double x1 = TopLeftX + 0.25*Height;
  double y1 = TopLeftY + 0.75*Height;
  
  int x3 = TopLeftX;
  int y3 = ebmpRound(TopLeftY+0.5*Height);
  
  int x4 = x3;
  int y4 = TopLeftY + Height;

/*  
  int x5 = ebmpRound(TopLeftX+0.5*Height);
  int y5 = ebmpRound(TopLeftY+0.75*Height);
  
  int x6 = x5;
  int y6 = y4;
*/  
  
  DrawArc(Image,x1,y1,0.25*Height,pi,2*pi,Color);
  
  DrawLine(Image,x3,y3,x4,y4,Color);
//  DrawLine(Image,x5,y5,x6,y6,Color);
 
  return ebmpRound(TopLeftX+0.5*Height);
 }  
 
 if( Letter == 's' )
 {
  double x1 = TopLeftX+0.125*Height;
  double y1 = TopLeftY+0.625*Height;

  double x2 = x1;
  double y2 = (TopLeftY+0.875*Height);
  
  double difference = (TopLeftY+Height)-y2;
  double MaxAngle1 = 0;
  double MaxAngle2 = pi;
  if( difference < 1.5 )
  { difference = 1.5; MaxAngle1 = 0; MaxAngle2 = 1.5; x1 = TopLeftX + difference; x2 = x1; }
  
  y1 = y2 - 2*difference;
  
  DrawArc(Image,x1,y1,difference,0.5*pi,MaxAngle1,Color);
  DrawArc(Image,x2,y2,difference,-0.5*pi,pi,Color);
 
  return ebmpRound(TopLeftX+2*difference);
 }
 
 if( Letter == 't' )
 {
  int x1 = ebmpRound( TopLeftX + 0.25*Height);
  int y1 = TopLeftY + Center;
  
  int x2 = x1;
  int y2 = TopLeftY + Height;
  
  int x3 = TopLeftX;
  int y3 = ebmpRound( TopLeftY + 0.5*Height);
  
  int x4 = x1 + (x1-x3);
  int y4 = y3;
  
  DrawLine(Image,x1,y1,x2,y2,Color);
  DrawLine(Image,x3,y3,x4,y4,Color);

  return x4; 
 }
 
 if( Letter == 'u' )
 {
  double x1 = TopLeftX + 0.25*Height;
  double y1 = TopLeftY + 0.75*Height;
  
  int x3 = TopLeftX;
  int y3 = ebmpRound(TopLeftY+0.5*Height);
  
  int x4 = x3;
  int y4 = ebmpRound(TopLeftY+0.75*Height);//  + Height;
  
  int x5 = ebmpRound(TopLeftX+0.5*Height);
  int y5 = TopLeftY + Height;// ebmpRound(TopLeftY+0.75*Height);
  
  int x6 = x5;
  int y6 = y3;
  
  DrawArc(Image,x1,y1,0.25*Height,0,pi,Color);
  
  DrawLine(Image,x3,y3,x4,y4,Color);
  DrawLine(Image,x5,y5,x6,y6,Color);
 
  return x5;
 }  
 
 if( Letter == 'v' )
 {
  int x1 = TopLeftX;
  int y1 = ebmpRound(TopLeftY+0.5*Height);
  
  int x2 = ebmpRound( TopLeftX+0.2*Height);
  int y2 = TopLeftY+Height;
  
  int x3 = ebmpRound( TopLeftX+0.4*Height);
  int y3 = y1;
  
  DrawLine(Image,x1,y1,x2,y2,Color);
  DrawLine(Image,x2,y2,x3,y3,Color);
 
  return x3;
 }

 if( Letter == 'w' )
 {
  int x1 = TopLeftX;
  int y1 = ebmpRound(TopLeftY+0.5*Height);
  
  int x2 = ebmpRound(TopLeftX+0.2*Height);
  int y2 = TopLeftY+Height;
  
  int x3 = ebmpRound(TopLeftX+0.4*Height);
  int y3 = y1;
  
  int x4 = ebmpRound(x3+0.2*Height);
  int y4 = y2;
  
  int x5 = ebmpRound(x3+0.4*Height);
  int y5 = y1;
  
  DrawLine(Image,x1,y1,x2,y2,Color);
  DrawLine(Image,x2,y2,x3,y3,Color);
  DrawLine(Image,x3,y3,x4,y4,Color);
  DrawLine(Image,x4,y4,x5,y5,Color);
  
  return x5;
 }
 
 if( Letter == 'x' )
 {
  int x1 = TopLeftX;
  int y1 = ebmpRound(TopLeftY+0.5*Height);
  
  int x2 = ebmpRound(TopLeftX+0.5*Height);
  int y2 = y1;
  
  int x3 = x1;
  int y3 = TopLeftY+Height;
  
  int x4 = x2;
  int y4 = y3;
  
  DrawLine(Image,x1,y1,x4,y4,Color);
  DrawLine(Image,x2,y2,x3,y3,Color);
 
  return x4;
 }
 
 if( Letter == 'y' )
 {
  int x1 = TopLeftX;
  int y1 = ebmpRound(TopLeftY+0.5*Height);
  
  int x2 = ebmpRound(TopLeftX+0.5*Height);
  int y2 = y1;
  
  int x3 = ebmpRound(x1+0.25*Height);
  int y3 = TopLeftY+Height;
  
  int x4 = x1;
  int y4 = ebmpRound(TopLeftY+1.25*Height)+1;
  
  DrawLine(Image,x1,y1,x3,y3,Color);
  DrawLine(Image,x2,y2,x4,y4,Color);
 
  return x2;
 } 
 
 if( Letter == 'z' )
 {
  int x1 = TopLeftX;
  int y1 = ebmpRound(TopLeftY+0.5*Height);
  
  int x2 = ebmpRound(TopLeftX+0.5*Height);
  int y2 = y1;
  
  int x3 = x1;
  int y3 = TopLeftY+Height;
  
  int x4 = x2;
  int y4 = y3;
  
  DrawLine(Image,x1,y1,x2,y2,Color);
  DrawLine(Image,x2,y2,x3,y3,Color);
  DrawLine(Image,x3,y3,x4,y4,Color);
  
  return x4;
 }
 
 if( Letter == 'A' )
 {
  // define some control points
  
  int x1 = TopLeftX;
  int y1 = TopLeftY+Height;
  
  int x2 = TopLeftX + ebmpRound( 0.3*Height );
  int y2 = TopLeftY;
  
  int x3 = TopLeftX + ebmpRound( 0.6*Height );
  int y3 = y1;
  
  int x4 = TopLeftX + ebmpRound( 0.1*Height );
  int y4 = ebmpRound( y1 - Height/3.0 );
  
  int x5 = ebmpRound( x3 - 0.1*Height ); 
  int y5 = y4;
 
  DrawLine( Image , x1, y1, x2, y2, Color );
  DrawLine( Image , x2, y2, x3, y3, Color );
  DrawLine( Image , x4, y4, x5, y5, Color );
 
  return x3; 
 }
 
 if( Letter == 'B' )
 {
  int x1 = TopLeftX;
  int y1 = TopLeftY+Height;
  
  int x2 = TopLeftX;
  int y2 = TopLeftY;
  
  int x3 = TopLeftX + Center; // (int) ebmpRound( 0.3*Height );
  int y3 = TopLeftY;
  
  int x4 = x1;
  int y4 = (int) ebmpRound( TopLeftY + 0.5*Height );

  int x5 = x3;
  int y5 = y4;
  
  int x6 = x3;
  int y6 = y1;

  // centers of the circles
  
  double x7 = x3;
  double y7 = ( TopLeftY + 0.25*Height );

  double x8 = x3;
  double y8 = ( TopLeftY + 0.75*Height );
  
  DrawLine( Image , x1, y1, x2, y2, Color );
  DrawLine( Image , x2, y2, x3, y3, Color );
  
  DrawLine( Image , x2, y2, x3, y3, Color );  
  DrawLine( Image , x4, y4, x5, y5, Color );  
  DrawLine( Image , x1, y1, x6, y6, Color );
  
  DrawArc( Image, x7, y7 , 0.25*Height , -1.57079632679490 , 1.57079632679490 , Color ); 
  DrawArc( Image, x8, y8 , 0.25*Height , -1.57079632679490 , 1.57079632679490 , Color ); 
  
  return ebmpRound( TopLeftX + Center + 0.25*Height); 
 }
 
 if( Letter == 'C' )
 {
  double x5 = TopLeftX + Center; 
  double y5 = TopLeftY + Center; 

  double x6 = x5;
  double y6 = TopLeftY + Height - Center; 
  
  int x7 = TopLeftX;
  int y7 = (int) y5; 
  
  int x8 = x7;
  int y8 = (int) y6; 

  DrawArc( Image, x5, y5 , Center , -3.14159265358979 , 0 , Color ); 
  DrawArc( Image, x6, y6 , Center , 0 , 3.14159265358979, Color ); 
  
  DrawLine( Image , x7, y7, x8, y8, Color );
 
  return TopLeftX + Width; //  ebmpRound(TopLeftX+0.6*Height);
 } 
 
 if( Letter == 'D' )
 {
  int x1 = TopLeftX;
  int y1 = TopLeftY;
  
  int x2 = TopLeftX + Center; // ebmpRound( TopLeftX + 0.3*Height );
  int y2 = y1;
  
  int x3 = TopLeftX;
  int y3 = TopLeftY+Height;
  
  int x4 = x2;
  int y4 = y3;
  
  double x5 = x2;
  double y5 = TopLeftY + Center; // TopLeftY + 0.3*Height;

  double x6 = x2;
  double y6 = TopLeftY + Height - Center; // TopLeftY + 0.7*Height;
  
  int x7 = TopLeftX + Width; // ebmpRound(TopLeftX + 0.6*Height);
  int y7 = (int) y5; // ebmpRound( y5 );
  
  int x8 = x7;
  int y8 = (int) y6; //  ebmpRound( y6 );
  
  DrawLine( Image , x1, y1, x2, y2, Color );
  DrawLine( Image , x1, y1, x3, y3, Color );
  DrawLine( Image , x3, y3, x4, y4, Color );

  DrawArc( Image, x5, y5 , Center , -1.57079632679490 , 0 , Color ); 
  DrawArc( Image, x6, y6 , Center , 0 , 1.57079632679490 , Color ); 

  DrawLine( Image , x7, y7, x8, y8, Color );
 
  return x7;
 }
 
 if( Letter == 'E' )
 {
  int x1 = TopLeftX;
  int y1 = TopLeftY;
  
  int x2 = x1;
  int y2 = TopLeftY + Height;
  
  int x3 = TopLeftX + Width;
  int y3 = TopLeftY;
  
  int x4 = TopLeftX;
  int y4 = ebmpRound( TopLeftY + 0.5*Height);
  
  int x5 = ebmpRound( TopLeftX + 0.45*Height);
  int y5 = y4;
  
  int x6 = TopLeftX + Width;
  int y6 = y2;

  DrawLine( Image , x1, y1, x2, y2, Color );
  DrawLine( Image , x1, y1, x3, y3, Color );
  DrawLine( Image , x4, y4, x5, y5, Color );
  DrawLine( Image , x2, y2, x6, y6, Color );
  
  return x6;
 }
 
 if( Letter == 'F' )
 {
  int x1 = TopLeftX;
  int y1 = TopLeftY;
  
  int x2 = x1;
  int y2 = TopLeftY + Height;
  
  int x3 = TopLeftX + Width;
  int y3 = TopLeftY;
  
  int x4 = TopLeftX;
  int y4 = ebmpRound( TopLeftY + 0.5*Height);
  
  int x5 = ebmpRound( TopLeftX + 0.45*Height); // x3;
  int y5 = y4;

  DrawLine( Image , x1, y1, x2, y2, Color );
  DrawLine( Image , x1, y1, x3, y3, Color );
  DrawLine( Image , x4, y4, x5, y5, Color );
  
  return x3;
 } 
 
 if( Letter == 'G' )
 {
  double x5 = TopLeftX + Center; 
  double y5 = TopLeftY + Center; 

  double x6 = x5;
  double y6 = TopLeftY + Height - Center; 
  
  int x7 = TopLeftX;
  int y7 = (int) y5; 
  
  int x8 = x7;
  int y8 = (int) y6; 
  
  int x9 = TopLeftX + Center; // ebmpRound( TopLeftX + 0.45*Height );
  int y9 = ebmpRound( TopLeftY + 0.6*Height );
  
  int x10 = TopLeftX + Width; // ebmpRound( TopLeftX + 0.65*Height );
  int y10 = y9;
  
  int x11 = x10;
  int y11 = TopLeftY + Height;

  DrawArc( Image, x5, y5 , Center , -3.14159265358979 , 0 , Color ); 
  DrawArc( Image, x6, y6 , Center , 0 , 3.14159265358979, Color ); 
  
  DrawLine( Image , x7, y7, x8, y8, Color );
  
  DrawLine( Image , x9, y9, x10 ,y10 ,Color );
  DrawLine( Image , x10, y10, x11 ,y11 ,Color );
 
  return x10; //  ebmpRound(TopLeftX+0.6*Height);
 }  
 
 if( Letter == 'H' )
 {
  int x1 = TopLeftX;
  int y1 = TopLeftY;
  
  int x2 = x1;
  int y2 = TopLeftY + Height;
  
  int x3 = TopLeftX + Width;
  int y3 = y1;
  
  int x4 = x3;
  int y4 = y2;
  
  int x5 = x1;
  int y5 = ebmpRound( TopLeftY + 0.5*Height );
  
  int x6 = x3;
  int y6 = y5;
  
  DrawLine( Image , x1, y1, x2, y2, Color );
  DrawLine( Image , x3, y3, x4, y4, Color );
  DrawLine( Image , x5, y5, x6, y6, Color );

  return x3;
 }
 
 if( Letter == 'I' )
 {
  int x1 = ebmpRound( TopLeftX + Height*0.05);
  int y1 = TopLeftY;
  
  int x2 = ebmpRound(x1 + 0.4*Height);
  int y2 = y1;
  
  int x3 = ebmpRound( x1 + 0.2*Height);
  int y3 = y1;
  
  int x4 = x1;
  int y4 = TopLeftY+Height;
  
  int x5 = x2;
  int y5 = y4;
  
  int x6 = x3;
  int y6 = y4;
  
  DrawLine( Image , x1, y1, x2, y2, Color );
  DrawLine( Image , x3, y3, x6, y6, Color );
  DrawLine( Image , x4, y4, x5, y5, Color );
  
  return x2;
 }
 
 if( Letter == 'J' )
 {
  int x1 = TopLeftX + Width;
  int y1 = TopLeftY;
  
  int x2 = x1;
  int y2 = TopLeftY + Height - Center;
  
  double x3 = TopLeftX + Center;
  double y3 = y2;
  
  DrawLine( Image , x1, y1, x2, y2, Color );
  DrawArc( Image, x3, y3, Center , 0 , 1.1*pi , Color );
  return x1;
 }
 
 if( Letter == 'K' )
 {
  int x1 = TopLeftX;
  int y1 = TopLeftY;
  
  int x2 = x1;
  int y2 = TopLeftY + Height;
  
  int x3 = x1;
  int y3 = TopLeftY + Height - Center; // ebmpRound( TopLeftY + 0.6*Height );
  
  int x4 = TopLeftX + Width;
  int y4 = y1;
  
  int x5 = TopLeftX + Center;
  int y5 = ebmpRound( TopLeftY + 0.5*Height );
  
  int x6 = x4;
  int y6 = y2;
  
  DrawLine( Image, x1, y1, x2, y2, Color );
  DrawLine( Image, x3, y3, x4, y4, Color );
  DrawLine( Image, x5, y5, x6, y6, Color );
 
  return x4;
 }
 
 if( Letter == 'L' )
 {
  int x1 = TopLeftX;
  int y1 = TopLeftY;
  
  int x2 = x1;
  int y2 = TopLeftY + Height;
  
  int x3 = TopLeftX + Width;
  int y3 = y2;
  
  DrawLine( Image , x1, y1, x2, y2, Color );
  DrawLine( Image , x2, y2, x3, y3, Color );

  return x3;
 }
  
 if( Letter == 'M' )
 {
  int x1 = TopLeftX;
  int y1 = TopLeftY;
  
  int x2 = x1;
  int y2 = TopLeftY + Height;
  
  int x3 = TopLeftX + Width;
  int y3 = y1;
  
  int x4 = x3;
  int y4 = y2;
  
  int x5 = TopLeftX + Center;
  int y5 = y4;

  DrawLine( Image , x1, y1, x2, y2, Color );
  DrawLine( Image , x3, y3, x4, y4, Color );
  DrawLine( Image , x1, y1, x5, y5, Color );
  DrawLine( Image , x3, y3, x5, y5, Color );
 
  return x3;
 } 
 
 if( Letter == 'N' )
 {
  int x1 = TopLeftX;
  int y1 = TopLeftY;
  
  int x2 = x1;
  int y2 = TopLeftY + Height;
  
  int x3 = TopLeftX + Width;
  int y3 = y1;
  
  int x4 = x3;
  int y4 = y2;

  DrawLine( Image , x1, y1, x2, y2, Color );
  DrawLine( Image , x3, y3, x4, y4, Color );
  DrawLine( Image , x1, y1, x4, y4, Color );
 
  return x3;
 }

 if( Letter == 'P' )
 {
  int x1 = TopLeftX;
  int y1 = TopLeftY+Height;
  
  int x2 = TopLeftX;
  int y2 = TopLeftY;
  
  int x3 = TopLeftX + Center; 
  int y3 = TopLeftY;
  
  int x4 = x1;
  int y4 = ebmpRound( TopLeftY + 0.5*Height );

  int x5 = x3;
  int y5 = y4;
  
  int x6 = x3;
  int y6 = y1;

  // centers of the circles
  
  double x7 = x3;
  double y7 = ( 0.5*(y3+y5) );

  DrawLine( Image , x1, y1, x2, y2, Color );
  DrawLine( Image , x2, y2, x3, y3, Color );
  
  DrawLine( Image , x2, y2, x3, y3, Color );  
  DrawLine( Image , x4, y4, x5, y5, Color );  
  
  DrawArc( Image, x7, y7 , 0.25*Height , -1.57079632679490 , 1.57079632679490 , Color ); 
  
  return ebmpRound( TopLeftX + Center + 0.25*Height); 
 }
 
 if( Letter == 'Q' )
 {
  int x1 = TopLeftX;
  int y1 = TopLeftY + Height - Center;
  
  int x2 = x1;
  int y2 = TopLeftY + Center; 
  
  int x3 = TopLeftX + Width;
  int y3 = y2;
  
  int x4 = x3;
  int y4 = y1;

  // centers of the circles
  
  double x5 = TopLeftX + Center; 
  double y5 = TopLeftY + Center; 

  double x6 = x5;
  double y6 = TopLeftY + Height - Center; 
  
  // more points
  
  int x7 = TopLeftX + Width;
  int y7 = TopLeftY + Height;
  
  int x8 = x7 - Center;
  int y8 = y7 - Center;
  
  DrawLine( Image , x1, y1, x2, y2, Color );
  DrawLine( Image , x3, y3, x4, y4, Color );
  
  DrawArc( Image, x5, y5 , Center , 3.14159265358979 , 6.28318530717959 , Color ); 
  DrawArc( Image, x6, y6 , Center , 0 ,  3.14159265358979 , Color ); 
  
  DrawLine( Image , x7, y7 , x8, y8 , Color );
  
  return x3; 
 } 
 
 if( Letter == 'R' )
 {
  int x1 = TopLeftX;
  int y1 = TopLeftY+Height;
  
  int x2 = TopLeftX;
  int y2 = TopLeftY;
  
  int x3 = TopLeftX + Center; 
  int y3 = TopLeftY;
  
  int x4 = x1;
  int y4 = ebmpRound( TopLeftY + 0.5*Height );

  int x5 = x3;
  int y5 = y4;
  
  int x6 = x3;
  int y6 = y1;
  
  // centers of the circles
  
  double x7 = x3;
  double y7 = ( 0.5*(y3+y5) );
  
  // more
  
  int x8 = TopLeftX + Width;
  int y8 = y1;
  
  int x9 = ebmpRound( TopLeftX + 0.25*Height);
  int y9 = y4;  

  DrawLine( Image , x1, y1, x2, y2, Color );
  DrawLine( Image , x2, y2, x3, y3, Color );
  
  DrawLine( Image , x2, y2, x3, y3, Color );  
  DrawLine( Image , x4, y4, x5, y5, Color );  
  
  DrawArc( Image, x7, y7 , 0.25*Height , -1.57079632679490 , 1.57079632679490 , Color ); 
  
  DrawLine( Image , x8, y8, x9, y9 , Color);
  
  return TopLeftX + Width; 
 } 
 
 if( Letter == 'T' )
 {
  int x1 = TopLeftX;
  int y1 = TopLeftY;
    
  int x2 = TopLeftX + Width;
  int y2 = y1; 
  
  int x3 = TopLeftX + Center;
  int y3 = y1;
  
  int x4 = x3;
  int y4 = TopLeftY + Height;
  
  DrawLine( Image , x1, y1, x2, y2, Color );
  DrawLine( Image , x3, y3, x4, y4, Color );
 
  return x2;
 }
 
 if( Letter == 'S' )
 {
  double x1 = TopLeftX + 0.25*Height;
  double y1 = TopLeftY + 0.25*Height;
  
  double x2 = x1;
  double y2 = TopLeftY + 0.75*Height;
  
  DrawArc( Image, x1, y1 , 0.25*Height, 1.5707963267948 , 6.28318530717 , Color );
  DrawArc( Image, x2, y2 , 0.25*Height, -1.5707963267948 ,3.1415926535897 , Color );
  
  return ebmpRound( TopLeftX + 0.5*Height );
 }
 
 if( Letter == 'U' )
 {
  int x1 = TopLeftX;
  int y1 = TopLeftY + Height - Center;
  
  int x2 = x1;
  int y2 = TopLeftY; 
  
  int x3 = TopLeftX + Width;
  int y3 = y2;
  
  int x4 = x3;
  int y4 = y1;

  // centers of the circle
  
  double x5 = TopLeftX + Center;
  double y5 = TopLeftY + Height - Center; 
  
  DrawLine( Image , x1, y1, x2, y2, Color );
  DrawLine( Image , x3, y3, x4, y4, Color );
  
  DrawArc( Image, x5, y5 , Center , 0 , 3.14159265358979 , Color ); 

  return x3; 
 }

 if( Letter == 'V' )
 {
  int x1 = TopLeftX;
  int y1 = TopLeftY;
  
  int x2 = TopLeftX + Width;
  int y2 = y1;
  
  int x3 = TopLeftX + Center;
  int y3 = TopLeftY + Height;
  
  DrawLine( Image , x1, y1, x3, y3, Color );
  DrawLine( Image , x2, y2, x3, y3, Color );

  return x2;
 } 
 
 if( Letter == 'W' )
 {
  int x1 = TopLeftX;
  int y1 = TopLeftY;
  
  int x2 = ebmpRound(TopLeftX + 0.4*Height);
  int y2 = y1;
  
  int x3 = ebmpRound( TopLeftX + 0.8*Height);
  int y3 = y1;
  
  int x4 = ebmpRound( TopLeftX + 0.2*Height );
  int y4 = TopLeftY + Height;
  
  int x5 = ebmpRound( TopLeftX + 0.6*Height );
  int y5 = y4;
  
  DrawLine( Image , x1, y1, x4, y4, Color );
  DrawLine( Image , x4, y4, x2, y2, Color );
  DrawLine( Image , x2, y2, x5, y5, Color );
  DrawLine( Image , x5, y5, x3, y3, Color );

  return x3;
 }  
 
 if( Letter == 'X' )
 {
  int x1 = TopLeftX;
  int y1 = TopLeftY;
  
  int x2 = TopLeftX + Width;
  int y2 = y1;
  
  int x3 = x1;
  int y3 = TopLeftY + Height;
  
  int x4 = x2;
  int y4 = y3;
  
  DrawLine( Image , x1 , y1, x4, y4 , Color );
  DrawLine( Image , x2 , y2, x3, y3 , Color );
 
  return x2;
 }
 
 if( Letter == 'Y' )
 {
  int x1 = TopLeftX;
  int y1 = TopLeftY;
  
  int x2 = TopLeftX + Width;
  int y2 = y1;
  
  int x3 = TopLeftX + Center;
  int y3 = ebmpRound( TopLeftY + 0.5*Height);
  
  int x4 = x3;
  int y4 = TopLeftY + Height;
  
  DrawLine( Image , x1, y1, x3, y3, Color );
  DrawLine( Image , x2, y2, x3, y3, Color );
  DrawLine( Image , x3, y3, x4, y4, Color );

  return x2;
 }
 
 if( Letter == 'Z' )
 {
  int x1 = TopLeftX;
  int y1 = TopLeftY;
  
  int x2 = TopLeftX + Width;
  int y2 = y1;
  
  int x3 = x1;
  int y3 = TopLeftY + Height;
  
  int x4 = x2;
  int y4 = y3;

  
  DrawLine( Image , x1 , y1, x2, y2 , Color );
  DrawLine( Image , x2 , y2, x3, y3 , Color );
  DrawLine( Image , x3 , y3, x4, y4 , Color );
 
  return x2;
 } 
 
 // space 
 
 if( Letter == ' ' || Letter == '\t' )
 {
  return ebmpRound( TopLeftX + 0.5*Height );
 }

 // numbers

 if( Letter == '0' || Letter == 'O' )
 {
  int x1 = TopLeftX;
  int y1 = TopLeftY + Height - Center;
  
  int x2 = x1;
  int y2 = TopLeftY + Center; 
  
  int x3 = TopLeftX + Width;
  int y3 = y2;
  
  int x4 = x3;
  int y4 = y1;

  // centers of the circles
  
  double x5 = TopLeftX + Center; 
  double y5 = TopLeftY + Center; 

  double x6 = x5;
  double y6 = TopLeftY + Height - Center; 
  
  DrawLine( Image , x1, y1, x2, y2, Color );
  DrawLine( Image , x3, y3, x4, y4, Color );
  
  DrawArc( Image, x5, y5 , Center , 3.14159265358979 , 6.28318530717959 , Color ); 
  DrawArc( Image, x6, y6 , Center , 0 ,  3.14159265358979 , Color ); 
  
  return x3; 
 }
 
 if( Letter == '1' )
 {
  int x1 = ebmpRound( TopLeftX + Height*0.05);
  int y1 = TopLeftY+Height;
  
  int x2 = ebmpRound(x1 + 0.4*Height);
  int y2 = y1;
  
  int x3 = ebmpRound( x1 + 0.2*Height);
  int y3 = y1;
  
  int x4 = x3;
  int y4 = TopLeftY;
  
  int x5 = ebmpRound(x1 + 0.05*Height);
  int y5 = ebmpRound(TopLeftY+ 0.2*Height);
  
  DrawLine( Image , x1, y1, x2, y2, Color );
  DrawLine( Image , x3, y3, x4, y4, Color );
  DrawLine( Image , x4, y4, x5, y5, Color );
  
  return ebmpRound(x2 + Height*0.05);
 } 
 
 if( Letter == '2' )
 {
  int x1 = TopLeftX + Width; // ebmpRound( TopLeftX + 0.6*Height );
  int y1 = TopLeftY+Height;
  
  int x2 = TopLeftX;
  int y2 = y1;
  
  int x3 = x1;
  int y3 = TopLeftY + Center; // ebmpRound( TopLeftY + 0.3*Height )+1;
  
  double x4 = TopLeftX + Center; // TopLeftX + (0.3*Height);
  double y4 = TopLeftY + Center; // TopLeftY + (0.3*Height);
  
  DrawLine( Image , x1, y1, x2, y2, Color );
  DrawLine( Image , x2, y2, x3, y3, Color );
  
  DrawArc( Image , x4 , y4 , Center , 2.74 , 6.3 , Color );
  
  return x1;
 } 
 
 if( Letter == '3' )
 {
  double x1 = TopLeftX + (0.25*Height);
  double y1 = TopLeftY + (0.25*Height);
  
  double x2 = x1;
  double y2 = TopLeftY + (0.75*Height);
  
  int x3 = ebmpRound( TopLeftX + 0.3*Height);
  int y3 = ebmpRound( TopLeftY + 0.5*Height);
  
  int x4 = ebmpRound( TopLeftX + 0.2*Height);
  int y4 = y3;
  
  DrawArc( Image , x1 , y1 , 0.25*Height , -3.14159265358979 , 1.57079632679490 , Color );
  DrawArc( Image , x2 , y2 , 0.25*Height , -1.57079632679490 , 3.14159265358979 , Color );
  DrawLine( Image , x3, y3, x4, y4, Color );

  return ebmpRound(TopLeftX + 0.5*Height); 
 }  
 
 if( Letter == '4' )
 {
  // define some control points
  
  int x1 = TopLeftX+Width;
  int y1 = TopLeftY+ebmpRound(Height*2.0/3.0);
  
  int x2 = TopLeftX;
  int y2 = y1;
  
  int x3 = ebmpRound( TopLeftX + 0.5*Height );
  int y3 = TopLeftY;
  
  int x4 = x3;
  int y4 = TopLeftY + Height;
  
 
  DrawLine( Image , x1, y1, x2, y2, Color );
  DrawLine( Image , x2, y2, x3, y3, Color );
  DrawLine( Image , x3, y3, x4, y4, Color );
  
  return x1;
 }
 
 if( Letter == '5' )
 {
  int x1 = TopLeftX + Width;
  int y1 = TopLeftY;
  
  int x2;
  int y2 = TopLeftY;
  
  int x3 = TopLeftX + ebmpRound( 0.2*Height )-1;
  int y3 = TopLeftY + ebmpRound( 0.48786796564404*Height );
  
  x2 = x3+1;
  
  double x4 = TopLeftX + Center;
  double y4 = TopLeftY + Height - Center;

  DrawArc( Image , x4, y4, Center, -2.35619449019234 , 3 , Color );
 
  DrawLine( Image , x1, y1, x2, y2, Color );
  DrawLine( Image , x2, y2, x3, y3, Color );
 
  return x1;
 }

 if( Letter == '6' )
 {
  double x1 = TopLeftX + (0.25*Height);
  double y1 = TopLeftY + (0.25*Height);
  
  double x2 = x1;
  double y2 = TopLeftY + (0.75*Height);
  
  int x3 = TopLeftX;
  int y3 = ebmpRound( y1 );
  
  int x4 = x3;
  int y4 = ebmpRound( y2 );

  DrawArc( Image , x1 , y1 , 0.25*Height , 3.1 , 6.2 , Color );
  DrawArc( Image , x2 , y2 , 0.25*Height , 0 , 6.29 , Color );
  
  DrawLine( Image , x3, y3, x4, y4, Color );

  return TopLeftX + (int) ebmpRound(.5*Height); 
 } 
 
 if( Letter == '7' )
 {
  int x1 = TopLeftX;
  int y1 = TopLeftY;
  
  int x2 = TopLeftX + Width;
  int y2 = y1;
  
  int x3 = ebmpRound(TopLeftX + 0.1*Height);
  int y3 = TopLeftY + Height;
 
  DrawLine( Image , x1, y1, x2, y2, Color );
  DrawLine( Image , x2, y2, x3, y3, Color );
  
  return x2; 
 }
 
 if( Letter == '8' )
 {
  double x1 = TopLeftX + (0.25*Height);
  double y1 = TopLeftY + (0.25*Height);
  
  double x2 = x1;
  double y2 = TopLeftY + (0.75*Height);

  DrawArc( Image , x1 , y1 , 0.25*Height , 0 , 6.28318530717959 , Color );
  DrawArc( Image , x2 , y2 , 0.25*Height , 0 , 6.28318530717959 , Color );

  return TopLeftX + (int) ebmpRound(.5*Height); 
 } 

 if( Letter == '9' )
 {
  double x1 = TopLeftX + (0.25*Height);
  double y1 = TopLeftY + (0.25*Height);
  
  double x2 = x1;
  double y2 = TopLeftY + (0.75*Height);
  
  int x3 = ebmpRound( TopLeftX + 0.5*Height );
  int y3 = ebmpRound( y1 );
  
  int x4 = x3;
  int y4 = ebmpRound( y2 );

  DrawArc( Image , x1 , y1 , 0.25*Height , 0 , 6.28318530717959 , Color );
  DrawArc( Image , x2 , y2 , 0.25*Height , 0 , 3 , Color );
  
  DrawLine( Image , x3, y3, x4, y4, Color );

  return TopLeftX + (int) ebmpRound(.5*Height); 
 } 
 
 if( Letter == '.' )
 {
  double x1 = TopLeftX + 1.25;
  double y1 = TopLeftY + Height  - 0.5;
 
  DrawArc( Image, x1, y1, 0.75 , 0 , 6.3 , Color );
  
  return ebmpRound( x1 + 1.25 );
 }
 
 if( Letter == '!' )
 {
  double x1 = TopLeftX + 1.25;
  double y1 = TopLeftY + Height  - 0.5;
  
  int x2 = ebmpRound( x1 );
  int y2 = TopLeftY;
  
  int x3 = x2;
  int y3 = ebmpRound( y1 - 2 );
  
  int y4 = ebmpRound( 0.05*(13*y3+7*y2) );
 
  DrawArc( Image, x1, y1, 0.75 , 0 , 6.3 , Color );
  
  DrawLine( Image, x2,y2, x3,y3 , Color);
  DrawLine( Image, x2-1, y2, x3-1,y4 , Color);
  
  return ebmpRound( x1 + 1.25 );
 } 
 
 if( Letter == ',' )
 {
  double x1 = TopLeftX + 1.25;
  double y1 = TopLeftY + Height  - 0.5;
  
  double x3 = x1;
  double y3 = y1 + 1.75;
 
  DrawArc( Image, x1, y1, 0.75 , 0 , 6.3 , Color );
  DrawArc( Image, x3, y3, 1.75 , -0.5*pi , 0.65*pi , Color );
  
  return ebmpRound( x1 + 1.25 );
 }
 
 if( Letter == '\'' )
 {
  double x1 = TopLeftX + 1.25;
  double y1 = TopLeftY + Center  - 0.5;
  
  double x3 = x1;
  double y3 = y1 + 1.75;
 
  DrawArc( Image, x1, y1, 0.75 , 0 , 6.3 , Color );
  DrawArc( Image, x3, y3, 1.75 , -0.5*pi , 0.65*pi , Color );
  
  return ebmpRound( x1 + 1.25 );
 } 

 if( Letter == '`' )
 {
  double x1 = TopLeftX + 1.25;
  double y1 = TopLeftY + Center  - 0.5;
  
  double x3 = x1;
  double y3 = y1 + 1.75;
 
  DrawArc( Image, x1, y1, 0.75 , 0 , 6.3 , Color );
  DrawArc( Image, x3, y3, 1.75 , 0.35*pi , 1.5*pi , Color );
  
  return ebmpRound( x1 + 1.25 );
 } 
 
 
 if( Letter == '"' )
 {
  double x1 = TopLeftX + 1.25;
  double y1 = TopLeftY + Center  - 0.5;
  
  double x3 = x1;
  double y3 = y1 + 1.75;
 
  DrawArc( Image, x1, y1, 0.75 , 0 , 6.3 , Color );
  DrawArc( Image, x3, y3, 1.75 , -0.5*pi , 0.65*pi , Color );
  
  DrawArc( Image, x1+3.5, y1, 0.75 , 0 , 6.3 , Color );
  DrawArc( Image, x3+3.5, y3, 1.75 , -0.5*pi , 0.65*pi , Color );  
  
  return ebmpRound( x1 + 1.25 + 3.5);
 }  
 
 if( Letter == '[' )
 {
  int x1 = TopLeftX;
  int y1 = TopLeftY;
  
  int x2 = x1;
  int y2 = y1 + Height;
  
  int x3 = ebmpRound(TopLeftX + 0.15*Height)+1;
  int y3 = y1;
  
  int x4 = x3;
  int y4 = y2;
  
  DrawLine(Image,x1,y1,x2,y2,Color);
  DrawLine(Image,x1,y1,x3,y3,Color);
  DrawLine(Image,x2,y2,x4,y4,Color);
  
  return x3;
 }   
 
 if( Letter == ']' )
 {
  int x1 = TopLeftX;
  int y1 = TopLeftY;
  
  int x2 = x1;
  int y2 = y1 + Height;
  
  int x3 = ebmpRound(TopLeftX + 0.15*Height)+1;
  int y3 = y1;
  
  int x4 = x3;
  int y4 = y2;
  
  DrawLine(Image,x3,y3,x4,y4,Color);
  DrawLine(Image,x1,y1,x3,y3,Color);
  DrawLine(Image,x2,y2,x4,y4,Color);
  
  return x3;
 }   
 
 if( Letter == '|' || Letter == 'l' )
 {
  int x1 = TopLeftX+2;
  int y1 = TopLeftY;
  
  int x2 = x1;
  int y2 = y1 + Height;
  
  DrawLine(Image,x1,y1,x2,y2,Color);
  
  return x1+2;
 }    
 
 if( Letter == ':' )
 {
  double x1 = TopLeftX + 1.25;
  double y1 = TopLeftY + Height  - 0.5;
  
  double x2 = x1;
  double y2 = TopLeftY + 0.5*Height;
 
  DrawArc( Image, x1, y1, 0.75 , 0 , 6.3 , Color );
  DrawArc( Image, x2, y2, 0.75 , 0 , 6.3 , Color );
  
  return ebmpRound( x1 + 1.25 );
 } 

 if( Letter == ';' )
 {
  double x1 = TopLeftX + 1.25;
  double y1 = TopLeftY + Height  - 0.5;
  
  double x2 = x1;
  double y2 = TopLeftY + 0.5*Height;
  
  double x3 = x1;
  double y3 = y1 + 1.75;
 
  DrawArc( Image, x1, y1, 0.75 , 0 , 6.3 , Color );
  DrawArc( Image, x2, y2, 0.75 , 0 , 6.3 , Color );
  DrawArc( Image, x3, y3, 1.75 , -0.5*pi , 0.65*pi , Color );
  
  return ebmpRound( x1 + 1.25 );
 } 
 
 if( Letter == '-' )
 {
  int TempWidth = ebmpRound(0.5*Height);
  if( TempWidth % 2 != 0 )
  { TempWidth++; }
  int TempRad = (TempWidth-1)/2;
 
  int x1 = TopLeftX;
  int y1 = TopLeftY + TempWidth;
  
  int x2 = TopLeftX + TempWidth;
  int y2 = y1;
  
  DrawLine( Image, x1, y1, x2, y2 , Color );
  
  return x2;
 } 
 
 if( Letter == '=' )
 {
  int TempWidth = ebmpRound(0.5*Height);
  if( TempWidth % 2 != 0 )
  { TempWidth++; }
  int TempRad = (TempWidth-1)/2;
 
  int x1 = TopLeftX;
  int y1 = TopLeftY + TempWidth-1;
  
  int x2 = TopLeftX + TempWidth+1;
  int y2 = y1;
  
  int x3 = x1;
  int y3 = y1+3;
  
  int x4 = x2;
  int y4 = y3;
  
  
  DrawLine( Image, x1, y1, x2, y2 , Color );
  DrawLine( Image, x3, y3, x4, y4 , Color );
  
  return x2;
 }  
 
 if( Letter == '+' )
 {
  int TempWidth = ebmpRound(0.5*Height);
  if( TempWidth % 2 != 0 )
  { TempWidth++; }
  int TempRad = (TempWidth-1)/2;
 
  int x1 = TopLeftX;
  int y1 = TopLeftY + TempWidth;
  
  int x2 = TopLeftX + TempWidth;
  int y2 = y1;
  
  int x3 = ( x1 + TempRad + 1);
  int y3 = ( y1 + TempRad + 1);

  int x4 = x3;
  int y4 = ( y1 - TempRad - 1);
  
  DrawLine( Image, x1, y1, x2, y2 , Color );
  DrawLine( Image, x3, y3, x4, y4 , Color );
  
  return x2;
 }  
 
 if( Letter == '/' )
 {
  int x1 = TopLeftX;
  int y1 = TopLeftY+Height;
 
  int x2 = TopLeftX+Width;
  int y2 = TopLeftY;
  
  DrawLine(Image,x1,y1,x2,y2,Color);
 
  return x2;
 }
 if( Letter == '\\' )
 {
  int x1 = TopLeftX+Width;
  int y1 = TopLeftY+Height;
 
  int x2 = TopLeftX;
  int y2 = TopLeftY;
  
  DrawLine(Image,x1,y1,x2,y2,Color);
 
  return x1;
 }
 
 if( Letter == '%' )
 {
  int x1 = TopLeftX;
  int y1 = TopLeftY+Height;
 
  int x2 = TopLeftX+Width;
  int y2 = TopLeftY;
  
  double x3 = TopLeftX + 0.15*Height;
  double y3 = TopLeftY + 0.15*Height;
  
  double x4 = ceil( TopLeftX + 0.45*Height + 0.5);
  double y4 = ceil( TopLeftY + 0.85*Height );
  
  DrawLine(Image,x1,y1,x2,y2,Color);

  DrawArc(Image,x3,y3,0.15*Height,0,2*pi,Color);
  DrawArc(Image,x4,y4,0.15*Height,0,2*pi,Color);
  
  return x2;
 } 
 
 if( Letter == '_' )
 {
  DrawLine(Image,TopLeftX,TopLeftY+Height,TopLeftX+Width,TopLeftY+Height,Color);
 
  return TopLeftX + Width;
 }
 
 if( Letter == '^' )
 {
  DrawLine(Image,TopLeftX,TopLeftY+Center,TopLeftX+Center,TopLeftY,Color);
  DrawLine(Image,TopLeftX+Center,TopLeftY,TopLeftX+Width,TopLeftY+Center,Color);
 
  return TopLeftX + Width;
 } 
 
 if( Letter == '<' )
 {
  int x1 = TopLeftX;
  int y1 = ebmpRound( TopLeftY + 0.5*Height );
  
  int x2 = TopLeftX+Width;
  int y2 = TopLeftY;
  
  int x3 = x2;
  int y3 = TopLeftY+Height;
  
  DrawLine(Image,x1,y1,x2,y2,Color);
  DrawLine(Image,x1,y1,x3,y3,Color);
  
  return x2;
 }
 
 if( Letter == '>' )
 {
  int x1 = TopLeftX+Width;
  int y1 = ebmpRound( TopLeftY + 0.5*Height );
  
  int x2 = TopLeftX;
  int y2 = TopLeftY;
  
  int x3 = x2;
  int y3 = TopLeftY+Height;
  
  DrawLine(Image,x1,y1,x2,y2,Color);
  DrawLine(Image,x1,y1,x3,y3,Color);
  
  return x1;
 } 
 
 if( Letter == '#' )
 {
  int TempWidth = ebmpRound(0.5*Height);
  if( TempWidth % 2 != 0 )
  { TempWidth++; }
  int TempRad = (TempWidth-1)/2;
 
  int x1 = TopLeftX;
  int y1 = (int) floor(TopLeftY + 0.5*Height-1);
  
  int x2 = TopLeftX + Width;
  int y2 = y1;
  
  int x3 = x1;
  int y3 = y1+2;
  
  int x4 = x2;
  int y4 = y3;
  
  int x5 = TopLeftX+Center-1;
  int y5 = TopLeftY;

  int x6 = x5;
  int y6 = TopLeftY+Height;
  
  int x7 = TopLeftX+Center+1;
  int y7 = TopLeftY;

  int x8 = x7;
  int y8 = TopLeftY+Height;
  
  DrawLine( Image, x1, y1, x2, y2 , Color );
  DrawLine( Image, x3, y3, x4, y4 , Color );
  DrawLine( Image, x5, y5, x6, y6, Color );
  DrawLine( Image, x7,y7,x8,y8,Color);

  return x2;
 }
 
 if( Letter == '?' )
 {
  double x1 = TopLeftX+Center;
  double y1 = TopLeftY+Center;
  
  int x2 = (int) x1;
  int y2 = TopLeftY + Width;
  
  int x3 = x2;
  int y3 = ebmpRound(TopLeftY+ 0.8*Height);
  if( TopLeftY+Height-y3 <= 2 )
  { y3--; }
  
  double x4 = x1;
  double y4 = TopLeftY + Height  - 0.5;
  
  DrawArc(Image,x1,y1,Center,pi,pi/2,Color);
  DrawLine(Image,x2,y2,x3,y3,Color);
  DrawArc( Image, x4, y4, 0.75 , 0 , 6.3 , Color );
 
  return TopLeftX + Width;
 }
 
 if( Letter == '*' )
 {
  int x1 = TopLeftX+Center;
  int y1 = TopLeftY;
 
  int x2 = x1;
  int y2 = TopLeftY+Height;
  
  int x3 = TopLeftX;
  int y3 = ebmpRound(TopLeftY+0.5*Height);
  
  int x4 = TopLeftX+Width;
  int y4 = y3;
  
  int x5 = TopLeftX+1; // ebmpRound(TopLeftX+0.15*Width);
  int y5 = TopLeftY+1; // ebmpRound(TopLeftY+0.15*Height);
  
  int x6 = TopLeftX+Width-1; // ebmpRound(TopLeftX+0.45*Width);
  int y6 = TopLeftY+Height-1; //  ebmpRound(TopLeftY+0.85*Height);
  
  int x7 = x6;
  int y7 = y5;
  
  int x8 = x5;
  int y8 = y6;
 
  DrawLine(Image,x1,y1,x2,y2,Color);
  DrawLine(Image,x3,y3,x4,y4,Color);
  DrawLine(Image,x5,y5,x6,y6,Color);
  DrawLine(Image,x7,y7,x8,y8,Color);
  
  return x4;
 }
 
 if( Letter == '@' )
 {
  double x1 = TopLeftX + 1.5*Center;
  double y1 = TopLeftY + Height - 1.5*Center;
  
  double x2 = x1 + .35*Center + .6*Center;
  double y2 = y1 + .35*Center;
  
  DrawArc(Image,x1,y1,Center*1.5,0,2*pi,Color);
  DrawArc(Image,x1,y1,Center*0.5,0,2*pi,Color);
  
  DrawArc(Image,x2,y2,Center*0.45,0,pi,Color);
 
  return ebmpRound(TopLeftX + 3*Center);
 }
 
 if( Letter == '~' )
 {
  double x1 = TopLeftX + 0.2*Height;
  double y1 = TopLeftY + 0.4*Height;
  
  double x2 = TopLeftX + 0.6*Height;
  double y2 = y1;
  
  DrawArc(Image,x1,y1,0.2*Height, 0.7*pi,2*pi, Color);
  DrawArc(Image,x2,y2,0.2*Height, -0.3*pi,pi, Color);
 
  return ebmpRound(TopLeftX + 0.8*Height);
 }
 
 if( Letter == '(' ) 
 {
  double x5 = TopLeftX + Center; 
  double y5 = TopLeftY + 0.5*Height; 

  DrawArc( Image, x5, y5 , 0.7*Height , 0.5*pi , -0.5*pi , Color ); 
 
  return TopLeftX;
 }
 
 if( Letter == ')' ) 
 {
  double x5 = TopLeftX - Center; 
  double y5 = TopLeftY + 0.5*Height; 

  DrawArc( Image, x5, y5 , 0.7*Height , -0.5*pi , 0.5*pi , Color ); 
 
  return TopLeftX;
 } 
 
 if( Letter == '&' )
 {
  double x1 = TopLeftX + Center;
  double y1 = TopLeftY + Height-Center;
  
  double x2 = x1;
  double y2 = TopLeftY + 0.2*Height;
  
  int x3 = ebmpRound(TopLeftX+0.5*Height);
  int y3 = ebmpRound(y2);
  
  int x4 = TopLeftX;
  int y4 = ebmpRound(y1);
  
  int x5 = ebmpRound(TopLeftX+0.25*Height);
  int y5 = TopLeftY+Height-2*Center;
  
  int x6 = ebmpRound(TopLeftX+1.1*Width);
  int y6 = TopLeftY+Height;  
  
  
  DrawArc( Image, x1,y1, Center, -.1*pi, 1.5*pi, Color);
  DrawArc( Image, x2,y2, 0.2*Height, 0,2*pi,Color); 
  
//  DrawLine( Image, x3,y3, x4,y4, Color);
  DrawLine( Image, x5,y5, x6,y6, Color);
 
  return x6;
 }

 if( Letter == '&' && 1 == 0 ) // alt ampersand &
 {
  double x1 = TopLeftX + Center;
  double y1 = TopLeftY + Height-Center;
  
  double x2 = x1;
  double y2 = TopLeftY + 0.2*Height;
  
  int x3 = ebmpRound(TopLeftX+0.5*Height);
  int y3 = ebmpRound(y2);
  
  int x4 = TopLeftX;
  int y4 = ebmpRound(y1);
  
  int x5 = ebmpRound(TopLeftX+0.1*Height);
  int y5 = y3;
  
  int x6 = ebmpRound(TopLeftX+1.1*Width);
  int y6 = TopLeftY+Height;  
  
  
  DrawArc( Image, x1,y1, Center, -.25*pi, pi, Color);
  DrawArc( Image, x2,y2, 0.2*Height, pi,2*pi,Color); 
  
  DrawLine( Image, x3,y3, x4,y4, Color);
  DrawLine( Image, x5,y5, x6,y6, Color);
 
  return x6;
 } 
 
 if( Letter == '$' )
 {
  double x1 = TopLeftX + 0.25*Height;
  double y1 = TopLeftY + 0.25*Height;
  
  double x2 = x1;
  double y2 = TopLeftY + 0.75*Height;
  
  int x3 = ebmpRound(x1);
  int y3 = ebmpRound(TopLeftY-0.1*Height);
  
  int x4 = x3;
  int y4 = ebmpRound(TopLeftY+1.1*Height);
  
  DrawArc( Image, x1, y1 , 0.25*Height, 1.5707963267948 , 6.28318530717 , Color );
  DrawArc( Image, x2, y2 , 0.25*Height, -1.5707963267948 ,3.1415926535897 , Color );
  DrawLine( Image, x3, y3, x4, y4, Color);
  
  return ebmpRound( TopLeftX + 0.5*Height );
 }
 
 if( Letter ==  '}' )
 {
  double x1 = TopLeftX;
  double y1 = TopLeftY + 0.15*Height;
  
  double x2 = x1+0.3*Height;
  double y2 = TopLeftY + 0.4*Height;

  double x3 = x2;
  double y3 = TopLeftY + 0.6*Height;
  
  double x4 = x1;
  double y4 = TopLeftY + 0.85*Height;
  
  DrawArc( Image, x1,y1, 0.2*Height, 1.5*pi, 2*pi, Color);
  
  DrawArc( Image, x2,y2, 0.1*Height, 0.5*pi,pi, Color);
  DrawArc( Image, x3,y3, 0.1*Height, pi,1.5*pi, Color);
  
  DrawArc( Image, x4,y4, 0.2*Height, 0,0.5*pi, Color);
  
  int x5 = ebmpRound( TopLeftX+0.2*Height);
  int y5 = ebmpRound( TopLeftY+0.15*Height);
  
  int x6 = x5;
  int y6 = ebmpRound( TopLeftY+0.4*Height);
  
  DrawLine( Image, x5,y5, x6,y6, Color );

  int x7 = x5;
  int y7 = ebmpRound( TopLeftY+0.6*Height);
  
  int x8 = x7;
  int y8 = ebmpRound( TopLeftY+0.85*Height);
  
  DrawLine( Image, x7,y7, x8,y8, Color );  

  return ebmpRound(TopLeftX + 0.4*Height);
 }
 
 if( Letter ==  '{' )
 {
  double x1 = TopLeftX + 0.3*Height;
  double y1 = TopLeftY + 0.15*Height;
  
  double x2 = TopLeftX  ;
  double y2 = TopLeftY + 0.4*Height;

  double x3 = x2;
  double y3 = TopLeftY + 0.6*Height;
  
  double x4 = x1;
  double y4 = TopLeftY + 0.85*Height;
  
  DrawArc( Image, x1,y1, 0.2*Height, pi,1.5*pi, Color);
  
  DrawArc( Image, x2,y2, 0.1*Height, 0,0.5*pi, Color);
  DrawArc( Image, x3,y3, 0.1*Height, 1.5*pi,2*pi, Color);
  
  DrawArc( Image, x4,y4, 0.2*Height, 0.5*pi,pi, Color);
  
  int x5 = ebmpRound( TopLeftX+0.1*Height);
  int y5 = ebmpRound( TopLeftY+0.15*Height);
  
  int x6 = x5;
  int y6 = ebmpRound( TopLeftY+0.4*Height);
  
  DrawLine( Image, x5,y5, x6,y6, Color );

  int x7 = x5;
  int y7 = ebmpRound( TopLeftY+0.6*Height);
  
  int x8 = x7;
  int y8 = ebmpRound( TopLeftY+0.85*Height);
  
  DrawLine( Image, x7,y7, x8,y8, Color );  

  return ebmpRound(TopLeftX + 0.4*Height);
 }  
 
 if( Letter == '&' && 1 == 0 ) // old ampersand '&'
 {
  double x1 = TopLeftX + (0.25*Height);
  double y1 = TopLeftY + (0.25*Height);
  
  double x2 = x1;
  double y2 = TopLeftY + (0.75*Height);
  
  int x3 = ebmpRound( TopLeftX + 0.3*Height);
  int y3 = ebmpRound( TopLeftY + 0.5*Height);
  
  int x4 = ebmpRound( TopLeftX + 0.2*Height);
  int y4 = y3;
  
  int x5 = ebmpRound(TopLeftX + 0.35*Height);
  int y5 = ebmpRound(TopLeftY + 0.75*Height);
  
  int x6 = ebmpRound(TopLeftX + 0.65*Height);
  int y6 = y5;
  
  
  DrawArc( Image , x1 , y1 , 0.25*Height , 1.57079632679490 , 2*pi , Color );
  DrawArc( Image , x2 , y2 , 0.25*Height , 0, 1.5*pi , Color );
  DrawLine( Image , x3, y3, x4, y4, Color );
  DrawLine( Image , x5, y5, x6, y6, Color );

  return x6;
 }   
 
 
 return TopLeftX;
}

int PrintCopyright( BMP& Image, int TopLeftX, int TopLeftY , int Height , 
                  RGBApixel Color )
{
 double pi = 3.14159265358979;
 
 int CharNumber = 0;
 int StartX = ebmpRound(TopLeftX+0.25*Height);
 int Spacing = (int) ebmpRound( 0.2*Height );
 if( Spacing < 3 )
 { Spacing = 3; }
 int StartY = ebmpRound( TopLeftY-0.25*Height);
 
 double x1 = TopLeftX + 0.5*Height;
 double y1 = TopLeftY + 0.5*Height;
 
 DrawArc(Image, x1, y1, 0.6*Height , 0, 2*pi , Color );
 return PrintLetter( Image, 'c' , StartX, StartY , Height, Color ) + Spacing; 
}
