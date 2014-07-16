#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <string.h>
#include <math.h>
#include "gdal_priv.h"
#include "ninja_conv.h"
#include "ninja_init.h"
#include "cpl_string.h"

#include "stl_create.h"


using namespace std;

struct Position
{
    float x;
    float y;
    float z;
};

Position computeNorm( const Position& v1, const Position& v2 )
{
     float norm_factor = 0;
     Position norm;

     norm.x = ( v1.y * v2.z ) - ( v1.z * v2.y );
     norm.y = ( v2.x * v1.z ) - ( v2.z * v1.x );
     norm.z = ( v1.x * v2.y ) - ( v1.y * v2.x );

     norm_factor = sqrt( norm.x*norm.x + norm.y*norm.y + norm.z*norm.z );

     norm.x /= norm_factor;
     norm.y /= norm_factor;
     norm.z /= norm_factor;

     return norm;
}


void writeStlFromGdal(  float ** data, const unsigned int nrows,
                const unsigned int ncols,
                const float xllcorner, const float yllcorner,
                const float dx, const float dy,
                const string output )
{
    ofstream out_file;

    out_file.open( output.c_str(), ios::out );
    out_file << "solid NAME" << endl;
    /* Iterate through (row,col) pairs and create all vertices */

    float xoffset = dx * 0.5;
    float yoffset = dy * 0.5;

    Position a, b, c, d;
    Position v1, v2, norm;
    for( unsigned int row = 0; row < nrows - 1; row++ )
    {
        for( unsigned int col = 0; col < ncols - 1;  col++ )
        {

            a.x = xllcorner + col * dx + xoffset;
            a.y = yllcorner + row * dy + yoffset;
            a.z = data[row][col];

            b.x = xllcorner + ( col + 1 ) * dx + xoffset;
            b.y = a.y;
            b.z = data[row][col + 1];

            c.x = a.x;
            c.y = yllcorner + ( row + 1 ) * dy + yoffset;
            c.z = data[row + 1][col];

            d.x = b.x;
            d.y = c.y;
            d.z = data[row + 1][col + 1];

            /*  compute normal for first triangle */
            v1.x = c.x - a.x;
            v1.y = c.y - a.y;
            v1.z = c.z - a.z;

            v2.x = b.x - a.x;
            v2.y = b.y - a.y;
            v2.z = b.z - a.z;

            norm = computeNorm( v1, v2 );

            /*  write the output  */
            out_file << "facet normal " << norm.x << " " << norm.y << " "
                     << norm.z << endl;
            out_file << "\touter loop" << endl;
            out_file << "\t\tvertex " << a.x << " " << a.y << " "
                     << a.z << " " << endl;
            out_file << "\t\tvertex " << b.x << " " << b.y << " "
                     << b.z << " " << endl;
            out_file << "\t\tvertex " << c.x << " " << c.y << " "
                     << c.z << " " << endl;
            out_file << "\tendloop" << endl;
            out_file << "endfacet" << endl;

            /*  compute normal for second triangle */
            v1.x = b.x - d.x;
            v1.y = b.y - d.y;
            v1.z = b.z - d.z;

            v2.x = c.x - d.x;
            v2.y = c.y - d.y;
            v2.z = c.z - d.z;

            norm = computeNorm( v1, v2 );

            /*  write the output  */
            out_file << "facet normal " << norm.x << " " << norm.y << " "
                     << norm.z << endl;
            out_file << "\touter loop" << endl;
            out_file << "\t\tvertex " << d.x << " " << d.y << " "
                     << d.z << " " << endl;
            out_file << "\t\tvertex " << b.x << " " << b.y << " "
                     << b.z << " " << endl;
            out_file << "\t\tvertex " << c.x << " " << c.y << " "
                     << c.z << " " << endl;
            out_file << "\tendloop" << endl;
            out_file << "endfacet" << endl;
        }
    }
    out_file.close();
}

void writeBinaryStlFromGdal(  float ** data, const unsigned int nrows,
                const unsigned int ncols,
                const float xllcorner, const float yllcorner,
                const float dx, const float dy,
                const string output )
{
    ofstream out_file;

    out_file.open( output.c_str(), ios::out | ios::binary );

    char header[80];
    for( unsigned short i = 0; i < 80; i++ )
    {
        header[i] = '\0';
    }
    out_file.write( header, 80 );
    unsigned int ntriangles = nrows * ncols * 2;
    out_file.write( (char*) &ntriangles, sizeof( unsigned int ) );
    unsigned short nattributes = 0;

    /* Iterate through (row,col) pairs and create all vertices */

    float xoffset = dx * 0.5;
    float yoffset = dy * 0.5;

    Position a, b, c, d;
    Position v1, v2, norm;
    for( unsigned int row = 0; row < nrows - 1; row++ )
    {
        for( unsigned int col = 0; col < ncols - 1;  col++ )
        {

            a.x = xllcorner + col * dx + xoffset;
            a.y = yllcorner + row * dy + yoffset;
            a.z = data[row][col];

            b.x = xllcorner + ( col + 1 ) * dx + xoffset;
            b.y = a.y;
            b.z = data[row][col + 1];

            c.x = a.x;
            c.y = yllcorner + ( row + 1 ) * dy + yoffset;
            c.z = data[row + 1][col];

            d.x = b.x;
            d.y = c.y;
            d.z = data[row + 1][col + 1];

            /*  compute normal for first triangle */
            v1.x = c.x - a.x;
            v1.y = c.y - a.y;
            v1.z = c.z - a.z;

            v2.x = b.x - a.x;
            v2.y = b.y - a.y;
            v2.z = b.z - a.z;

            norm = computeNorm( v1, v2 );

            /*  write the output  */
            out_file.write( (char*) &norm.x, sizeof(float) );
            out_file.write( (char*) &norm.y, sizeof(float) );
            out_file.write( (char*) &norm.z, sizeof(float) );

            out_file.write( (char*) &a.x, sizeof(float) );
            out_file.write( (char*) &a.y, sizeof(float) );
            out_file.write( (char*) &a.z, sizeof(float) );

            out_file.write( (char*) &b.x, sizeof(float) );
            out_file.write( (char*) &b.y, sizeof(float) );
            out_file.write( (char*) &b.z, sizeof(float) );

            out_file.write( (char*) &c.x, sizeof(float) );
            out_file.write( (char*) &c.y, sizeof(float) );
            out_file.write( (char*) &c.z, sizeof(float) );
            out_file.write( (char*) &nattributes, sizeof(unsigned short) );


            /*  compute normal for second triangle */
            v1.x = b.x - d.x;
            v1.y = b.y - d.y;
            v1.z = b.z - d.z;

            v2.x = c.x - d.x;
            v2.y = c.y - d.y;
            v2.z = c.z - d.z;

            norm = computeNorm( v1, v2 );

            /*  write the output  */
            out_file.write( (char*) &norm.x, sizeof(float) );
            out_file.write( (char*) &norm.y, sizeof(float) );
            out_file.write( (char*) &norm.z, sizeof(float) );

            out_file.write( (char*) &d.x, sizeof(float) );
            out_file.write( (char*) &d.y, sizeof(float) );
            out_file.write( (char*) &d.z, sizeof(float) );

            out_file.write( (char*) &b.x, sizeof(float) );
            out_file.write( (char*) &b.y, sizeof(float) );
            out_file.write( (char*) &b.z, sizeof(float) );

            out_file.write( (char*) &c.x, sizeof(float) );
            out_file.write( (char*) &c.y, sizeof(float) );
            out_file.write( (char*) &c.z, sizeof(float) );

            out_file.write( (char*) &nattributes, sizeof(unsigned short) );
        }
    }
    out_file.close();
}


void fromGdalToStl( const string input, const string output )
{
    string line = "";

    float zvalue            = 0.0;
    unsigned int ncols      = 0;
    unsigned int nrows      = 0;
    float xllcorner         = 0.0;
    float yllcorner         = 0.0;
    float nodata_value      = 0.0;

    GDALDataset *poDataset;

    GDALAllRegister();

    poDataset = ( GDALDataset * ) GDALOpen( input.c_str(), GA_ReadOnly );
    if( NULL == poDataset )
    {
        exit(1);
    }

    double adfGeoTransform[ 6 ];
    poDataset->GetGeoTransform( adfGeoTransform );

    ncols = poDataset->GetRasterXSize();
    nrows = poDataset->GetRasterYSize();


    float ** data;
    data = new float*[nrows];

    GDALRasterBand * poBand;
    poBand = poDataset->GetRasterBand( 1 );

    float * padfScanline;
    padfScanline = new float[ ncols ];

    for (int i = 0; i < nrows; ++i)
    {
        data[i] = new float[ncols];
    }
    int current_row = 0, current_col = 0;
    while( current_row < nrows )
    {
        current_col = 0;
        poBand->RasterIO( GF_Read, 0, current_row, ncols, 1, padfScanline, ncols,
                          1, GDT_Float32, 0, 0 );
        while(current_col < ncols )
        {
            data[current_row][current_col] =  padfScanline[ current_col ];
            ++current_col;
        }
        ++current_row;
    }
    delete [] padfScanline;


    writeBinaryStlFromGdal(
                data,
                poDataset->GetRasterYSize(),
                poDataset->GetRasterXSize(),
                adfGeoTransform[0], adfGeoTransform[3],
                adfGeoTransform[1], adfGeoTransform[5],
                output );
    GDALClose( poDataset );

    for( int i = 0; i < nrows; i++ )
    {
        delete [] data[i];
    }
    delete [] data;

    return;
}

void fromAscToStl( const string input, const string output )
{
    ifstream infile;
    infile.open( input.c_str() );
    string line = "";

    float zvalue            = 0.0;
    unsigned int ncols      = 0;
    unsigned int nrows      = 0;
    float xllcorner         = 0.0;
    float yllcorner         = 0.0;
    float cellsize          = 0.0;
    float nodata_value      = 0.0;

    //------------read in header data and store values----------
    infile >> line >> ncols;
    infile >> line >> nrows;
    infile >> line >> xllcorner;
    infile >> line >> yllcorner;
    infile >> line >> cellsize;
    infile >> line >> nodata_value;
    /*
    cout << "ncols = " << ncols << endl
         << "nrows = " << nrows << endl
         << "xllcorner = " << xllcorner << endl
         << "yllcorner = " << yllcorner << endl
         << "cellsize = " << cellsize << endl
         << "no data value = " << nodata_value << endl;
    */
    float offset = cellsize*0.5;

   //-----------read elevation data into an array------------
    float **data;
    data = new float*[nrows];
    for (int i = 0; i < nrows; ++i)
    {
        data[i] = new float[ncols];
    }
    int current_row = 0;
    while (current_row <= (nrows-1))
    {
        int current_col = 0;
        while (current_col <= (ncols-1))
        {
            infile >> zvalue;
            data[current_row][current_col] =  zvalue;
            ++current_col;
        }
        ++current_row;
    }
    infile.close();

    ofstream out_file;
    out_file.open( output.c_str(), ios::out );
    out_file << "solid NAME" << endl;
    /* Iterate through (row,col) pairs and create all vertices */

    Position a, b, c, d;
    Position v1, v2, norm;
    for( unsigned int row = 0; row < nrows - 1; row++ )
    {
        for( unsigned int col = 0; col < ncols - 1;  col++ )
        {

            a.x = xllcorner + col * cellsize + offset;
            a.y = yllcorner + ( nrows - row ) * cellsize - offset;
            a.z = data[row][col];

            b.x = xllcorner + ( col + 1 ) * cellsize + offset;
            b.y = a.y;
            b.z = data[row][col + 1];

            c.x = a.x;
            c.y = yllcorner + ( nrows - row - 1 ) * cellsize - offset;
            c.z = data[row + 1][col];

            d.x = b.x;
            d.y = c.y;
            d.z = data[row + 1][col + 1];

            /*  compute normal for first triangle */
            v1.x = c.x - a.x;
            v1.y = c.y - a.y;
            v1.z = c.z - a.z;

            v2.x = b.x - a.x;
            v2.y = b.y - a.y;
            v2.z = b.z - a.z;

            norm = computeNorm( v1, v2 );

            /*  write the output  */
            out_file << "facet normal " << norm.x << " " << norm.y << " "
                     << norm.z << endl;
            out_file << "\touter loop" << endl;
            out_file << "\t\tvertex " << a.x << " " << a.y << " "
                     << a.z << " " << endl;
            out_file << "\t\tvertex " << b.x << " " << b.y << " "
                     << b.z << " " << endl;
            out_file << "\t\tvertex " << c.x << " " << c.y << " "
                     << c.z << " " << endl;
            out_file << "\tendloop" << endl;
            out_file << "endfacet" << endl;

            /*  compute normal for second triangle */
            v1.x = b.x - d.x;
            v1.y = b.y - d.y;
            v1.z = b.z - d.z;

            v2.x = c.x - d.x;
            v2.y = c.y - d.y;
            v2.z = c.z - d.z;

            norm = computeNorm( v1, v2 );

            /*  write the output  */
            out_file << "facet normal " << norm.x << " " << norm.y << " "
                     << norm.z << endl;
            out_file << "\touter loop" << endl;
            out_file << "\t\tvertex " << d.x << " " << d.y << " "
                     << d.z << " " << endl;
            out_file << "\t\tvertex " << b.x << " " << b.y << " "
                     << b.z << " " << endl;
            out_file << "\t\tvertex " << c.x << " " << c.y << " "
                     << c.z << " " << endl;
            out_file << "\tendloop" << endl;
            out_file << "endfacet" << endl;
        }
    }
    out_file.close();

    /* free all dynamically allocated data */
    for (int i=0; i<nrows; ++i)
    {  //delete data row by row
        delete[] data[i];
    }
    delete[] data;


}

int main( int argc, char* argv[] )
{
    NinjaInitialize();
    /*  parse input arguments  */
    if( argc != 3 )
    {
        cout << "Invalid arguments!" << endl;
        cout << "converter [input filename] [output filename]" << endl;
        return 1;
    }
    string input_file = string( argv[1] );
    string output_file = string( argv[2] );

    //Take a GDAL compatible file and convert to binary STL with name 'output_file'
#ifdef USE_OLD_STL_CREATE
    fromGdalToStl( input_file, output_file );
    return 0;
#else
    return NinjaElevationToStl( argv[1], argv[2], 1, NinjaStlBinary, GDALTermProgress );
#endif /* USE_OLD_STL_CREATE */
}

