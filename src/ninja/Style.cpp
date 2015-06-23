/******************************************************************************
 *
 * $Id$
 *
 * Project:  WindNinja
 * Purpose:  Sub-class for KmlVector
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

#include "Style.h"

Style::Style()
{}
Style::Style(std::string l, int a, int b, int g, int r, double w) : LineStyle(a, b, g, r, w)
{
	label = l;
}

Style::~Style()
{}

bool Style::printStyle()
{
	std::cout << "<Style id=\"" << label << "\">" << std::endl;
	printLineStyle();
	std::cout << "</Style>" << std::endl << std::endl;
	return true;
}

bool Style::writeStyle(VSILFILE *fileOut)
{
	VSIFPrintfL(fileOut, "<Style id=\"%s\">\n", label.c_str());
	writeLineStyle(fileOut);
	VSIFPrintfL(fileOut, "</Style>\n");
	return true;
}

std::string Style::asOGRStyleString()
{
    return asOGRLineStyle(); 
}





	/*<Style id="blue">
	<LineStyle>
	<color>ffff0000</color>
	<width>1</width>
	</LineStyle>
	</Style>*/
