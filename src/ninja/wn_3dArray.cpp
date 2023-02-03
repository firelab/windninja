/******************************************************************************
 *
 * $Id$
 *
 * Project:  WindNinja
 * Purpose:  Class for storing a 3D array
 * Author:   Jason Forthofer <jforthofer@gmail.com>
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

#include "wn_3dArray.h"

wn_3dArray::wn_3dArray()
    : rows_ (0)
    , cols_ (0)
    , layers_ (0)
{
	data_ = NULL;
}

wn_3dArray::wn_3dArray(int rows, int cols, int layers)
	: rows_ (rows)
	, cols_ (cols)
	, layers_ (layers)
{
#ifdef NINJA_DEBUG
	if (rows <= 0 || cols <= 0 || layers <= 0)
		throw std::range_error("Rows, columns, or layers are less than or equal to 0 in wn_3dArray::wn_3dArray(int rows, int cols, int layers).");
#endif
	if(rows>0 && cols>0 && layers>0)
	    data_ = new double[rows * cols * layers];
}

wn_3dArray::~wn_3dArray()
{
	delete[] data_;
}

wn_3dArray::wn_3dArray(wn_3dArray const& m)	// Copy constructor
{
	allocate(m.rows_, m.cols_, m.layers_);

	for(int i=0; i<rows_*cols_*layers_; i++)
		data_[i] = m(i);
}
		
wn_3dArray& wn_3dArray::operator= (wn_3dArray const& m)	// Assignment operator
{
	if(&m != this)
	{
	    allocate(m.rows_, m.cols_, m.layers_);

		for(int i=0; i<rows_*cols_*layers_; i++)
			data_[i] = m(i);
	}
	return *this;
}

void wn_3dArray::allocate(int rows, int cols, int layers)
{
#ifdef NINJA_DEBUG
    if (rows <= 0 || cols <= 0 || layers <= 0)
        throw std::range_error("Rows, columns, or layers are less than or equal to 0 in wn_3dArray::allocate(int rows, int cols, int layers).");
#endif
    delete[] data_;
    data_ = NULL;

    rows_ = rows;
    cols_ = cols;
    layers_ = layers;

    if(rows>0 && cols>0 && layers>0)
        data_ = new double[rows * cols * layers];
}

void wn_3dArray::deallocate()
{
	if(data_ != NULL)
			delete[] data_;
	data_ = NULL;

	rows_ = 0;
	cols_ = 0;
	layers_ = 0;
}

double& wn_3dArray::operator() (int row, int col, int layer)
{
#ifdef NINJA_DEBUG
	if(data_ == NULL)
		throw std::domain_error("No memory allocated for \"data_\" in wn3dArray.");
	if (row >= rows_ || col >= cols_ || layer >= layers_ || row < 0 || col < 0 || layer < 0)
		throw std::range_error("Rows, columns, or layers are are out of range in wn_3dArray::operator()(int row, int col, int layer).");
	
#endif
	return data_[layer*rows_*cols_ + cols_*row + col];
}

double wn_3dArray::operator() (int row, int col, int layer) const
{
#ifdef NINJA_DEBUG
	if(data_ == NULL)
		throw std::domain_error("No memory allocated for \"data_\" in wn3dArray.");
	if (row >= rows_ || col >= cols_ || layer >= layers_ || row < 0 || col < 0 || layer < 0)
		throw std::range_error("Rows, columns, or layers are are out of range in wn_3dArray::operator()(int row, int col, int layer) const.");
#endif
	return data_[layer*rows_*cols_ + cols_*row + col];
}

double& wn_3dArray::operator() (int num)
{
#ifdef NINJA_DEBUG
	if(data_ == NULL)
		throw std::domain_error("No memory allocated for \"data_\" in wn3dArray.");
	if (num >= rows_*cols_*layers_)
		throw std::range_error("Index is out of range in wn_3dArray::operator()(int num).");
#endif
	return data_[num];
}

double wn_3dArray::operator() (int num) const
{
#ifdef NINJA_DEBUG
	if(data_ == NULL)
		throw std::domain_error("No memory allocated for \"data_\" in wn3dArray.");
	if (num >= rows_*cols_*layers_)
		throw std::range_error("Index is out of range in wn_3dArray::operator()(int num).");
#endif
	return data_[num];
}
