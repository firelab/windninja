/******************************************************************************
 *
 * $Id: Array2D.h 1307 2012-05-22 21:30:42Z cody.posey $
 *
 * Project:  WindNinja
 * Purpose:  Class for storing and operating on 2D Arrays built from vectors
 * Author:   Cody Posey <cody.posey85@gmail.com>
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

#ifndef ARRAY2D_H
#define ARRAY2D_H

#include "Array2D.h"
#include <assert.h>
#include <vector>
#include <limits>
#include <algorithm>
#include <iostream>
/**
 * @class Array2D
 * @brief Container for any template data type that uses a 1-dimensional vector to store 2-dimensional data using [row * numberColumns + columns] zero based format.
 *
 */
template<typename T>
class Array2D
{
  public:
    Array2D();
    Array2D(unsigned nRows, unsigned nCols);
    Array2D(unsigned nRows, unsigned nCols, T noDataVal);
    Array2D(const Array2D& A);
    ~Array2D();

    inline int get_numRows() const;
    inline int get_numCols() const;
    void set_numRows(unsigned int m);
    void set_numCols(unsigned int n);
    void setMatrix(unsigned nRows, unsigned nCols, T noDataVal);
    void setMatrix(unsigned nRows, unsigned nCols, T noDataVal, T defaultValue);
    unsigned size() const;
    bool hasNoDataValues() const;
    void setNoDataValue(T nDV);
    T getNoDataValue() const;
    T max() const;
    T min() const;
    double mean() const;
    T* sortData();
    void dumpMatrix();
    void dumpMatrix(std::string file);

    T& operator()(unsigned i, unsigned j);
    const T& operator()(unsigned const i, unsigned const j) const;
    Array2D& operator=(const T data);
    Array2D& operator=(const Array2D& rhs);
    bool operator==(const Array2D& rhs);
    bool operator!=(const Array2D& rhs);
    
  private:
    std::vector<T> matrix;
    unsigned int rows;
    unsigned int cols;
    T noDataValue;
};

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
*@return number of rows in array
*/
template<typename T>
inline int Array2D<T>::get_numRows() const
{
    return rows;
}

/**
*@return number of columns in array
*/
template<typename T>
inline int Array2D<T>::get_numCols() const
{
    return cols;
}

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
*@brief get size of Array2D object, which is just size of matrix
*@return number of elements in matrix
*/
template<typename T>
inline unsigned Array2D<T>::size() const
{
    return matrix.size();
}

/**
*@brief check if any no data values are present
*@return true if contains no data value, false otherwise
*/
template<typename T>
inline bool Array2D<T>::hasNoDataValues() const
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
inline void Array2D<T>::setNoDataValue(T nDV)
{
    noDataValue = nDV;
}

/**
*@brief returns no data value
*@return current no data value
*/
template<typename T>
inline T Array2D<T>::getNoDataValue() const
{
    return noDataValue;
}

/**
*@brief return max element of template type contained within matrix
*@return returns the max template value
*/
template<typename T>
inline T Array2D<T>::max() const
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
inline T Array2D<T>::min() const
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
inline double Array2D<T>::mean() const
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
*@brief overloads paranthesis operator to allow setting matrix elements via return value reference to elements
*@param row row of element
*@param col column of element
*@return returns the element at the row and column and a reference to it for setting
*/
template<typename T>
T& Array2D<T>::operator()(unsigned row, unsigned col)
{
    assert (row <= rows || col <= cols);
    return matrix[row * cols + col];
}

/**
*@brief constant overloaded parenthesis operator for returning matrix elements without modifying
*@param row row of element
*@param col column of element
*@return returns the element at the row and column
*/
template<typename T>
const T& Array2D<T>::operator()(unsigned row, unsigned col) const
{
  assert (row <= rows || col <= cols);
  return matrix[row * cols + col];
}

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

#endif /* ARRAY2D_H */
