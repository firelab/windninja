/******************************************************************************
 *
 * $Id$
 *
 * Project:  WindNinja
 * Purpose:  Basic surface storage grid for spatial values
 * Author:   Kyle Shannon <ksshannon@gmail.com>
 *
 ******************************************************************************
 *
 * THIS SOFTWARE WAS DEVELOPED AT THE ROCKY MOUNTAIN RESEARCH STATION (RMRS)
 * MISSOULA FIRE SCIENCES LABORATORY BY EMPLOYEES OF THE FEDERAL GOVERNMENT
 * IN THE COURSE OF THEIR OFFICIAL DUTIES. PURSUANT TO TITLE 17 SECTION 105
 * OF THE UNITED STATES CODE, THIS SOFTWARE IS NOT SUBJECT TO COPYRIGHT
 * PROTECTION AND IS IN THE PUBLIC DOMAIN. RMRS MISSOULA FIRE SCIENCES
 * LABORATORY ASSUMES NO RESPONSIBILITY WHATSOEVER FOR ITS USE BY OTHER
 * PARTIES,  AND MAKES NO GUARANTEES, EXPRESSED OR IMPLIED, ABOUT ITS QUALITY,
 * RELIABILITY, OR ANY OTHER CHARACTERISTIC.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 *****************************************************************************/

#ifndef ASCII_GRID_H
#define ASCII_GRID_H

//#define ASCII_GRID_DEBUG

#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <sstream>
#include <cmath>
#include <stdio.h>
#include <stdlib.h>
#include <limits>
#include <algorithm>
#include <regex>
#include "ninjaException.h"
#include "ninjaMathUtility.h"

#include "gdal.h"
#include "gdal_priv.h"
#include "cpl_port.h"
#include "Array2D.h"

#include "ogr_spatialref.h" //nsw
#include "gdal_version.h" //nsw
#include "ogr_core.h" //nsw
#include "gdalwarper.h" //nsw
#include "gdal_priv.h" //nsw
#include "EasyBMP.h"  //nsw
#include "EasyBMP_Font.h"  //nsw
#include "EasyBMP_DataStructures.h"  //nsw
#include "EasyBMP_Geometry.h"  //nsw


/**
 * Class that stores 2-dimensional arrays of data for mostly spatial
 * use.  The data are stored in [row][column] order.  The header data
 * is read in and stored as variables for the class and the data are
 * read in and stored in a two dimensional array based on the TNT
 * (Templated Numerical Toolkit) from NIST. See tnt.h and other files
 * for more details.
 */
template <class T>
class AsciiGrid
{
    template <class T1>
    friend std::ostream &operator<<(std::ostream &out, AsciiGrid<T> &A);

    template <class T2>
    friend std::istream &operator>>(std::istream &in, AsciiGrid<T> &A);


public:
    AsciiGrid();
    AsciiGrid(const std::string fileName);
    AsciiGrid(const AsciiGrid &A);
    AsciiGrid(int nC, int nR, double xL, double yL, double cS, double nDV, std::string prjStr = "");
    AsciiGrid(int nC, int nR, double xL, double yL, double cS, double nDV, T a, std::string prjStr = "");
    AsciiGrid(GDALDataset* pSrcDS, int band);

    virtual ~AsciiGrid();
    void deallocate();

    enum interpTypeEnum  /*!< Interpolation order, 0->nearest neighbor */
    {
        order0,
        order1,
        order2,
        order3,
    };

    enum tiffType
    {
        tiffRgb,
        tiffGray
    };

    typedef T valueType;

    void read_Grid(const std::string inputFile);
    void GDALReadGrid(const std::string inputFile, int band = 1);
    void initialize_Grid(T value);
    void set_cellValue(int m, int n, T value);
    void set_headerData(AsciiGrid &A);
    void set_headerData(int nC, int nR, double xL, double yL, double cS,
                        double nDV, T a, std::string prjStr = "");

    inline bool set_xllCorner(double x) {xllCorner = x; return true;}
    inline bool set_yllCorner(double y) {yllCorner = y; return true;}
    bool set_noDataValue(double nDV);
    inline bool set_cellSize(double cS) {cellSize = cS; return true;}
    inline bool set_prjString(std::string prjStr) {prjString = prjStr; return true;}

    inline int get_nRows() const {return data.get_numRows();}
    inline int get_nCols() const {return data.get_numCols();}
    void set_nRows(int m) { data.set_numRows(m); }
    void set_nCols(int n) { data.set_numCols(n); }
    inline long get_arraySize() const {return data.get_numRows() * data.get_numCols();}
    inline double get_xllCorner() const {return xllCorner;}
    inline double get_yllCorner() const {return yllCorner;}
    inline double get_noDataValue() const {return data.getNoDataValue();}
    inline double get_cellSize() const {return cellSize;}
    inline T get_cellValue(int m, int n) const {return data(m,n);}
    inline double get_NoDataValue() const {return data.getNoDataValue();}
    inline bool get_hasNoDataValues() const {return data.hasNoDataValues();}
    void get_cellPosition(int i, int j, double *xCoord, double *yCoord) const;
    void get_cellPositionLocalCoordinates(int i, int j, double *xCoord,
                                          double *yCoord) const;
    void get_cellIndex(double xCoord, double yCoord, int *i, int *j) const;
    void get_cellIndexLocalCoordinates(double xCoord, double yCoord, int *i,
                                       int *j) const;
    void get_nearestCellIndex(double xCoord, double yCoord, int *i, int *j) const;
    double get_xDimension() const {return cellSize * data.get_numCols();};
    double get_yDimension() const {return cellSize * data.get_numRows();};

    void get_gridCenter(double *x, double *y);

    T get_maxValue();
    T get_minValue();

    double get_meanValue() const;
    bool fillNoDataValues( int minNeighborCells, double maxPercentNoData, int maxNumPasses );

    bool find_firstValue(T m, T buffer, int *k, int *l);

    bool checkForNoDataValues();

    bool checkForCoincidentGrids(AsciiGrid &A);

    bool check_inBounds(int m,int n) const;
    bool check_inBounds(double X, double Y) const;

    AsciiGrid<T> resample_Grid(double resampleCellSize,
                               interpTypeEnum interpType);
    void resample_Grid_in_place(double resampleCellSize,
                                interpTypeEnum interpType);
    void resample_Grid_in_place(int arraySize, interpTypeEnum interpType);

    void interpolateFromGrid(AsciiGrid &A, interpTypeEnum interpType);

    void interpolateFromPoints(T* pointData, double* X, double* Y,
                               double* influenceRadius, int numPoints,
                               double interpDistPower);

    void clipGridInPlaceSnapToCells(double percentClip);

    AsciiGrid<T> normalize_Grid(T lowBound, T highBound);

    T interpolateGrid(double x, double y, interpTypeEnum interpType) const;
    T interpolateGridLocalCoordinates(double x, double y,
                                      interpTypeEnum interpType) const;

    void replaceValue( T initial, T final );
    void replaceNan( T final );

    void print_GridInfo() const ;
    void print_Grid() const;

    void write_GridInfo(FILE *fileOut);
    void write_integralGridData(FILE *fileOut);
    void write_fpGridData(FILE *fileOut, int numDecimals);
    void write_Grid(std::string outputFile, int numDecimals);
    void write_json_Grid(std::string outputFile, int numDecimals);

    void write_ascii_4326_Grid (std::string outputFile, int numDecimals);
    void write_json_4326_Grid(std::string outputFile, int numDecimals);

    bool crop_noData(int noDataThreshold);

    void sort_grid();

    void divide_gridData(double *d, int splits);

    GDALDatasetH ascii2GDAL();

    void ascii2png(std::string outFilename,
                   std::string legendTitle,
                   std::string legendUnits,
                   std::string scalarLegendFilename,
                   bool writeLegend);

    void exportToTiff( std::string outFilename, tiffType type = tiffGray );

    AsciiGrid<T> BufferGrid( int nAddCols=1, int nAddRows=1 );
    void BufferGridInPlace( int nAddCols=1, int nAddRows=1 );
    AsciiGrid<T> BufferAroundGrid( int nAddCols=1, int nAddRows=1 );
    void BufferAroundGridInPlace( int nAddCols=1, int nAddRows=1 );
    void BufferToOverlapGrid(AsciiGrid &A);
    bool CheckForGridOverlap(AsciiGrid &A);

    /* @todo
     * Ideally this would not be a public variable that other classes have access to, instead
     * they should be given methods to access the 2 D Array contained within each Ascii_Grid,
     * due to the large number of classes accessing the data member this will need to be overhauled
     * at a later date.
     * */
    Array2D<T> data;

    AsciiGrid<T> &operator=(const AsciiGrid &A);

    bool operator=(T m);

    bool operator==(AsciiGrid &A);
    bool operator!=(AsciiGrid &A);
    T& operator()(unsigned m, unsigned n);
    const T& operator() (const unsigned m, const unsigned n) const;

    AsciiGrid<T> operator+(T m);
    AsciiGrid<T> operator-(T m);
    AsciiGrid<T> operator/(T m);
    AsciiGrid<T> operator*(T m);

    bool operator+=(T m);
    bool operator-=(T m);
    bool operator/=(T m);
    bool operator*=(T m);

    AsciiGrid<T> operator+(AsciiGrid<T> &A);
    AsciiGrid<T> operator-(AsciiGrid<T> &A);
    AsciiGrid<T> operator/(AsciiGrid<T> &A);
    AsciiGrid<T> operator*(AsciiGrid<T> &A);

    bool operator+=(AsciiGrid<T> &A);
    bool operator-=(AsciiGrid<T> &A);
    bool operator/=(AsciiGrid<T> &A);
    bool operator*=(AsciiGrid<T> &A);

    double cellSize;
    double xllCorner;
    double yllCorner;

    std::string prjString;


private:
    T *sortedData;
    //do not change!!!!!--This means you Jason!!!!!!!
    //consider this deprecated, but we *might* have written a binary grid
    //at some point.  *sigh* leave it in.  My bad -kss.
    static const int magicNumber = 0x3634af3;

    void write_4326_Grid (std::string& filename, int precision, void (AsciiGrid<T>::*write_grid)(std::string,int));
};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#endif  //ASCII_GRID_H
