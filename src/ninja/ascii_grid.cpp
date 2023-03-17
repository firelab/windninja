#include "ascii_grid.h"


inline void check(CPLErr res) {
    if (res != CE_None) {
        CPLErrorNum err = CPLGetLastErrorNo();
        CPLError(res, err, "GDAL error: %d", err);
    }
}

template <class T> GDALDataType getGdalDataType() { return GDT_Unknown; }
template<> GDALDataType getGdalDataType<double>() { return GDT_Float64; }
template<> GDALDataType getGdalDataType<short>() { return GDT_Int16; }
template<> GDALDataType getGdalDataType<int>() { return GDT_Int32; }
// ...and more specializations to follow
// make the default throw so that we don't try to scan into a wrong type size 
// note - older MSVCs seem to have problems with regex_replace(const char* fmt..) hene we have to pass in temporary strings 
template <class T> string dataFormat(const char* fmt) { throw std::runtime_error("unknown data format specifier"); } 
template<> string dataFormat<double>(const char* fmt) { return std::regex_replace(std::string(fmt), regex("<T>"), std::string("lf")); } 
template<> string dataFormat<int>(const char* fmt) { return std::regex_replace(std::string(fmt), regex("<T>"), std::string("d")); } 
template<> string dataFormat<short>(const char* fmt) { return std::regex_replace(std::string(fmt), regex("<T>"), std::string("hd")); } 

template <class T> inline T epsClr() { throw std::runtime_error("unknown eps value"); }
template<> inline double epsClr<double>() { return 0.001; }
template<> inline int epsClr<int>() { return 1; }
template<> inline short epsClr<short>() { return 1; }

template <class T>
inline bool cplIsNan(T t) {
    return CPLIsNan((double)t); // we need to disambiguate
}


/**
 * @brief Create an empty grid
 * Create an empty grid with no header data and no data
 */
template <class T>
AsciiGrid<T>::AsciiGrid()
{
    cellSize = 0;
    xllCorner = 0;
    yllCorner = 0;
    sortedData = 0;

    #ifdef ASCII_GRID_DEBUG
        std::cout << "Created AsciiGrid using AsciiGrid()" << std::endl;
    #endif

}

/**
 * @brief Construct a grid from file
 * Construct a grid from a file of type AAIGrid.  Set the header data and
 * fill the grid from file.
 *
 * @param fileName file to read
 */
template <class T>
AsciiGrid<T>::AsciiGrid(const std::string fileName)
{
    prjString = "";
    sortedData = 0;

    read_Grid(fileName);

}

/**
 * @brief Construct a grid from another grid
 * Construct a grid using another grid as a copy.  All header data and data
 * are copied.
 *
 * @param A reference to the grid to be copied
 */
template <class T>
AsciiGrid<T>::AsciiGrid(const AsciiGrid &A)
{
    sortedData = 0;
    data = A.data;
    cellSize = A.cellSize;
    xllCorner = A.xllCorner;
    yllCorner = A.yllCorner;
    prjString = A.prjString;

    if(A.sortedData != NULL)
    {
        sortedData = new T[data.get_numRows() * data.get_numCols()];
        for(int i = 0; i < data.get_numRows() * data.get_numCols(); i++)
            sortedData[i] = A.sortedData[i];
    }

}

/**
 * @brief Construct an empty grid using header data.
 * Construct a grid by specifying the corner and cellsize, along with other
 * data.
 *
 * @param nC number of columns
 * @param nR number of rows
 * @param xL x coordinate of the lower left corner
 * @param yL y coordinate of the lower left corner
 * @param cS cell size, assumed square cells
 * @param nDV no data value to fill the grid with
 * @param prjStr projection reference, assumed to be wkt format
 */
template <class T>
AsciiGrid<T>::AsciiGrid(int nC, int nR, double xL, double yL, double cS, double nDV, std::string prjStr)
:data(nR, nC, nDV)
{
    cellSize = 0;
    xllCorner = 0;
    yllCorner = 0;
    prjString = prjStr;
    sortedData = 0;
    cellSize = cS;
    xllCorner = xL;
    yllCorner = yL;
}

/**
 * @brief Construct a grid with header data and a default value.
 * Construct a grid with header data specified and fill the grid with some
 * default value.
 *
 * @param nC number of columns
 * @param nR number of rows
 * @param xL x coordinate of the lower left corner
 * @param yL y coordinate of the lower left corner
 * @param cS cell size, assumed square cells
 * @param nDV no data value
 * @param a default value to fill the grid
 * @param prjStr projection reference, assumed to be wkt format
 */
template <class T>
AsciiGrid<T>::AsciiGrid(int nC, int nR, double xL, double yL, double cS, double nDV, T a, std::string prjStr)
:data(nR, nC, nDV)
{
    prjString = prjStr;
    sortedData = 0;
    cellSize = cS;
    xllCorner = xL;
    yllCorner = yL;
    data = a;
}

template <class T>
AsciiGrid<T>::AsciiGrid(GDALDataset *poDS, int band) 
:data(poDS->GetRasterYSize(), poDS->GetRasterXSize(), poDS->GetRasterBand(band)->GetNoDataValue())
{
    sortedData = nullptr;

    int nXSize = poDS->GetRasterXSize();
    int nYSize = poDS->GetRasterYSize();

    double adfGeoTransform[6];
    poDS->GetGeoTransform( adfGeoTransform );

    //find llcorner
    double adfCorner[2];
    adfCorner[0] = adfGeoTransform[0];
    adfCorner[1] = adfGeoTransform[3] + ( nYSize * adfGeoTransform[5] );

    //check for non-square cells
    /*
    if( abs( adfGeoTransform[1] ) - abs( adfGeoTransform[5] ) > 0.0001 )
	CPLDebug( "GDAL2AsciiGrid", "Non-uniform cells" );
    */

    GDALRasterBand *poBand;
    poBand = poDS->GetRasterBand( band );

    //not used yet
    //GDALDataType eDataType = poBand->GetRasterDataType();

    int pbSuccess = 0;
    double dfNoData = poBand->GetNoDataValue( &pbSuccess );
    if( pbSuccess == false )
	dfNoData = -9999.0;

    //reallocate all memory and set header info
    set_headerData( nXSize, nYSize, adfCorner[0], adfCorner[1], adfGeoTransform[1], dfNoData, dfNoData);

    //set the data
    double *padfScanline;
    padfScanline = new double[nXSize];
    GDALDataType gdt = getGdalDataType<T>();

    for(int i = nYSize - 1;i >= 0;i--) {
        check(poBand->RasterIO(GF_Read, 0, i, nXSize, 1, padfScanline, nXSize, 1, gdt, 0, 0));

        for(int j = 0;j < nXSize;j++) {
            set_cellValue(nYSize - 1 - i, j, padfScanline[j]);
        }
    }

    delete [] padfScanline;

    //try to get the projection info
    std::string prj = poDS->GetProjectionRef();
    set_prjString( prj );
}


/**
 * @brief Destroy a grid
 * Delete a grid and destroy all allocated memory stored in the grid
 */
template <class T>
AsciiGrid<T>::~AsciiGrid()
{
    deallocate();
}

/**
 * @brief Deallocate the memory allocated for the grid
 * Deallocate all memory dynamically allocated for the grid and reset the
 * header data.
 */

template <class T>
void AsciiGrid<T>::deallocate()
{
    if(sortedData)
    {
        delete[]sortedData;
        sortedData = NULL;
    }

    cellSize = 0.0;
    xllCorner = 0.0;
    yllCorner = 0.0;
}

/**
 * @brief Set the contents of the grid to a uniform value.
 *
 * @param value uniform value for the grid
 */
template <class T>
void AsciiGrid<T>::initialize_Grid(T value)
{
    data = value;
}

/**
 * @brief Set a cell value for a given index
 *
 * @param m row for insertion
 * @param n column for insertion
 * @param value value to insert
 */
template <class T>
void AsciiGrid<T>::set_cellValue(int m, int n, T value)
{
    if(m < 0 || m > data.get_numRows() - 1 || n < 0 || n > data.get_numCols() - 1)
        throw std::range_error("Invalid cell reference in AsciiGrid<T>::set_cellValue().");
    data(m,n) = value;
}

/**
 * @brief Reset an existing grid and delete the data.
 * Reset the grid using header data from another grid.
 *
 * @param A grid to fetch header data from
 */
template <class T>
void AsciiGrid<T>::set_headerData(AsciiGrid &A)
{
    data.set_numCols(A.get_nCols());
    data.set_numRows(A.get_nRows());
    xllCorner = A.get_xllCorner();
    yllCorner = A.get_yllCorner();
    cellSize = A.get_cellSize();
    prjString = A.prjString;

    data = Array2D<T>(A.get_nRows(), A.get_nCols(), A.get_noDataValue());
}

/**
 * @brief Reset an existing grid and initilize it using header data
 *
 * @param nC number of columns
 * @param nR number of rows
 * @param xL x coordinate of the lower left corner
 * @param yL y coordinate of the lower left corner
 * @param cS cell size, assumed square cells
 * @param nDV no data value
 * @param a default value to fill the grid
 * @param prjStr projection reference, assumed to be wkt format
 */
template <class T>
void AsciiGrid<T>::set_headerData(int nC, int nR, double xL, double yL, double cS, double nDV, T a, std::string prjStr)
{
    data.setMatrix(nR, nC, nDV, a);
    xllCorner = xL;
    yllCorner = yL;
    cellSize = cS;
    prjString = prjStr;
}

/**
 * @brief Set the no data value for a grid
 *
 * @note These are set back to no data because they could have been originally
 * set to the no data value, but changed here.  This will mess up the min/max
 * value functions because they first do a check to see if the min/max value
 * has been already computed (no computed if set equal to the no data value)
 *
 * @param nDV new no data value
 */
template <class T>
bool AsciiGrid<T>::set_noDataValue(double nDV)
{
    data.setNoDataValue(nDV);
    return true;
}


/**
 * @brief Read the contents of a file into a grid
 * Read a file and populate a grid.  The file MUST be an AAIGrid file.
 *
 * @param inputFile file to read into the grid
*/
template <class T>
void AsciiGrid<T>::read_Grid(const std::string inputFile)
{
    char *krap;
    krap = new char[5000];
    std::string kkrap;
    FILE *fin;

    if((fin = fopen(inputFile.c_str(), "r")) == NULL)
        throw std::runtime_error("No input file found in AsciiGrid<T>::read_Grid().");

    fscanf(fin, "%s", krap);
    kkrap.assign(krap);
    if((kkrap!="ncols") && (kkrap!="NCOLS"))
        throw std::runtime_error("File does not appear to be in the correct format in AsciiGrid<T>::read_Grid().");

    int nCols,nRows;

    T noDataValue;
    string noDataFmt = dataFormat<T>("%s %<T>");

    T value;
    string valueFmt = dataFormat<T>("%<T>");

    fscanf(fin, "%d", &nCols);
    fscanf(fin, "%s %d", krap, &nRows);
    fscanf(fin, "%s %lf", krap, &xllCorner);
    fscanf(fin, "%s %lf", krap, &yllCorner);
    fscanf(fin, "%s %lf", krap, &cellSize);
    fscanf(fin, noDataFmt.c_str(), krap, &noDataValue);

    data.setMatrix(nRows,nCols,noDataValue);

    for(int i = nRows - 1;i >= 0;i--) {
        for (int j = 0;j < nCols;j++) {
            fscanf(fin, valueFmt.c_str(), &value);
            data(i,j) = T(value);
        }
    }
    fclose(fin);

    if(data.size() == 0)
        throw std::runtime_error("File has no data values in AsciiGrid<T>::read_Grid().");
}

/**
 * @brief stub for reading files using gdal.
 *
 * @warning Not Operational
 *
 * @todo add datatype support.
 *
 * This allows any number of raster data files to be read in as GDALDatasets
 *
 * @param inputFile file to read
 * @param band which band to read in, default = 1
 *
 */

template <class T>
void AsciiGrid<T>::GDALReadGrid(std::string inputFile, int band)
{
    GDALDatasetH hDataset;
    hDataset = GDALOpen(inputFile.c_str(), GA_ReadOnly);
    if(hDataset == NULL)
    {
        throw std::runtime_error(CPLSPrintf("Cannot open %s for reading",
                                             inputFile.c_str()));
    }
    const char* pszWkt = GDALGetProjectionRef(hDataset);
    if(pszWkt == NULL)
        throw std::runtime_error(CPLSPrintf("The file %s should contain "
                                            "projection information, but "
                                            "it does not.", inputFile.c_str()));

    int nXSize = GDALGetRasterXSize(hDataset);
    int nYSize = GDALGetRasterYSize(hDataset);

    double adfGeoTransform[6];
    GDALGetGeoTransform( hDataset, adfGeoTransform );

    double adfCorner[2];
    adfCorner[0] = adfGeoTransform[0];
    adfCorner[1] = adfGeoTransform[3] + (nYSize * adfGeoTransform[5]);

    GDALRasterBandH hBand = GDALGetRasterBand(hDataset, band);

    int pbSuccess = TRUE;
    double dfNoData = GDALGetRasterNoDataValue(hBand, &pbSuccess);
    if( pbSuccess == FALSE )
        dfNoData = -9999.0;

    //reallocate all memory and set header info    
    set_headerData(nXSize, nYSize, adfCorner[0], adfCorner[1],
                               adfGeoTransform[1], dfNoData, dfNoData, std::string(pszWkt));
    
    //set the data
    double *padfScanline;
    padfScanline = new double[nXSize];
    for(int i = nYSize - 1;i >= 0;i--) {
        check(GDALRasterIO(hBand, GF_Read, 0, i, nXSize, 1, padfScanline, nXSize, 1, GDT_Float64, 0, 0));
        for(int j = 0;j < nXSize;j++) {
            set_cellValue(nYSize - 1 - i, j, padfScanline[j]);
        }
    }

    delete [] padfScanline;
    GDALClose(hDataset);
    return;
}

template <class T>
void AsciiGrid<T>::get_cellPosition(int i, int j, double *xCoord, double *yCoord) const
{   //Function computes (xCoord, yCoord) in projected coordinates given cell index (i, j).
    if(j > data.get_numCols() || i > data.get_numRows() || j < 0 || i < 0)
    {
        #ifdef ASCII_GRID_DEBUG
            std::cout << "Invalid cell reference in AsciiGrid->get_cellPosition!" << std::endl;
        #endif
        throw std::range_error("Invalid cell reference in AsciiGrid<T>::get_cellPosition().");
        //return false;
    }
    *xCoord = (cellSize / 2.0) + (j * cellSize) + xllCorner;
    *yCoord = (cellSize / 2.0) + (i * cellSize) + yllCorner;
    //return true;
}

template <class T>
void AsciiGrid<T>::get_cellPositionLocalCoordinates(int i, int j, double *xCoord, double *yCoord) const
{   //Function computes (xCoord, yCoord) in the local coordinate system given cell index (i, j).
    //The local coordinate system is a coordinate system in which the lower left corner of the grid is (0, 0).
    if(j > data.get_numCols() || i > data.get_numRows() || j < 0 || i < 0)
    {
        #ifdef ASCII_GRID_DEBUG
            std::cout << "Invalid cell reference in AsciiGrid->get_cellPosition!" << std::endl;
        #endif
        throw std::range_error("Invalid cell reference in AsciiGrid<T>::get_cellPosition().");
        //return false;
    }
    *xCoord = (cellSize / 2.0) + (j * cellSize);
    *yCoord = (cellSize / 2.0) + (i * cellSize);
    //return true;
}

template <class T>
void AsciiGrid<T>::get_cellIndex(double xCoord, double yCoord, int *i, int *j) const
{   //Function computes the cell index (i, j) given a position (xCoord, yCoord) in the projected coordinate system.  Throws exception if (xCoord,yCoord) are outside the domain
    if(xCoord < xllCorner || yCoord < yllCorner ||
        xCoord > (xllCorner + ((data.get_numCols()) * cellSize)) ||
        yCoord > (yllCorner + ((data.get_numRows()) * cellSize)))
    {
        throw std::range_error("Invalid cell reference in AsciiGrid<T>::get_cellIndex().");
        //std::cout << endl << "Invalid cell reference in AsciiGrid->get_cellIndex!" << endl;
        //std::cout << setiosflags(ios_base::scientific);
        //std::cout << "xllCorner = " << xllCorner << "\t" << "yllCorner = " << yllCorner << "\t" << "xMax = " << (xllCorner + ((nCols) * cellSize)) << "\t" << "yMax = " << (yllCorner + ((nRows) * cellSize)) << endl;
        //std::cout << "i = " << *i << "\t" << "j = " <<  *j << "\t" << "xCoord = " << xCoord << "\t" << "yCoord = " << yCoord << endl;
        //std::cout << resetiosflags(ios_base::scientific);
        //return false;
    }

    *j = long(((xCoord - xllCorner) / cellSize));
    *i = long(((yCoord - yllCorner) / cellSize));
    //return true;
}

template <class T>
void AsciiGrid<T>::get_cellIndexLocalCoordinates(double xCoord, double yCoord, int *i, int *j) const
{   //Function computes the cell index (i, j) given a position (xCoord, yCoord) in the local coordinate system.  Throws exception if (xCoord,yCoord) are outside the domain
    //The local coordinate system is a coordinate system in which the lower left corner of the grid is (0, 0).
    if(xCoord < 0.0 || yCoord < 0.0 ||
        xCoord > (data.get_numCols() * cellSize) ||
        yCoord > (data.get_numRows() * cellSize))
    {
        throw std::range_error("Invalid cell reference in AsciiGrid<T>::get_cellIndex().");
        //std::cout << endl << "Invalid cell reference in AsciiGrid->get_cellIndex!" << endl;
        //std::cout << setiosflags(ios_base::scientific);
        //std::cout << "xllCorner = " << xllCorner << "\t" << "yllCorner = " << yllCorner << "\t" << "xMax = " << (xllCorner + ((nCols) * cellSize)) << "\t" << "yMax = " << (yllCorner + ((nRows) * cellSize)) << endl;
        //std::cout << "i = " << *i << "\t" << "j = " <<  *j << "\t" << "xCoord = " << xCoord << "\t" << "yCoord = " << yCoord << endl;
        //std::cout << resetiosflags(ios_base::scientific);
        //return false;
    }

    *j = long(xCoord / cellSize);
    *i = long(yCoord / cellSize);
    //return true;
}

template <class T>
void AsciiGrid<T>::get_nearestCellIndex(double xCoord, double yCoord, int *i, int *j) const
{   //Function computes the closest cell index (i, j) given a position (xCoord, yCoord) in the projected coordinate system. Works even when (xCoord, yCoord) is outside the grid's domain (doesn't throw like the other functions).
    if(xCoord < xllCorner || yCoord < yllCorner ||
        xCoord > (xllCorner + ((data.get_numCols()) * cellSize)) ||
        yCoord > (yllCorner + ((data.get_numRows()) * cellSize)))   // (xCoord,yCoord) are outside the grid's domain
    {
        //Use brute force computation of closest cell center (could make faster and more sophisticated, but not necessary at this time)
        double dist = std::numeric_limits<double>::max();
        double cellX, cellY, computedDist;
        for( int ii = 0;ii < data.get_numRows();ii++ )
        {
            for( int jj = 0;jj < data.get_numCols();jj++ )
            {
                get_cellPosition(ii, jj, &cellX, &cellY);
                computedDist = sqrt((cellX-xCoord)*(cellX-xCoord) + (cellY-yCoord)*(cellY-yCoord));
                if( computedDist < dist )
                {
                    dist = computedDist;
                    *i = ii;
                    *j = jj;
                }
            }
        }
    }else   // (xCoord,yCoord) are in the grid's domain
    {
        *j = long(((xCoord - xllCorner) / cellSize));
        *i = long(((yCoord - yllCorner) / cellSize));
    }
}

template <class T>
T AsciiGrid<T>::get_maxValue()
{
    return data.max();
}

template <class T>
T AsciiGrid<T>::get_minValue()
{
    return data.min();
}

template<class T>
double AsciiGrid<T>::get_meanValue() const
{
    return data.mean();
}

template<class T>
bool AsciiGrid<T>::find_firstValue(T m, T buffer, int *k, int *l)
{
    for(int i = data.get_numRows() - 1; i >= 0; i--)
    {
        for (int j = 0; j < data.get_numCols(); j++)
        {
            if(data(i,j) >= (m - buffer) && data(i,j) <= (m + buffer))
            {
                *k = i;
                *l = j;
                return true;
            }
        }
    }
    return false;
}

template <class T>
bool AsciiGrid<T>::check_inBounds(int m, int n) const
{
    if(m < 0 || n < 0)
        return false;
    else if(m > data.get_numRows() - 1 || n > data.get_numCols() - 1)
        return false;
    else
        return true;
}

template <class T>
bool AsciiGrid<T>::check_inBounds(double X, double Y) const
{
    if(X <= xllCorner || Y <= yllCorner)
        return false;
    else if(X >= (xllCorner + data.get_numCols() * cellSize) || Y >= (yllCorner + data.get_numRows() * cellSize))
        return false;
    else
        return true;
}

template <class T>
AsciiGrid<T> AsciiGrid<T>::resample_Grid(double resampleCellSize, interpTypeEnum interpType)
{
    double xDim = get_xDimension();
    double yDim = get_yDimension();
    double shortSide = 0;

    if(resampleCellSize == cellSize)
        return *this;

    (xDim > yDim) ? (shortSide = yDim) : (shortSide = xDim);

    //check and make sure cell size is at least 1/4??? the total size
    if(resampleCellSize > shortSide)
    {
        #ifdef ASCII_GRID_DEBUG
            std::cout << "Cell size too large!" << std::endl;
        #endif

            return *this;
    }

    int newNumCols = int(xDim / resampleCellSize);
    int newNumRows = int(yDim / resampleCellSize);

    AsciiGrid<T>A(newNumCols, newNumRows, xllCorner, yllCorner, resampleCellSize, data.getNoDataValue(), data.getNoDataValue(), prjString);

    double xC = 0;
    double yC = 0;

    for(int i = 0;i < newNumRows;i++)
    {
        for(int j = 0;j < newNumCols;j++)
        {
            A.get_cellPosition(i, j, &xC, &yC);

            A.set_cellValue(i, j, interpolateGrid(xC, yC, interpType));
        }
    }
    return A;
}

template <class T>
void AsciiGrid<T>::resample_Grid_in_place(double resampleCellSize, interpTypeEnum interpType)
{
    double xDim = get_xDimension();
    double yDim = get_yDimension();
    double shortSide = 0;
    (xDim > yDim) ? (shortSide = yDim) : (shortSide = xDim);

    if(cellSize != resampleCellSize)
    {
        //check and make sure cell size is at least 1/4??? the total size
        if(resampleCellSize > shortSide)
        {
            #ifdef ASCII_GRID_DEBUG
                std::cout << "Cell size too large!" << std::endl;
            #endif

            throw std::runtime_error("Desired resampleCellSize is too large for grid in AsciiGrid<T>::resample_Grid_in_place().");
            //return 0;
        }

        int newNumCols = int(xDim / resampleCellSize);
        int newNumRows = int(yDim / resampleCellSize);

        AsciiGrid<T>A(newNumCols, newNumRows, xllCorner, yllCorner, resampleCellSize, data.getNoDataValue(), data.getNoDataValue(), prjString);

        double xC = 0;
        double yC = 0;

        for(int i = 0;i < newNumRows;i++)
        {
            for(int j = 0;j < newNumCols;j++)
            {
                A.get_cellPosition(i, j, &xC, &yC);

                A.set_cellValue(i, j, interpolateGrid(xC, yC, interpType));
            }
        }
        *this = A;
    }
}

template <class T>
void AsciiGrid<T>::resample_Grid_in_place(int arraySize, interpTypeEnum interpType)
{           // Given arraySize (approximate number of desired surface cells), resample the grid
            //      I think this function will  mostly be used for resampling DEMs for viewing
            //      in 3D graphics windows like the OpenGL window in the GUI
    double xDim = get_xDimension();
    double yDim = get_yDimension();
    double area = xDim*yDim;
    double cellArea = area/arraySize;
    double resampleCellSize = std::sqrt(cellArea);
    double shortSide = 0;
    (xDim > yDim) ? (shortSide = yDim) : (shortSide = xDim);

    if(cellSize != resampleCellSize)
    {
        //check and make sure cell size is at least 1/4??? the total size
        if(resampleCellSize > shortSide)
        {
            #ifdef ASCII_GRID_DEBUG
                std::cout << "Cell size too large!" << std::endl;
            #endif

            throw std::runtime_error("Desired resampleCellSize is too large for grid in AsciiGrid<T>::resample_Grid_in_place().");
            //return false;
        }

        int newNumCols = int(xDim / resampleCellSize);
        int newNumRows = int(yDim / resampleCellSize);

        AsciiGrid<T>A(newNumCols, newNumRows, xllCorner, yllCorner, resampleCellSize, data.getNoDataValue(), data.getNoDataValue(), prjString);

        double xC = 0;
        double yC = 0;

        for(int i = 0;i < newNumRows;i++)
        {
            for(int j = 0;j < newNumCols;j++)
            {
                A.get_cellPosition(i, j, &xC, &yC);

                A.set_cellValue(i, j, interpolateGrid(xC, yC, interpType));
            }
        }

        *this = A;
    }
}

/**
 * \brief Add cells to an ascii grid
 *
 * Add or remove one or more cells to the Grid, typically 1 cell to cover the
 * entire grid that is input.
 *
 * \param nAddCols number of columns to add
 * \param nAddRows number of rows to add
 * \return a new grid with a buffer
 */
template<class T>
AsciiGrid<T> AsciiGrid<T>::BufferGrid( int nAddCols, int nAddRows )
{
    int nOrigXSize = get_nCols();
    int nOrigYSize = get_nRows();
    if( nOrigXSize + nAddCols <= 0 || nOrigYSize + nAddRows <= 0 )
    {
        throw std::range_error("Invalid number of rows or columns to be "
                               "removed");
    }
    AsciiGrid<T>A( get_nCols() + nAddCols, get_nRows() + nAddRows, get_xllCorner(),
                   get_yllCorner(), get_cellSize(), get_noDataValue(),
                   prjString );
    for( int i = 0;i < A.get_nRows();i++ )
    {
        for( int j = 0;j < A.get_nCols();j++ )
        {
            if( i < nOrigYSize && j < nOrigXSize )
            {
                A.set_cellValue( i, j, get_cellValue( i, j ) );
            }
            else if( i > nOrigYSize - 1  && j > nOrigXSize - 1 )
            {
                A.set_cellValue( i, j, get_cellValue( nOrigYSize - 1,
                                                      nOrigXSize - 1 ) );
            }
            else if( i > nOrigYSize - 1 )
            {
                A.set_cellValue( i, j, get_cellValue( nOrigYSize - 1, j ) );
            }
            else if( j > nOrigXSize - 1 )
            {
                A.set_cellValue( i, j, get_cellValue( i, nOrigXSize - 1 ) );
            }
        }
    }
    return A;
}

/**
 * \brief Add cells to an ascii grid
 *
 * Add or remove one or more cells to the Grid, typically 1 cell to cover the
 * entire grid that is input.
 *
 * \param nAddCols number of columns to add
 * \param nAddRows number of rows to add
 */
template<class T>
void AsciiGrid<T>::BufferGridInPlace( int nAddCols, int nAddRows )
{
    AsciiGrid<T>A = BufferGrid(nAddCols, nAddRows);
    *this = A;
}

/**
 * \brief Add or remove cells around all edges of an ascii grid
 *
 * Add or remove one or more cells around the Grid.
 *
 * \param nAddCols number of columns to add
 * \param nAddRows number of rows to add
 * \return a new grid with a buffer
 */
template<class T>
AsciiGrid<T> AsciiGrid<T>::BufferAroundGrid( int nAddCols, int nAddRows )
{
    int nOrigXSize = get_nCols();
    int nOrigYSize = get_nRows();
    if( nOrigXSize + nAddCols <= 0 || nOrigYSize + nAddRows <= 0 )
    {
        throw std::range_error("Invalid number of rows or columns to be "
                               "removed");
    }
    AsciiGrid<T>A( get_nCols() + 2*nAddCols, get_nRows() + 2*nAddRows,
            get_xllCorner()-(nAddCols*get_cellSize()),
            get_yllCorner()-(nAddRows*get_cellSize()),
            get_cellSize(), get_noDataValue(), get_noDataValue(), prjString );

    for( int i = 0;i < nOrigYSize;i++ )
    {
        for( int j = 0;j < nOrigXSize;j++ )
        {
            if(i+nAddRows < 0 || i+nAddRows >= A.get_nRows() ||
                j+nAddCols < 0 || j+nAddCols >= A.get_nCols())
            {
                continue;
            } 
            A.set_cellValue( i+nAddRows, j+nAddCols, get_cellValue(i, j));
        }
    }
    if(!A.fillNoDataValues(1, 50.0, 1000))
        throw std::runtime_error("Could not fill no data values in AsciiGrid::BufferAroundGrid()");

    return A;
}

/**
 * \brief Add cells to overlap another ascii grid
 *
 * Add cells to overlap an ascii grid.
 *
 * \param A grid to buffer
 */
template<class T>
void AsciiGrid<T>::BufferToOverlapGrid( AsciiGrid &A )
{
    double xMinOverlap, yMinOverlap, xMaxOverlap, yMaxOverlap;
    double biggestX, biggestY;
    int nColsBuffer, nRowsBuffer;
    xMinOverlap = get_xllCorner() - A.get_xllCorner();
    xMaxOverlap = (A.get_xllCorner() + A.get_nCols()*A.get_cellSize()) -
        (get_xllCorner() + get_nCols()*get_cellSize());
    yMinOverlap = get_yllCorner() - A.get_yllCorner();
    yMaxOverlap = (A.get_yllCorner() + A.get_nRows()*A.get_cellSize()) -
        (get_yllCorner() + get_nRows()*get_cellSize());

    if(!(xMinOverlap < 0.0 && xMaxOverlap < 0.0 && yMinOverlap < 0.0 && yMaxOverlap < 0.0))
    {
        //if we only need to buffer on top and right side of grid
        if(get_xllCorner() <= A.get_xllCorner() && get_yllCorner() <= A.get_yllCorner())
        {
            nColsBuffer = (int)(xMaxOverlap/get_cellSize())+1;
            nRowsBuffer = (int)(yMaxOverlap/get_cellSize())+1;
            if(nColsBuffer < 0)
                nColsBuffer = 0;
            if(nRowsBuffer < 0)
                nRowsBuffer = 0;
            BufferGridInPlace(nColsBuffer, nRowsBuffer);
        }
        else //if we need to buffer all sides of the grid
        {
            biggestX = xMaxOverlap;
            if(xMinOverlap > biggestX)
                biggestX = xMinOverlap;
            biggestY = yMaxOverlap;
            if(yMinOverlap > biggestY)
                biggestY = yMinOverlap;
            nColsBuffer = (int)(biggestX/get_cellSize())+1;
            nRowsBuffer = (int)(biggestY/get_cellSize())+1;
            if(nColsBuffer < 0)
                nColsBuffer = 0;
            if(nRowsBuffer < 0)
                nRowsBuffer = 0;
            BufferAroundGridInPlace(nColsBuffer, nRowsBuffer);
        }
    }
}

/**
 * \brief Check if one grid overlaps another 
 *
 * Check if one grid completely overlaps another grid.
 *
 * \param A grid to check overlap against
 * \return true if there is complete overalap, otherwise false
 */
template<class T>
bool AsciiGrid<T>::CheckForGridOverlap( AsciiGrid &A )
{
    double xMinOverlap, yMinOverlap, xMaxOverlap, yMaxOverlap;
    xMinOverlap = get_xllCorner() - A.get_xllCorner();
    xMaxOverlap = (get_xllCorner() + get_nCols()*get_cellSize()) -
        (A.get_xllCorner() + A.get_nCols()*A.get_cellSize());
    yMinOverlap = get_yllCorner() - A.get_yllCorner();
    yMaxOverlap = (get_yllCorner() + get_nRows()*get_cellSize()) -
        (A.get_yllCorner() + A.get_nRows()*A.get_cellSize());

    if(!(xMinOverlap < 0.0 && xMaxOverlap < 0.0 && yMinOverlap < 0.0 && yMaxOverlap < 0.0))
    {
        return true;
    }
    else
        return false;
}

template<class T>
bool AsciiGrid<T>::fillNoDataValues( int minNeighborCells, double maxPercentNoData, int maxNumPasses )
{
    int numNoDataValues = 0;
    for(int i = 0;i < data.get_numRows();i++)
    {
        for(int j = 0;j < data.get_numCols();j++)
        {
            if(get_cellValue(i,j) == get_noDataValue() || cplIsNan(get_cellValue(i,j)))
                numNoDataValues++;
        }
    }
    if(numNoDataValues == 0)
        return true;
    else if(numNoDataValues == (data.get_numRows() * data.get_numCols()))
        throw std::runtime_error("The grid does not contain any values. Cannot fill with AsciiGrid<T>::fillNoDataValues().");
    
    double percentNoData = 100.0 * numNoDataValues / (data.get_numRows()*data.get_numCols());
    double sum;
    int nValues;
    int numPasses = 0;
    if(percentNoData > maxPercentNoData)
    {
        return false;
    }else{
        do{
            numNoDataValues = 0;
            numPasses++;
            for(int i = 0;i < data.get_numRows();i++)
            {
                for(int j = 0;j < data.get_numCols();j++)
                {
                    if(get_cellValue(i, j) == get_noDataValue() || cplIsNan(get_cellValue(i,j)))
                    {
                        sum = 0.0;
                        nValues = 0;
                        for(int ii = i-1; ii <= i+1; ii++)
                        {
                            for(int jj = j-1; jj <= j+1; jj++)
                            {
                                if(ii < 0 || ii >= get_nRows() ||
                                    jj < 0 || jj >= get_nCols())
                                    continue;

                                if(get_cellValue(ii, jj) == get_noDataValue() || cplIsNan(get_cellValue(ii, jj)))
                                    continue;

                                sum = sum + get_cellValue(ii, jj);
                                nValues++;
                            }
                        }
                        if(nValues > 0)
                            set_cellValue(i, j, sum/nValues);
                        else
                            numNoDataValues++;
                    }
                }
            }
        }while(numNoDataValues > 0 && numPasses <= maxNumPasses);
    }
    return true;
}

/**
 * \brief Add cells to all edges of an ascii grid
 *
 * Add or remove one or more cells around the Grid.
 *
 * \param nAddCols number of columns to add
 * \param nAddRows number of rows to add
 */
template<class T>
void AsciiGrid<T>::BufferAroundGridInPlace( int nAddCols, int nAddRows )
{
    AsciiGrid<T>A = BufferAroundGrid(nAddCols, nAddRows);
    *this = A;
}

template <class T>
void AsciiGrid<T>::interpolateFromGrid(AsciiGrid &A, interpTypeEnum interpType)
{   //Function interpolates data from A onto the current grid

    double xC, yC;

    //Set noData value to be the same (since A.interpolateGrid() returns its noDataValue if xC,yC is out of bounds)
    data.setNoDataValue(A.data.getNoDataValue());

    for(int i = 0;i < data.get_numRows();i++)
    {
        for(int j = 0;j < data.get_numCols();j++)
        {
            get_cellPosition(i, j, &xC, &yC);

            set_cellValue(i, j, A.interpolateGrid(xC, yC, interpType));
        }
    }
}

template <class T>
void AsciiGrid<T>::interpolateFromPoints(T* pointData, double* X, double* Y, double* influenceRadius, int numPoints, double interpDistPower)
{   //Function interpolates from an array of point data using inverse distance squared weighting
    //pointData is the array of values at the points, X and Y are the coordinates of the points, influenceRadius is
    //the array containing the maximum interpolation distance for each station (if <0 then infinite influence radius)
    //numPoints is the number of points, and
    //interpDistPower is the power used for the distance weighting (usually 1.0 or 2.0 for inverse distance weighting or inverse distance squared weighting, respectively)


    double weight, weight_sum, distance, xC, yC;
    T value;

    if(interpDistPower <= 0)
        throw std::out_of_range("interpDistPower in AsciiGrid<T>::interpolateFromPoints() must be greater than 0.");
    if(numPoints <=0)
        throw std::out_of_range("numPoints in AsciiGrid<T>::interpolateFromPoints() must be greater than 0.");


    *this = data.getNoDataValue();

    for(int i = 0;i < data.get_numRows();i++)
    {
        for(int j = 0;j < data.get_numCols();j++)
        {
            value = 0.0;
            get_cellPosition(i, j, &xC, &yC);
            weight_sum = 0.0;

            for(int k = 0; k < numPoints; k++)
            {
                distance = std::sqrt((xC-X[k])*(xC-X[k]) + (yC-Y[k])*(yC-Y[k]));
                if(influenceRadius[k] >= 0.0)   //negative influence radius means infinite influence radius
                {
                    if(distance > influenceRadius[k])   //if distance from current cell location to station is larger than the influence radius, skip this station
                        continue;
                }
                weight = 1.0/std::pow(distance,interpDistPower);
                weight_sum = weight_sum + weight;
                value = value + pointData[k] * weight;
            }
            if(weight_sum != 0) //if value IS zero, then don't set the value because this means all points don't have an influence radius that reaches this cell (this effectively leaves the value as the no_data value)
                set_cellValue(i, j, value/weight_sum);
        }
    }


}

template <class T>
void AsciiGrid<T>::clipGridInPlaceSnapToCells(double percentClip)
{
    if(percentClip < 0.0 || percentClip >= 50.0)
    {
        std::ostringstream buff_str;
        buff_str << "The AsciiGrid clipping value is improperly set to " << percentClip << ".  It should be 0-50 percent.";
        throw std::out_of_range(buff_str.str().c_str());
    }

    if(percentClip != 0.0)  //if it's zero, just return without doing anything
    {

        double xClipDist, yClipDist, newXllCorner, newYllCorner;
        int newNumCols, newNumRows, xClipCells, yClipCells;

        xClipDist = get_xDimension() * percentClip/100.0;
        yClipDist = get_yDimension() * percentClip/100.0;

        xClipCells = (int) (xClipDist / cellSize + 0.5);    //the 1/2 cellsize makes rounding correct since xClipCells is an int, not double
        yClipCells = (int) (yClipDist / cellSize + 0.5);

        if(!(xClipCells ==0 && yClipCells == 0))    //if these are both zero, exit without doing anything
        {
            newNumCols = data.get_numCols() - 2*xClipCells;
            newNumRows = data.get_numRows() - 2*yClipCells;

            newXllCorner = xllCorner + xClipCells*cellSize;
            newYllCorner = yllCorner + yClipCells*cellSize;

            AsciiGrid<T>A(newNumCols, newNumRows, newXllCorner, newYllCorner, cellSize, data.getNoDataValue(), data.getNoDataValue(), prjString);

            for(int i=0; i<A.get_nRows(); i++)
            {
                for(int j=0; j<A.get_nCols(); j++)
                {
                    A.set_cellValue(i,j,get_cellValue(i+yClipCells, j+xClipCells));
                }
            }

            *this = A;
        }
    }
}

template <class T>
AsciiGrid<T> AsciiGrid<T>::normalize_Grid(T lowBound, T highBound)
{
  AsciiGrid<T>A(*this);
  A = get_noDataValue();
  T high, low, newRange, oldRange;
  high = get_maxValue();
  low = get_minValue();
  newRange = highBound - lowBound;
  oldRange = high - low;

  T value = 0;

  for(int i = 0;i < get_nRows();i++)
    {
      for(int j = 0;j < get_nCols();j++)
    {
      value = ((data(i,j) / oldRange) * newRange) + lowBound;
      A.set_cellValue(i, j, value);
    }
    }

  return A;
}

template <class T>
T AsciiGrid<T>::interpolateGrid(double x, double y, interpTypeEnum interpType) const
{
    int i = 0;
    int j = 0;
    double xCoord = x;
    double yCoord = y;
    T t = 0;
    T u = 0;
    T answer = 0;
    T val1, val2, val3, val4;

    switch(interpType)
    {
        //nearest neighbor, 0th order interpolation.
        case order0:
            get_cellIndex(xCoord, yCoord, &i, &j);
            answer = get_cellValue(i, j);
            return answer;

        //bilinear interpolation
        case order1:
            if(xCoord >= (get_xllCorner() + (get_xDimension() - (get_cellSize() / 2)))
                || xCoord <= get_xllCorner() + (get_cellSize() / 2)
                || yCoord >= (get_yllCorner() + (get_yDimension() - (get_cellSize() / 2)))
                || yCoord <= get_yllCorner() + (get_cellSize() / 2))
            {
                get_cellIndex(xCoord, yCoord, &i, &j);
                answer =  get_cellValue(i, j);

                return answer;
            }
            else
            {
                get_cellIndex((xCoord - get_cellSize() / 2), (yCoord - get_cellSize() / 2), &i, &j);

                t = (yCoord - ((i * get_cellSize() + (get_cellSize() / 2)) + get_yllCorner())) /

                    ((((i + 1) * get_cellSize() + (get_cellSize() / 2)) + get_yllCorner()) -

                    (((i * get_cellSize() + (get_cellSize() / 2))) + get_yllCorner()));

                u = (xCoord - ((j * get_cellSize() + (get_cellSize() / 2)) + get_xllCorner())) /

                    ((((j + 1) * get_cellSize() + (get_cellSize() / 2)) + get_xllCorner()) -

                    (((j * get_cellSize() + (get_cellSize() / 2))) + get_xllCorner()));

                val1 = get_cellValue(i, j);
                val2 = get_cellValue(i + 1, j);
                val3 = get_cellValue(i + 1, j + 1);
                val4 = get_cellValue(i, j + 1);

                if(val1==data.getNoDataValue() || val2==data.getNoDataValue() || val3==data.getNoDataValue() || val4==data.getNoDataValue())
                {
                    return data.getNoDataValue();
                }

                answer = (1 - t) * (1 - u) * val1
                     + t * (1 - u) * val2
                     + t * u * val3
                     + (1 - t) * u * val4;

                return answer;
            }
        //bicubic interpolation
        case order2:
            return answer;
        //bicubic spline
        case order3:
            return answer;
        default:
            return 0.0;
    }
}

template <class T>
T AsciiGrid<T>::interpolateGridLocalCoordinates(double x, double y, interpTypeEnum interpType) const
{
    int i = 0;
    int j = 0;
    double xCoord = x;
    double yCoord = y;
    T t = 0;
    T u = 0;
    T answer = 0;
    T val1, val2, val3, val4;

    switch(interpType)
    {
        //nearest neighbor, 0th order interpolation.
        case order0:
            get_cellIndexLocalCoordinates(xCoord, yCoord, &i, &j);
            answer = get_cellValue(i, j);
            return answer;

        //bilinear interpolation
        case order1:
            if(xCoord >= ((get_xDimension() - (get_cellSize() / 2)))
                || xCoord <= (get_cellSize() / 2)
                || yCoord >= ((get_yDimension() - (get_cellSize() / 2)))
                || yCoord <= (get_cellSize() / 2))
            {
                get_cellIndexLocalCoordinates(xCoord, yCoord, &i, &j);
                answer =  get_cellValue(i, j);

                return answer;
            }
            else
            {
                get_cellIndexLocalCoordinates((xCoord - get_cellSize() / 2), (yCoord - get_cellSize() / 2), &i, &j);

                t = (yCoord - ((i * get_cellSize() + (get_cellSize() / 2)))) /

                    ((((i + 1) * get_cellSize() + (get_cellSize() / 2))) -

                    (((i * get_cellSize() + (get_cellSize() / 2)))));

                u = (xCoord - ((j * get_cellSize() + (get_cellSize() / 2)))) /

                    ((((j + 1) * get_cellSize() + (get_cellSize() / 2))) -

                    (((j * get_cellSize() + (get_cellSize() / 2)))));

                val1 = get_cellValue(i, j);
                val2 = get_cellValue(i + 1, j);
                val3 = get_cellValue(i + 1, j + 1);
                val4 = get_cellValue(i, j + 1);

                if(data.hasNoDataValues())
                {
                    if(val1==data.getNoDataValue() || val2==data.getNoDataValue() || val3==data.getNoDataValue() || val4==data.getNoDataValue())
                        return data.getNoDataValue();
                }

                answer = (1 - t) * (1 - u) * val1
                     + t * (1 - u) * val2
                     + t * u * val3
                     + (1 - t) * u * val4;

                return answer;
            }
        //bicubic interpolation
        case order2:
            return answer;
        //bicubic spline
        case order3:
            return answer;
        default:
            return 0.0;
    }
}

template <class T>
void AsciiGrid<T>::replaceValue( T initial, T final )
{
    for(int i = 0; i < data.get_numRows(); i++)
    {
        for (int j = 0; j < data.get_numCols(); j++)
        {
            if( data(i,j) == initial )
            data(i,j) = final;
        }
    }
}

template <class T>
void AsciiGrid<T>::replaceNan( T final )
{
    for(int i = 0;i < data.get_numRows();i++)
    {
        for (int j = 0;j < data.get_numCols();j++)
        {
            if( cplIsNan(data(i,j)) )
            data(i,j) = final;
        }
    }
}

template <class T>
void AsciiGrid<T>::print_GridInfo() const
{
    std::cout << "ncols"
        << "\t"
        << data.get_numCols()
        << std::endl;
    std::cout << "nrows"
        << "\t"
        << data.get_numRows()
        << std::endl;
    std::cout << "xllcorner"
        << "\t"
        << setiosflags(std::ios::fixed)
        << std::setprecision(2)
        << xllCorner
        << std::endl;
    std::cout << "yllcorner"
        << "\t"
        << setiosflags(std::ios::fixed)
        << std::setprecision(2)
        << yllCorner
        << std::endl;
    std::cout << "cellsize"
        << "\t" << setiosflags(std::ios::fixed)
        << std::setprecision(2) << cellSize
        << std::endl;
    std::cout << "NODATA_value"
        << "\t"
        << std::setprecision(2)
        << data.getNoDataValue()
        << std::endl;

    //return true;
}

template <class T>
void AsciiGrid<T>::print_Grid() const
{
    print_GridInfo();

    for(int i = data.get_numRows() - 1;i >= 0;i--)
    {
        for (int j = 0;j < data.get_numCols();j++)
        {
            std::cout << setiosflags(std::ios::fixed)
                    << std::setprecision(2)
                    << data(i,j)
                    << "\t";
        }
        std::cout << std::endl;
    }
    //return true;
}

template <class T>
void AsciiGrid<T>::write_GridInfo(FILE *fileOut)
{
    string noDataFmt = dataFormat<T>("NODATA_value\t%<T>\n");

    fprintf(fileOut,"ncols\t%d\n",data.get_numCols());
    fprintf(fileOut,"nrows\t%d\n",data.get_numRows());
    fprintf(fileOut,"xllcorner\t%lf\n",xllCorner);
    fprintf(fileOut,"yllcorner\t%lf\n",yllCorner);
    fprintf(fileOut,"cellsize\t%lf\n",cellSize);
    fprintf(fileOut,noDataFmt.c_str(),data.getNoDataValue());
    //return true;
}

template <class T>
void AsciiGrid<T>::write_integralGridData (FILE* fout) {
    int nRows = data.get_numRows();
    int nCols = data.get_numCols();

    for (int i=nRows-1; i>=0; i--) {
        for (int j=0; j<nCols; j++) {
            fprintf(fout,"%ld\t", (long)data(i,j));
        }
        fprintf(fout,"\n");
    }
}


template <class T>
void AsciiGrid<T>::write_fpGridData (FILE* fout, int numDecimals) {
    string fmt = dataFormat<T>("%.*<T>\t");

    int nRows = data.get_numRows();
    int nCols = data.get_numCols();

    for (int i=nRows-1; i>=0; i--) {
        for (int j=0; j<nCols; j++) {
            fprintf(fout,fmt.c_str(), numDecimals, data(i,j));
        }
        fprintf(fout,"\n");
    }
}

template <class T>
void AsciiGrid<T>::write_json_Grid(std::string outputFile, int numDecimals)
{
    FILE *fout = fopen(outputFile.c_str(), "w");
    int nCols = data.get_numCols();
    int nRows = data.get_numRows();

    string noDataFmt = dataFormat<T>("\"NODATA_value\":%<T>\n");
    string dataFmt = dataFormat<T>("%.*<T>,");

    printf("writing JSON output: %s\n", outputFile.c_str());
    fprintf(fout, "{\n");

    fprintf( fout, "\"ncols\":%d,\n", nCols);
    fprintf( fout, "\"nrows\":%d,\n", nRows);
    fprintf( fout, "\"xllcorner\":%lf\n", xllCorner);
    fprintf( fout, "\"yllcorner\":%lf\n", yllCorner);
    fprintf( fout, "\"cellsize\":%lf\n", cellSize);
    fprintf( fout, noDataFmt.c_str(), data.getNoDataValue());

    // this is not in the AAIGRID specs but since this is our own JSON version we just add it here
    string const wkt = std::regex_replace( std::string(prjString), regex("\""), std::string("\\\""));
    fprintf( fout, "\"WKT\":\"%s\"\n", wkt.c_str());

    fprintf( fout, "\"data\":[\n");
    for (int i=nRows-1; i>=0; i--) {
        for (int j=0; j<nCols; j++) {
            fprintf( fout, dataFmt.c_str(), numDecimals, data(i,j));
        }
        fprintf(fout,"\n");
    }
    fprintf(fout, "]\n}\n");

    fclose(fout);
}

template <class T>
void AsciiGrid<T>::write_Grid(std::string outputFile, int numDecimals)
{
    FILE *fout;
    fout = fopen(outputFile.c_str(), "w");
    if(fout == NULL)
    //if((fout = fopen("kyle_sucks.com", "w")) == NULL)
    {
        #ifdef ASCII_GRID_DEBUG
            std::cout << "Cannot open output file, exiting..." << std::endl;
        #endif
        throw std::runtime_error("Cannot open output file in AsciiGrid<T>::write_Grid().");
        //return false;
    }

    CPLDebug("WINDNINJA", "writing ascii output: %s", outputFile.c_str());

    write_GridInfo(fout);

    if(numDecimals < -1 || numDecimals > 3)
        numDecimals = 2;

    if (numDecimals == -1) { 
        write_integralGridData(fout);
    } else { 
        write_fpGridData(fout,numDecimals);
    }

    fclose(fout);

    //write projection file
    if(!prjString.empty())
    {
        std::string outPrjFileName(outputFile);
        int strPos = outPrjFileName.find_last_of('.');
        if(strPos > 0)
            outPrjFileName.erase(strPos);
        outPrjFileName.append(".prj");
        std::ofstream outputFile(outPrjFileName.c_str(), std::fstream::trunc);
        outputFile << prjString;
        outputFile.close();
    }

    #ifdef ASCII_GRID_DEBUG
        std::cout << "File written." << std::endl;
    #endif

    #ifdef ASCII_GRID_DEBUG
        std::cout << "File closed." << std::endl;
    #endif
    //return true;
}

template<class T>
void AsciiGrid<T>::sort_grid()
{
    if(sortedData)
        delete[]sortedData;

    sortedData = data.sortData();

}

template<class T>
void AsciiGrid<T>::divide_gridData(double *d, int splits)
{
    if(!sortedData)
        sort_grid();
    int x = int(floor((double)(data.size() / splits)));;
    for(int i = 0;i < splits;i++)
    {
        d[i] = sortedData[i * x];
    }
}

template<class T>
void AsciiGrid<T>::get_gridCenter(double *x, double *y)
{
  *x = get_xllCorner() + (get_xDimension() / 2);
  *y = get_yllCorner() + (get_yDimension() / 2);
  //return true;
}

template<class T>
bool AsciiGrid<T>::checkForNoDataValues()
{
    return data.hasNoDataValues();
}

template<class T>
bool AsciiGrid<T>::checkForCoincidentGrids(AsciiGrid &A)
{
    if(A.data.get_numRows() != data.get_numRows())
        return false;
    if(A.data.get_numCols() != data.get_numCols())
        return false;
    if(A.cellSize != cellSize)
        return false;
    if(A.xllCorner != xllCorner)
        return false;
    if(A.yllCorner != yllCorner)
        return false;

    return true;
}

/**
 * Create an in-meomory GDALDataset from an ascii grid.
 *
 */

template <class T>
GDALDatasetH AsciiGrid<T>::ascii2GDAL()
{
    GDALDatasetH hDS;
    GDALDriverH hDriver = GDALGetDriverByName( "MEM" );

    int nXSize = get_nCols();
    int nYSize = get_nRows();

    hDS = GDALCreate(hDriver, "", nXSize, nYSize, 1, GDT_Float64, NULL);

    double adfGeoTransform[6] = {get_xllCorner(),  get_cellSize(), 0,
                                get_yllCorner()+(get_nRows()*get_cellSize()),
                                0, -(get_cellSize())};

    GDALSetGeoTransform(hDS, adfGeoTransform);

    double *padfScanline;
    padfScanline = new double[nXSize];
    CPLErr eErr = CE_None;

    adfGeoTransform[0] = get_xllCorner();
    adfGeoTransform[1] = get_cellSize();
    adfGeoTransform[2] = 0;
    adfGeoTransform[3] = get_yllCorner()+(get_nRows()*get_cellSize());
    adfGeoTransform[4] = 0;
    adfGeoTransform[5] = -get_cellSize();
    
    char* pszDstWKT = (char*)prjString.c_str();
    GDALSetProjection(hDS, pszDstWKT);
    GDALSetGeoTransform(hDS, adfGeoTransform);
    
    GDALRasterBandH hBand = GDALGetRasterBand( hDS, 1 );
    
    GDALSetRasterNoDataValue(hBand, -9999.0);

    for(int i=nYSize-1; i>=0; i--)
    {
        for(int j=0; j<nXSize; j++)
        {
            padfScanline[j] = get_cellValue(nYSize-1-i, j);
        }
        eErr = GDALRasterIO(hBand, GF_Write, 0, i, nXSize, 1, padfScanline, nXSize,
                            1, GDT_Float64, 0, 0); 
        assert( eErr == CE_None );
    }

    return hDS;
}
    

/**
 * Create a png for an ascii grid.
 * Creates a color (RGBA) png.
 *
 * @param outFilename output file to write
 * @param legendTitle title to be printed in legend
 * @param legendUnits units to be printed in legend
 * @param scalarLegendFilename filename for optional legend
 * @param writeLegend flag to indicate whether or not to write a legend
 *
 */
template <class T>
void AsciiGrid<T>::ascii2png(std::string outFilename,
                             std::string legendTitle,
                             std::string legendUnits,
                             std::string scalarLegendFilename,
                             bool writeLegend)
{
    GDALDataset *poDS;
    GDALDriver *tiffDriver = GetGDALDriverManager()->GetDriverByName("GTiff");
    char** papszOptions = NULL;
    std::string tempFileout = "temp_fileout";

    poDS = tiffDriver->Create(tempFileout.c_str(), get_nCols(), get_nRows(), 1,
                   GDT_Byte, papszOptions);

    double adfGeoTransform[6] = {get_xllCorner(),  get_cellSize(), 0,
                                get_yllCorner()+(get_nRows()*get_cellSize()),
                                0, -(get_cellSize())};
    poDS->SetGeoTransform(adfGeoTransform);
    int nXSize = poDS->GetRasterXSize();
    int nYSize = poDS->GetRasterYSize();

    GDALRasterBand *poBand = poDS->GetRasterBand(1);
    
    //this->write_Grid("this_grid", 2);
    

    /* -------------------------------------------------------------------- */
    /*  Scale data for the color table                                      */
    /* -------------------------------------------------------------------- */
    
    AsciiGrid<T>scaledDataGrid(*this);
    double translation = 0;
    double scalingFactor = 1;

    // convert nodata values to 0 (0 is transparent channel in color table)
    for(int i=0;i<scaledDataGrid.get_nRows();i++)
    {
        for(int j=0;j<scaledDataGrid.get_nCols();j++)
        {
            if(scaledDataGrid(i,j) == 0)
            {
                scaledDataGrid(i,j) = epsClr<T>(); //if want to show *real* 0 values in image
            }
            if(scaledDataGrid(i,j) == scaledDataGrid.get_noDataValue())
            {
                scaledDataGrid(i,j) = 0;
            }
        }
    }//scaledDataGrid.write_Grid("scaled_datagrid", 2);

    // need min value (without 0s) later to make legend
    double raw_minValue = std::numeric_limits<double>::max();
    for(int i=0; i<scaledDataGrid.get_nRows(); i++)
        {
            for (int j=0;j<scaledDataGrid.get_nCols();j++)
            {
                if(scaledDataGrid(i,j) < raw_minValue && scaledDataGrid(i,j) != get_noDataValue() &&
                   scaledDataGrid(i,j)!=0)
                {
                    raw_minValue = scaledDataGrid(i,j);
                }
            }
        }
        
    double raw_maxValue = std::numeric_limits<double>::min();
    for(int i = scaledDataGrid.get_nRows() - 1;i >= 0;i--)
    {
        for (int j = 0;j < scaledDataGrid.get_nCols();j++)
        {
            if(scaledDataGrid(i,j) > raw_maxValue && scaledDataGrid(i,j) != get_noDataValue())
            {
                raw_maxValue = scaledDataGrid(i,j);
            }
        }
    }

    if(raw_minValue<0)
    {
        translation = fabs(raw_minValue)+1.1; //+1.1 since 0 = transparent in color table
    }
    else if(raw_minValue<2 && raw_minValue!=0)
    {
        translation = raw_minValue+1.1;
    }
    else if(raw_minValue!=0)  //since 0 is transparent channel
    {
        translation = -raw_minValue-1.1;
    }
    if(raw_maxValue>255 || raw_maxValue-raw_minValue<5)
    {
        scalingFactor = 254/(raw_maxValue+translation);
    }
    for(int i=0;i<scaledDataGrid.get_nRows();i++)
    {
        for(int j=0;j<scaledDataGrid.get_nCols();j++)
        {
            {
                if(scaledDataGrid(i,j)!=0)
                {
                    scaledDataGrid(i,j) = scaledDataGrid(i,j)+translation;
                    scaledDataGrid(i,j) = scaledDataGrid(i,j)*0.6*scalingFactor; //0.6 is temp fix for scaling issue
                }
            }
        }
    } //scaledDataGrid.write_Grid("scaled_datagrid_again", 2);

    /* -------------------------------------------------------------------- */
    /*  Fill in band with scaled data                                       */
    /* -------------------------------------------------------------------- */

    double *padfScanline;
    padfScanline = new double[nXSize];

    for(int i=nYSize-1;i>=0;i--)
    {
        for(int j=0;j<nXSize;j++)
        {
            padfScanline[j] = scaledDataGrid.get_cellValue(nYSize-1-i, j);
            check(poBand->RasterIO(GF_Write, 0, i, nXSize, 1, padfScanline, nXSize, 1, GDT_Float64, 0, 0));
        }
    }

    /* -------------------------------------------------------------------- */
    /*  Make a color table                                                  */
    /* -------------------------------------------------------------------- */

    GDALColorTable *poCT = new GDALColorTable();
    GDALColorEntry red, yellow, green, blue, white;
    red.c1=255; red.c2=0; red.c3=0; red.c4=150;
    yellow.c1=255; yellow.c2=255; yellow.c3=0; yellow.c4=150;
    green.c1=0; green.c2=255; green.c3=0; green.c4=150;
    blue.c1=0; blue.c2=0; blue.c3=255; blue.c4=150;
    white.c1=255; white.c2=255; white.c3=255; white.c4=0;

    int brk0, brk1, brk2, brk3, brk4;

    double _maxValue = std::numeric_limits<double>::min();
    for(int i = scaledDataGrid.get_nRows() - 1;i >= 0;i--)
    {
        for (int j = 0;j < scaledDataGrid.get_nCols();j++)
        {
            if(scaledDataGrid(i,j) > _maxValue && scaledDataGrid(i,j) != get_noDataValue())
            {
                _maxValue = scaledDataGrid(i,j);
            }
        }
    }

    double _minValue = std::numeric_limits<double>::max();
    for(int i=nYSize-1;i>=0;i--)
        {
            for (int j=0;j<scaledDataGrid.get_nCols();j++)
            {
                if(scaledDataGrid(i,j) < _minValue && scaledDataGrid(i,j) != get_noDataValue() &&
                   scaledDataGrid(i,j)!=0)
                {
                    _minValue = scaledDataGrid(i,j);
                }
            }
        }

    brk0 = 0;
    brk1 = _minValue;
    brk2 = 0.2*(_maxValue-_minValue)+_minValue;
    brk4 = _maxValue;
    brk3 = (brk4+brk2)/2;

    poCT->SetColorEntry(brk0, &white);
    poCT->SetColorEntry(brk1, &blue);
    poCT->SetColorEntry(brk2, &green);
    poCT->SetColorEntry(brk3, &yellow);
    poCT->SetColorEntry(brk4, &red);

    int nStartIndex = brk1;
    int nEndIndex = brk2;
    const GDALColorEntry psStartColor1 = blue;
    const GDALColorEntry psEndColor1 = green;
    GDALCreateColorRamp(poCT, nStartIndex, &psStartColor1,  nEndIndex, &psEndColor1);
    nStartIndex = brk2;
    nEndIndex = brk3;
    const GDALColorEntry psStartColor2 = green;
    const GDALColorEntry psEndColor2 = yellow;
    GDALCreateColorRamp(poCT, nStartIndex, &psStartColor2,  nEndIndex, &psEndColor2);
    nStartIndex = brk3;
    nEndIndex = brk4;
    const GDALColorEntry psStartColor3 = yellow;
    const GDALColorEntry psEndColor3 = red;
    GDALCreateColorRamp(poCT, nStartIndex, &psStartColor3,  nEndIndex, &psEndColor3);

    //poBand->SetColorTable(poCT);

    /* -------------------------------------------------------------------- */
    /*  Create the optional legend                                          */
    /* -------------------------------------------------------------------- */

    if(writeLegend == true)
    {
        //make bitmap
	    int legendWidth = 180;
	    int legendHeight = int(legendWidth / 0.75);
	    BMP legend;

	    std::string legendStrings[6];
	    ostringstream os;

	    double maxxx = get_maxValue();
	    double minnn = raw_minValue;
        double _brk0 = 0;
        double _brk1 = raw_minValue;
        double _brk2 = 0.25*(raw_maxValue-raw_minValue)+raw_minValue;
        double _brk4 = raw_maxValue;
        double _brk3 = (_brk4+_brk2)/2;

	    for(int i = 0;i < 5;i++)
	    {
		    os << setiosflags(ios::fixed) << setiosflags(ios::showpoint) << setprecision(2);
		    if(i == 0)
			    os << maxxx;
            else if (i==1)
                os << (double)_brk3;
		    else if(i == 2)
			    os << (double)_brk2;
            else if(i == 3)
                os << minnn;
		    else if(i == 4)
			    os << "0.00";
            legendStrings[i] = os.str();
		    os.str("");
	    }

	    legend.SetSize(legendWidth,legendHeight);
	    legend.SetBitDepth(8);

	    //black legend
	    for(int i = 0;i < legendWidth;i++)
	    {
		    for(int j = 0;j < legendHeight;j++)
		    {
			    legend(i,j)->Alpha = 0;
			    legend(i,j)->Blue = 0;
			    legend(i,j)->Green = 0;
			    legend(i,j)->Red = 0;
		    }
        }

	    //for white text
	    RGBApixel w;
	    w.Red = 255;
	    w.Green = 255;
	    w.Blue = 255;
	    w.Alpha = 0;

	    RGBApixel colors[10];

	    //RGBApixel red, yellow, green, blue
	    colors[0].Red = 255;  //Red
	    colors[0].Green = 0;
	    colors[0].Blue = 0;
	    colors[0].Alpha = 0;

        colors[1].Red = 255;  //RRY
	    colors[1].Green = 85;
	    colors[1].Blue = 0;
	    colors[1].Alpha = 0;

	    colors[2].Red = 255;  //RYY
	    colors[2].Green = 170;
	    colors[2].Blue = 0;
	    colors[2].Alpha = 0;

	    colors[3].Red = 255;  //Yellow
	    colors[3].Green = 255;
	    colors[3].Blue = 0;
	    colors[3].Alpha = 0;

	    colors[4].Red = 170;  //YYG
	    colors[4].Green = 255;
	    colors[4].Blue = 0;
	    colors[4].Alpha = 0;

	    colors[5].Red = 85;  //YGG
	    colors[5].Green = 255;
	    colors[5].Blue = 0;
	    colors[5].Alpha = 0;

	    colors[6].Red = 0;  //Green
	    colors[6].Green = 255;
	    colors[6].Blue = 0;
	    colors[6].Alpha = 0;

	    colors[7].Red = 0;  //GGB
        colors[7].Green = 170;
        colors[7].Blue = 80;
        colors[7].Alpha = 0;

        colors[8].Red = 0;  //GBB
        colors[8].Green = 80;
        colors[8].Blue = 170;
        colors[8].Alpha = 0;

        colors[9].Red = 0;  //Blue
        colors[9].Green = 0;
        colors[9].Blue = 255;
        colors[9].Alpha = 0;

        int arrowLength = 30;	//pixels;
        int textHeight = 12;
        int titleTextHeight = int(1.2 * textHeight);
        int titleX, titleY;

        int x1, x2;
        double x;
        int y1, y2;
        double y;
        int textX;
        int textY;

        x = 0.05;
        y = 0.30;

        titleX = x * legendWidth;
        titleY = (y / 3) * legendHeight;

        PrintString(legend, legendTitle.c_str() , titleX, titleY-10, titleTextHeight, w);
        PrintString(legend, legendUnits.c_str() , titleX+5, titleY+15, titleTextHeight, w);

        x1 = int(legendWidth * x);
        x2 = x1 + arrowLength;
        y1 = int(legendHeight * y);
        y2 = y1;
        int k = -1;

        for(int i=0;i < 10;i++)
        {
            for(int j=0; j<20; j++)
            {
                x1 = int(legendWidth * (x+0.1));
                x2 = x1 + arrowLength;
                y1 = int(legendHeight * (y-0.03));
                y2 = y1;
                DrawLine(legend, x1, y1, x2, y2, colors[i]);
                y+=0.003;
            }
            if(i==0 || i==3 || i==6 || i==9)
            {
                k+=1;
                textX = x2 + 15;
                textY = y2 - int(textHeight * 0.5) - 8;
                PrintString(legend, legendStrings[k].c_str(), textX, textY, textHeight, w);
            }
        }
        legend.WriteToFile(scalarLegendFilename.c_str());
    }

    /* -------------------------------------------------------------------- */
    /*  close and reopen with GDALOpenShared and set color table        */
    /* -------------------------------------------------------------------- */

    AsciiGrid<T> poDS_grid(poDS, 1);
    poDS_grid.write_Grid("poDS_grid", 0);
    
    GDALDataset *srcDS;
    //reopen with GDALOpenShared() for GDALAutoCreateWarpedVRT()
    srcDS = (GDALDataset*)GDALOpenShared( "poDS_grid", GA_ReadOnly );
    if( srcDS == NULL ) {
        CPLDebug( "ascii_grid::ascii2png()", "cannot open poDS_grid");
    }

    GDALRasterBand *srcBand = srcDS->GetRasterBand(1);
    srcBand->SetColorTable(poCT);

    /* -------------------------------------------------------------------- */
    /*   Warp the image                                                     */
    /* -------------------------------------------------------------------- */

    OGRSpatialReference oSRS;
    char *pszSRS_WKT = NULL;

    const char* prj2 = (const char*)prjString.c_str();
    oSRS.importFromWkt((char **)&prj2);
    oSRS.exportToWkt(&pszSRS_WKT);

    char *pszDST_WKT = NULL;
    oSRS.importFromEPSG(4326);
    oSRS.exportToWkt(&pszDST_WKT);

    GDALDataset *wrpDS;

    wrpDS = (GDALDataset*)GDALAutoCreateWarpedVRT(srcDS, pszSRS_WKT, pszDST_WKT,
                                                   GRA_NearestNeighbour,
                                                   0.0, NULL);

    /* -------------------------------------------------------------------- */
    /*   Write the png                                                      */
    /* -------------------------------------------------------------------- */
    
    GDALDriver *poDstDriver = GetGDALDriverManager()->GetDriverByName("PNG");
    GDALDataset *poDstDS;
    
    CPLPushErrorHandler(CPLQuietErrorHandler); //silence PNG dirver data type error
    poDstDS = poDstDriver->CreateCopy(outFilename.c_str(), wrpDS, FALSE, NULL, NULL, NULL);
    CPLPopErrorHandler();

    //---for testing--------------------------------
    /*AsciiGrid<double> poDstDS_grid;
    GDAL2AsciiGrid(poDstDS, 1, poDstDS_grid);
    poDstDS_grid.write_Grid("poDstDS_grid", 2);
    
    AsciiGrid<double> wrpDS_grid;
    GDAL2AsciiGrid(wrpDS, 1, wrpDS_grid);
    wrpDS_grid.write_Grid("wrpDS_grid", 2);

    AsciiGrid<double> poDS2_grid;
    GDAL2AsciiGrid(poDS, 1, poDS2_grid);
    poDS2_grid.write_Grid("poDS2_grid", 2);*/

    
    /* -------------------------------------------------------------------- */
    /*  clean up                                                            */
    /* -------------------------------------------------------------------- */

    CPLFree(pszSRS_WKT);
    CPLFree(pszDST_WKT);
    
    delete [] padfScanline;

    GDALDestroyColorTable((GDALColorTableH) poCT);

    GDALClose((GDALDatasetH) poDS);
    GDALClose((GDALDatasetH) srcDS);
    if( poDstDS != NULL){
        GDALClose((GDALDatasetH) poDstDS);
    }
    GDALClose((GDALDatasetH) wrpDS);

    VSIUnlink("poDS_grid");
    VSIUnlink("poDS_grid.aux.xml");
    VSIUnlink("temp_fileout");

}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//OPERATORS

template <class T>
AsciiGrid<T> &AsciiGrid<T>::operator=(const AsciiGrid &A)
{
    if(&A != this)
    {
        data = A.data;
        cellSize = A.cellSize;
        xllCorner = A.xllCorner;
        yllCorner = A.yllCorner;
        prjString = A.prjString;

        if(A.sortedData == NULL)
        {
            delete[] sortedData;
            sortedData = NULL;
        }
        else{
            delete[] sortedData;

            sortedData = new T[data.get_numRows()*data.get_numCols()];
            for(int i = 0; i < data.get_numRows()*data.get_numCols(); i++)
                sortedData[i] = A.sortedData[i];
        }
    }
    return *this;
}

template <class T>
bool AsciiGrid<T>::operator==(AsciiGrid &A)
{

    #ifdef ASCII_GRID_DEBUG
        std::cout << "Checking Grid '= ='." << std::endl;
    #endif

    if(data.get_numCols() != A.data.get_numCols())
    {
        #ifdef ASCII_GRID_DEBUG
            std::cout << "Grids !=: nCols different." << std::endl;
        #endif
        return false;
    }
    else if(data.get_numRows() != A.data.get_numRows())
    {
        #ifdef ASCII_GRID_DEBUG
            std::cout << "Grids !=: nRows different." << std::endl;
        #endif
        return false;
    }
    if(cellSize != A.cellSize)
    {
        #ifdef ASCII_GRID_DEBUG
            std::cout << "Grids !=: cellSize different." << std::endl;
        #endif
        return false;
    }
    if(data.getNoDataValue() != A.data.getNoDataValue())
    {
        #ifdef ASCII_GRID_DEBUG
            std::cout << "Grids !=: noDataValue different." << std::endl;
        #endif
        return false;
    }
    if(xllCorner != A.xllCorner)
    {
        #ifdef ASCII_GRID_DEBUG
            std::cout << "Grids !=: xllCorner different." << std::endl;
        #endif
        return false;
    }
    if(yllCorner != A.yllCorner)
    {
        #ifdef ASCII_GRID_DEBUG
            std::cout << "Grids !=: yllCorner different." << std::endl;
        #endif
        return false;
    }
    if(data != A.data)
    {
        #ifdef ASCII_GRID_DEBUG
            std::cout << "Grids !=: values different." << std::endl;
        #endif
        return false;
    }
    return true;
}

template <class T>
bool AsciiGrid<T>::operator!=(AsciiGrid &A)
{
    return !(*this == A);
}

template <class T1>
std::ostream &operator<<(std::ostream &out, AsciiGrid<T1> &A)
{
    out << "ncols"
        << "\t"
        << A.get_nCols()
        << std::endl;
    out << "nrows"
        << "\t"
        << A.get_nRows()
        << std::endl;
    out << "xllcorner"
        << "\t"
        << setiosflags(std::ios::fixed)
        << std::setprecision(2)
        << A.xllCorner
        << std::endl;
    out << "yllcorner"
        << "\t"
        << setiosflags(std::ios::fixed)
        << std::setprecision(2)
        << A.yllCorner
        << std::endl;
    out << "cellsize"
        << "\t" << setiosflags(std::ios::fixed)
        << std::setprecision(2)
        << A.cellSize
        << std::endl;
    out << "NODATA_value"
        << "\t"
        << std::setprecision(2)
        << A.noDataValue
        << std::endl;

    for(int i = A.get_nRows() - 1;i >= 0;i--)
    {
        for (int j = 0;j < A.get_nCols();j++)
        {
            out << setiosflags(std::ios::fixed)
                << std::setprecision(2)
                << A(i,j)
                << "\t";
        }
        out << std::endl;
    }
    return out;
}

/**
 * Create a tiff for the ascii grid for debugging and other uses
 * This will create a grayscale tiff by default.
 *
 * @param outFilename output file to write
 * @param type rgb or grayscale
 *
 */

template <class T>
void AsciiGrid<T>::exportToTiff( std::string outFilename, tiffType type )
{
    GDALDataset *poDS;
    GDALDriver *tiffDriver = GetGDALDriverManager()->GetDriverByName( "GTiff" );
    char** papszOptions = NULL;
    papszOptions = CSLAddString( papszOptions, "PROFILE=BASELINE" );
    if( tiffDriver == NULL )
    return;

    poDS = tiffDriver->Create( outFilename.c_str(), get_nRows(), get_nCols(), 1,
                   GDT_Float64, papszOptions );

    int nXSize = poDS->GetRasterXSize();
    int nYSize = poDS->GetRasterYSize();

    GDALRasterBand *poBand = poDS->GetRasterBand( 1 );

    double *padfScanline;
    padfScanline = new double[nXSize];
    for( int i = nYSize - 1;i >= 0;i-- ) {
    for( int j = 0;j < nXSize;j++ )
        padfScanline[j] = (double)get_cellValue( nYSize - 1 - i, j );

    check(poBand->RasterIO( GF_Write, 0, i, nXSize, 1, padfScanline, nXSize, 1, GDT_Float64, 0, 0));
    }
    poBand->SetColorInterpretation( GCI_GrayIndex );
    GDALClose((GDALDatasetH) poDS );
}

template <class T>
bool AsciiGrid<T>::operator=(T m)
{
    data = m;

    return true;
}

template <class T>
T& AsciiGrid<T>::operator()(unsigned m, unsigned n)
{
    return data(m,n);
}

template <class T>
const T& AsciiGrid<T>::operator() (const unsigned m, const unsigned n) const
{
    return data(m,n);
}


template <class T>
AsciiGrid<T> AsciiGrid<T>::operator+(T m)
{
    AsciiGrid<T>A(data.get_numCols(),data.get_numRows(),xllCorner,yllCorner,cellSize,data.getNoDataValue(),data.getNoDataValue(),prjString);

    for(int i=0; i<data.get_numRows(); i++)
    {
        for(int j=0; j<data.get_numCols(); j++)
        {
            if(data(i,j)==data.getNoDataValue())
            {
                A.data(i,j) = A.data.getNoDataValue();
                continue;
            }
            A.data(i,j) = data(i,j) + m;
        }
    }

    return A;
}

template <class T>
AsciiGrid<T> AsciiGrid<T>::operator-(T m)
{
    AsciiGrid<T>A(data.get_numCols(),data.get_numRows(),xllCorner,yllCorner,cellSize,data.getNoDataValue(),data.getNoDataValue(),prjString);

    for(int i=0; i<data.get_numRows(); i++)
    {
        for(int j=0; j<data.get_numCols(); j++)
        {
            if(data(i,j)==data.getNoDataValue())
            {
                data(i,j) = A.data.getNoDataValue();
                continue;
            }
            A.data(i,j) = data(i,j) - m;
        }
    }

    return A;
}

template <class T>
AsciiGrid<T> AsciiGrid<T>::operator/(T m)
{
    AsciiGrid<T>A(data.get_numCols(),data.get_numRows(),xllCorner,yllCorner,cellSize,data.getNoDataValue(),data.getNoDataValue(),prjString);

    for(int i=0; i<data.get_numRows(); i++)
    {
        for(int j=0; j<data.get_numCols(); j++)
        {
            if(data(i,j)==data.getNoDataValue())
            {
                A.data(i,j) = A.data.getNoDataValue();
                continue;
            }
            A.data(i,j) = data(i,j) / m;
        }
    }

    return A;
}

template <class T>
AsciiGrid<T> AsciiGrid<T>::operator*(T m)
{
    AsciiGrid<T>A(data.get_numCols(),data.get_numRows(),xllCorner,yllCorner,cellSize,data.getNoDataValue(),data.getNoDataValue(),prjString);

    for(int i=0; i<data.get_numRows(); i++)
    {
        for(int j=0; j<data.get_numCols(); j++)
        {
            if(data(i,j)==data.getNoDataValue())
            {
                A.data(i,j) = A.data.getNoDataValue();
                continue;
            }
            A.data(i,j) = data(i,j) * m;
        }
    }

    return A;
}

template <class T>
bool AsciiGrid<T>::operator+=(T m)
{
    for(int i=0; i<data.get_numRows(); i++)
    {
        for(int j=0; j<data.get_numCols(); j++)
        {
            if(data(i,j)==data.getNoDataValue())
                continue;
            data(i,j) += m;
        }
    }

    return true;
}

template <class T>
bool AsciiGrid<T>::operator-=(T m)
{
    for(int i=0; i<data.get_numRows(); i++)
    {
        for(int j=0; j<data.get_numCols(); j++)
        {
            if(data(i,j)==data.getNoDataValue())
                continue;
            data(i,j) -= m;
        }
    }

    return true;
}

template <class T>
bool AsciiGrid<T>::operator/=(T m)
{
    for(int i=0; i<data.get_numRows(); i++)
    {
        for(int j=0; j<data.get_numCols(); j++)
        {
            if(data(i,j)==data.getNoDataValue())
                continue;
            data(i,j) /= m;
        }
    }

    return true;
}

template <class T>
bool AsciiGrid<T>::operator*=(T m)
{
    for(int i=0; i<data.get_numRows(); i++)
    {
        for(int j=0; j<data.get_numCols(); j++)
        {
            if(data(i,j)==data.getNoDataValue())
                continue;
            data(i,j) *= m;
        }
    }

    return true;
}

template <class T>
AsciiGrid<T> AsciiGrid<T>::operator+(AsciiGrid<T> &A)
{
    AsciiGrid<T>B(A);
    for(int i=0; i<data.get_numRows(); i++)
    {
        for(int j=0; j<data.get_numCols(); j++)
        {
            if(data(i,j)==data.getNoDataValue() || A.data(i,j)==A.data.getNoDataValue())
            {
                B.data(i,j) = B.data.getNoDataValue();
                continue;
            }
            B.data(i,j) = data(i,j) + A.data(i,j);
        }
    }

    return B;
}

template <class T>
AsciiGrid<T> AsciiGrid<T>::operator-(AsciiGrid<T> &A)
{
    AsciiGrid<T>B(A);
    for(int i=0; i<data.get_numRows(); i++)
    {
        for(int j=0; j<data.get_numCols(); j++)
        {
            if(data(i,j)==data.getNoDataValue() || A.data(i,j)==A.data.getNoDataValue())
            {
                B.data(i,j) = B.data.getNoDataValue();
                continue;
            }
            B.data(i,j) = data(i,j) - A.data(i,j);
        }
    }

    return B;
}

template <class T>
AsciiGrid<T> AsciiGrid<T>::operator/(AsciiGrid<T> &A)
{
    AsciiGrid<T>B(A);
    for(int i=0; i<data.get_numRows(); i++)
    {
        for(int j=0; j<data.get_numCols(); j++)
        {
            if(data(i,j)==data.getNoDataValue() || A.data(i,j)==A.data.getNoDataValue())
            {
                B.data(i,j) = B.data.getNoDataValue();
                continue;
            }
            B.data(i,j) = data(i,j) / A.data(i,j);
        }
    }

    return B;
}

template <class T>
AsciiGrid<T> AsciiGrid<T>::operator*(AsciiGrid<T> &A)
{
    AsciiGrid<T>B(A);
    for(int i=0; i<data.get_numRows(); i++)
    {
        for(int j=0; j<data.get_numCols(); j++)
        {
            if(data(i,j)==data.getNoDataValue() || A.data(i,j)==A.data.getNoDataValue())
            {
                B.data(i,j) = B.data.getNoDataValue();
                continue;
            }
            B.data(i,j) = data(i,j) * A.data(i,j);
        }
    }

    return B;
}

template <class T>
bool AsciiGrid<T>::operator+=(AsciiGrid<T> &A)
{
    for(int i=0; i<data.get_numRows(); i++)
    {
        for(int j=0; j<data.get_numCols(); j++)
        {
            if(data(i,j)==data.getNoDataValue() || A.data(i,j)==A.data.getNoDataValue())
                continue;
            data(i,j) += A.data(i,j);
        }
    }

    return true;
}

template <class T>
bool AsciiGrid<T>::operator-=(AsciiGrid<T> &A)
{
    for(int i=0; i<data.get_numRows(); i++)
    {
        for(int j=0; j<data.get_numCols(); j++)
        {
            if(data(i,j)==data.getNoDataValue() || A.data(i,j)==A.data.getNoDataValue())
                continue;
            data(i,j) -= A.data(i,j);
        }
    }

    return true;
}

template <class T>
bool AsciiGrid<T>::operator/=(AsciiGrid<T> &A)
{
    for(int i=0; i<data.get_numRows(); i++)
    {
        for(int j=0; j<data.get_numCols(); j++)
        {
            if(data(i,j)==data.getNoDataValue() || A.data(i,j)==A.data.getNoDataValue())
                continue;
            data(i,j) /= A.data(i,j);
        }
    }

    return true;
}

template <class T>
bool AsciiGrid<T>::operator*=(AsciiGrid<T> &A)
{
    for(int i=0; i<data.get_numRows(); i++)
    {
        for(int j=0; j<data.get_numCols(); j++)
        {
            if(data(i,j)==data.getNoDataValue() || A.data(i,j)==A.data.getNoDataValue())
                continue;
            data(i,j) *= A.data(i,j);
        }
    }

    return true;
}

//--- these functions convert the grid to EPSG:4326 (lat/lon) before writing

template <class T>
void AsciiGrid<T>::write_4326_Grid (std::string& filename, int precision, void (AsciiGrid<T>::*write_grid)(std::string,int))
{
    GDALDatasetH hSrcDS = ascii2GDAL();
    if (hSrcDS) {
        const char *pszSrcWKT = GDALGetProjectionRef( hSrcDS );
        if (pszSrcWKT && strlen(pszSrcWKT) > 0) {
            OGRSpatialReference oSRS;
            oSRS.SetWellKnownGeogCS( "EPSG:4326" );

            char* pszDstWKT = NULL;
            oSRS.exportToWkt( &pszDstWKT );

            GDALDatasetH hTmpDS = GDALAutoCreateWarpedVRT(hSrcDS, pszSrcWKT, pszDstWKT, GRA_NearestNeighbour, 1.0, NULL);
            if (hTmpDS) {
                AsciiGrid<T> geoAsciiGrid( (GDALDataset*) hTmpDS, 1);
                
                if (!geoAsciiGrid.crop_noData( 10)){
                    cerr << "failed to crop EPSG:4326 grid to defined data\n";
                }
                

                (geoAsciiGrid.*(write_grid))( filename, precision);

                GDALClose(hTmpDS);

            } else {
                cerr << "failed to warp " << filename << "to EPSG:4326 (geo)\n";
            }
        }

        GDALClose(hSrcDS);

    } else {
        cerr << "failed to convert AsciiGrid to GDAL data set\n";
    }
}

template <class T>
void AsciiGrid<T>::write_ascii_4326_Grid (std::string filename, int precision) 
{
    write_4326_Grid(filename, precision, &AsciiGrid<T>::write_Grid);
}

template <class T>
void AsciiGrid<T>::write_json_4326_Grid (std::string filename, int precision) 
{
    write_4326_Grid(filename, precision, &AsciiGrid<T>::write_json_Grid);
}

template<class T>
bool AsciiGrid<T>::crop_noData (int noDataThreshold)
{
    int minRow, maxRow, minCol, maxCol;
    if (data.get_dataBoundaries( noDataThreshold, minRow, maxRow, minCol, maxCol)) {
        if (minRow > 0 || minCol > 0 || maxRow < data.get_numRows()-1 || maxCol < data.get_numCols()-1) {
            xllCorner += minCol * cellSize;
            yllCorner += (data.get_numRows()-1 - maxRow) * cellSize;
            data.crop( minRow, maxRow, minCol, maxCol); // note this crops in-place            
        }
        return true;
    } else {
        return false;
    }
}

//--- template instantiations

template class AsciiGrid<double>;
template class AsciiGrid<int>;
template class AsciiGrid<short>;
