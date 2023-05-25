
#include "Array2D.h"

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                          Constructors
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


/**
*@brief default constructor
*/
template<typename T>
Array2D<T>::Array2D()
{
    this->setMatrix(0,0, -9999.0);
}


/**
*@brief constructor that sets number of rows and columns
*/
template<typename T>
Array2D<T>::Array2D(unsigned nRows, unsigned nCols)
{
    if(!matrix.empty())
    {
        matrix.clear(); //Calls the destructor on all elements of matrix if not empty
        std::vector<T>(matrix).swap(matrix); //Swap trick to free up memory, may not be needed
    }

    rows = nRows;
    cols = nCols;

    matrix.resize(rows * cols); //Reserves size needed for matrix
}


/**
*@brief constructor that sets number of rows, columns, and no data value
*/
template<typename T>
Array2D<T>::Array2D(unsigned nRows, unsigned nCols, T noDataVal)
{
    noDataValue = noDataVal;

    if(!matrix.empty())
    {
        matrix.clear(); //Calls the destructor on all elements of matrix if not empty
        std::vector<T>(matrix).swap(matrix); //Swap trick to free up memory, may not be needed
    }

    rows = nRows;
    cols = nCols;

    matrix.resize(rows * cols); //Reserves exact size needed for matrix
    matrix.assign(matrix.size(), noDataValue);
}


/**
*@brief constructor that sets this Array2D object equal to another
*/
template<typename T>
Array2D<T>::Array2D(const Array2D& A)
{
    if(&A != this)
        {
            cols = A.cols;
            rows = A.rows;
            noDataValue = A.noDataValue;

            matrix = A.matrix;
        }
}


/**
*@brief default destructor
*/
template<typename T>
Array2D<T>::~Array2D()
{

}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                          Functions
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


/**
*@param m sets number of rows to m
*/
template<typename T>
void Array2D<T>::set_numRows(unsigned int m) 
{
    rows = m;
}

/**
*@param n sets number of columns to n
*/
template<typename T>
void Array2D<T>::set_numCols(unsigned int n) 
{
    cols = n;
}

/**
*@brief resizes the matrix and fills empty space with no data value if needed
*@param nRows rows in matrix
*@param nCols columns in matrix
*@param noDataVal no data value for matrix
*/
template<typename T>
void Array2D<T>::setMatrix(unsigned nRows, unsigned nCols, T noDataVal)
{
    rows = nRows;
    cols = nCols;
    noDataValue = noDataVal;
    matrix.resize(cols*rows, noDataValue);
}

/**
*@brief resizes the matrix, sets the no data value, and fills empty space with default value if needed
*@param nRows rows in matrix
*@param nCols columns in matrix
*@param noDataVal no data value for matrix
*@param defaultValue default value for empty cells in matrix
*/
template<typename T>
void Array2D<T>::setMatrix(unsigned nRows, unsigned nCols, T noDataVal, T defaultValue)
{
    rows = nRows;
    cols = nCols;
    noDataValue = noDataVal;
    matrix.resize(cols*rows, defaultValue);
}

/**
*@brief check if any no data values are present
*@return true if contains no data value, false otherwise
*/
template<typename T>
bool Array2D<T>::hasNoDataValues() const
{
    if(std::find(matrix.begin(), matrix.end(), noDataValue) != matrix.end()) {
        return true;
    } else {
        return false;
    }

}

/**
*@brief sets the no data value
*@param nDV no data value
*/
template<typename T>
void Array2D<T>::setNoDataValue(T nDV)
{
    noDataValue = nDV;
}


/**
*@brief return max element of template type contained within matrix
*@return returns the max template value
*/
template<typename T>
T Array2D<T>::max() const
{
    T max;
    max = std::numeric_limits<double>::min();

    for(int i = rows - 1;i >= 0;i--)
    {
        for (int j = 0; j < cols; j++)
        {
            if(matrix[i * cols + j] > max && matrix[i * cols + j] != noDataValue)
            {
                max = matrix[i * cols + j];
            }
        }
    }

    return max;
}

/**
*@brief return min element of template type contained within matrix
*@return returns the min template value
*/
template<typename T>
T Array2D<T>::min() const
{
    T min;
    min = std::numeric_limits<double>::max();

    for(int i = rows - 1;i >= 0;i--)
    {
        for (int j = 0; j < cols; j++)
        {
            if(matrix[i * cols + j] < min && matrix[i * cols + j] != noDataValue)
            {
                min = matrix[i * cols + j];
            }
        }
    }

    return min;
}

/**
*@brief return mean element of template type contained within matrix
*@return returns the mean template value
*/
template<typename T>
double Array2D<T>::mean() const
{
    double mean = 0;
    T sum = 0;
    for(int i = rows - 1; i >= 0; i--)
    {
        for (int j = 0; j < cols; j++)
        {
            sum += matrix[i * cols + j];
        }
    }

    mean = sum / matrix.size();
    return mean;
}

/**
*@brief uses quicksort method to sort data within matrix
*@return returns a pointer to the new sorted data array
*/
template<typename T>
T* Array2D<T>::sortData()
{
    T* sorted = new T[matrix.size()];
    for(int i=0; i<matrix.size(); i++)
    {
        sorted[i] = matrix[i];
    }
    std::sort(sorted, sorted + matrix.size());
    return sorted;
}

/**
*@brief dumps all matrix data to stdout
*/
template<typename T>
void Array2D<T>::dumpMatrix()
{
    for(int x=rows-1; x>=0; x--)
    {
        for(int y=0; y<cols; y++)
        {
            std::cout << matrix[x * cols + y];
        }
        std::cout << "\n";
    }
}

/**
*@brief dumps all matrix data into a file
*@param path path to the file to dump the data into
*/
template<typename T>
void Array2D<T>::dumpMatrix(std::string path)
{
    std::ofstream file;
    file.open(path.c_str(), std::ios::out | std::ios::trunc);
    if(file.is_open())
    {
        for(int x=rows-1; x>=0; x--)
        {
            for(int y=0; y<cols; y++)
            {
                file << matrix[x * cols + y];
            }
            std::cout << "\n";
        }
        std::cout << "File written";
        file.close();
    }
    else std::cout << "Unable to open file.";
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                          Operators
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

/**
*@brief overloads equals operator to set all matrix elements equal to a single template element
*@param data template element to set all elements to
*@return returns a pointer to self
*/
template<typename T>
Array2D<T>& Array2D<T>::operator=(const T data)
{
    matrix.assign(matrix.size(), data);

    return *this;
}

/**
*@brief overloads equals operator to set calling Array2D object equal to another
*@param rhs Array2D object to set equal to
*@return returns a pointer to self
*/
template<typename T>
Array2D<T>& Array2D<T>::operator=(const Array2D& rhs)
{
    this->cols = rhs.cols;
    this->rows = rhs.rows;
    this->noDataValue = rhs.noDataValue;
    matrix = rhs.matrix;

    return *this;
}

/**
*@brief overloaded equal to operator to determine if 2 Array2D objects are identical
*@param rhs Array2D object to compare to
*@return return true if matrix obects within are equal, false otherwise
*/
template<typename T>
bool Array2D<T>::operator==(const Array2D& rhs)
{
    if(this->cols == rhs.cols && this->rows == rhs.rows && this->noDataValue == rhs.noDataValue && matrix == rhs.matrix)
        return true;
    else
        return false;
}

/**
*@brief overloaded not equal operator to determine if 2 Array2D objects are identical
*@param rhs Array2D object to compare to
*@return return true if matrix obects within are not equal, false otherwise
*/
template<typename T>
bool Array2D<T>::operator!=(const Array2D& rhs)
{
    if(this->cols == rhs.cols && this->rows == rhs.rows && this->noDataValue == rhs.noDataValue && matrix == rhs.matrix)
        return false;
    else
        return true;
}

template <typename T>
int Array2D<T>::count_consecutive_noData (int row) 
{
    if (row < 0 || row >= rows) throw std::runtime_error("invalid row index");

    int i0 = row * cols;
    int nNoData = 0;
    int n = 0;

    for (int i=0; i<cols; i++) {
        if (matrix[i0+i] == noDataValue) {
            n++;
            if (n > nNoData) nNoData = n;
        } else {
            n = 0;
        }
    }

    return nNoData;
}

template <typename T>
int Array2D<T>::count_leading_noData (int row) 
{
    if (row < 0 || row >= rows) throw std::runtime_error("invalid row index");

    int i0 = row * cols;
    int nNoData = 0;

    for (int i=0; (i<cols) && (matrix[i0+i] == noDataValue); i++) nNoData++;

    return nNoData;
}

template <typename T>
int Array2D<T>::count_trailing_noData (int row) 
{
    if (row < 0 || row >= rows) throw std::runtime_error("invalid row index");

    int i0 = row * cols;
    int nNoData = 0;

    for (int i=cols-1; (i>=0) && (matrix[i0+i] == noDataValue); i--) nNoData++;

    return nNoData;
}

/**
 * @brief get min/max row and column indices that contain grid with defined data values
 * 
 * @param maxNoData number of consecutive noData values that cause leading and trailing rows to be skipped
 * @return true if grid boundaries for defined data values exist, false if interspersed noData values.
 * If true minRow, maxRow, minCol and maxCol contain valid row/col index values defining the sub-grid
 */
template <typename T>
bool Array2D<T>::get_dataBoundaries(int maxNoData, int& minRow, int& maxRow, int& minCol, int& maxCol)
{
    minRow = 0;
    maxRow = rows-1;
    minCol = 0;
    maxCol = cols-1;

    for (int i=0; i<rows && count_consecutive_noData(i) >=maxNoData; i++) {
        minRow++;
    }

    if (minRow < maxRow) {
        for (int i=maxRow; i>=0 && count_consecutive_noData(i) >= maxNoData; i--) {
            maxRow--;
        }

        for (int i=minRow; i<=maxRow; i++) {
            minCol =  std::max<T>( count_leading_noData(i), minCol);

            if (minCol < cols-1) {
                maxCol = std::min<T>( cols-1 - count_trailing_noData(i), maxCol);

                int j0 = i*cols;
                for (int j=minCol; j<=maxCol; j++) {
                    if (matrix[j0 + j] == noDataValue) {
                        return false; // interior noData value
                    }
                }
            } else {
                return false;
            }
        }
    } else {
        return false;
    }

    return true;
}

/**
 * crop (shrink) in place.
 * we use explicit nested loops instead of memcpy so that we can shrink in-place
 * (memcpy does not handle overlapping src/dst).
 * This foregoes some speed gains for (potentially) saving a lot of memory
 */
template <typename T>
void Array2D<T>::crop (int minRow, int maxRow, int minCol, int maxCol){
    assert( minRow >= 0 && minRow <rows);
    assert( maxRow >= minRow && maxRow <rows);
    assert( minCol >= 0 && minCol < cols);
    assert( maxCol >= minCol && maxCol <cols);

    T* dst = matrix.data();
    T* src = matrix.data() + (minRow*cols);
    int k = 0;

    for (int i=minRow; i<=maxRow; i++) {
        for (int j=minCol; j<=maxCol; j++) {
            dst[k++] = src[j];
        }
        src += cols;
    }
    cols = maxCol - minCol + 1;
    rows = maxRow - minRow + 1;

    matrix.resize(k);
}

//--- template instantiations

template class Array2D<double>;
template class Array2D<int>;
template class Array2D<short>;
