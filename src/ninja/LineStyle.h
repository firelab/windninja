/******************************************************************************
 *
 * $Id$
 *
 * Project:  WindNinja
 * Purpose:  KmlVector sub-class
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

#ifndef LINESTYLE_H
#define LINESTYLE_H

#include <sstream>
#include <string>
#include <iostream>
#include <fstream>
#include "gdal_priv.h"
	
class LineStyle
{
public:
	LineStyle();
	LineStyle(int a, int b, int g, int r, double w);
	virtual ~LineStyle();

	bool setHexColor();
	bool setHexColorRGBA();

	bool printLineStyle();
	bool writeLineStyle(VSILFILE *fileOut);
    std::string  asOGRLineStyle();

private:
	std::string hexColor;
	std::string colorName;
	int alpha;
	int blue;
	int green;
	int red;
	double width;
};

#endif	//LINESTYLE_H
