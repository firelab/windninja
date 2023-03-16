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
#include <string.h>
#include <vector>
#include <limits>
#include <algorithm>
#include <fstream>
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
    inline unsigned size() const;
    bool hasNoDataValues() const;
    void setNoDataValue(T nDV);
    inline T getNoDataValue() const;
    T max() const;
    T min() const;
    double mean() const;
    T* sortData();
    void dumpMatrix();
    void dumpMatrix(std::string file);

    void crop(int minRow, int maxRow, int minCol, int maxCol);

    inline T& operator()(unsigned i, unsigned j);
    inline const T& operator()(unsigned const i, unsigned const j) const;

    Array2D& operator=(const T data);
    Array2D& operator=(const Array2D& rhs);
    bool operator==(const Array2D& rhs);
    bool operator!=(const Array2D& rhs);

    int count_consecutive_noData (int row);
    int count_leading_noData (int row);
    int count_trailing_noData (int row);
    bool get_dataBoundaries(int maxNoData, int& minRow, int& maxRow, int& minCol, int& maxCol);
    
    inline T* data() {return matrix.data();}

  private:
    std::vector<T> matrix;
    unsigned int rows;
    unsigned int cols;
    T noDataValue;
};

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
*@brief get size of Array2D object, which is just size of matrix
*@return number of elements in matrix
*/
template<typename T>
inline unsigned Array2D<T>::size() const
{
    return matrix.size();
}

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
*@brief overloads paranthesis operator to allow setting matrix elements via return value reference to elements
*@param row row of element
*@param col column of element
*@return returns the element at the row and column and a reference to it for setting
*/
template<typename T>
inline T& Array2D<T>::operator()(unsigned row, unsigned col)
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
inline const T& Array2D<T>::operator()(unsigned row, unsigned col) const
{
  assert (row <= rows || col <= cols);
  return matrix[row * cols + col];
}

#endif /* ARRAY2D_H */
