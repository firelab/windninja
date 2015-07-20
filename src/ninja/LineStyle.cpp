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

#include "LineStyle.h"


LineStyle::LineStyle()
{}

LineStyle::LineStyle(int a, int b, int g, int r, double w)
{
	if(a > 255 || a < 0)
		alpha = 0;
	else
        alpha = a;
	if(b > 255 || b < 0)
		blue = 0;
	else
        blue = b;
	if(g > 255 || g < 0)
		green = 0;
	else
        green = g;
	if(r > 255 || r < 0)
		red = 0;
	else
        red = r;
	if(w < 0)
		width = 1.0;
	else
		width = w;

	setHexColor();
}

LineStyle::~LineStyle()
{}

bool LineStyle::setHexColor()
{
    std::ostringstream a, b, g, r;

	a << std::hex << alpha << std::flush;
	hexColor = a.str();
	if(alpha == 0)
		hexColor.append("0");

	b << std::hex << blue << std::flush;
	hexColor.append(b.str());
	if(blue == 0)
		hexColor.append("0");

	g << std::hex << green << std::flush;
	hexColor.append(g.str());
	if(green == 0)
		hexColor.append("0");
	
	r << std::hex << red << std::flush;
	hexColor.append(r.str());
	if(red == 0)
		hexColor.append("0");
	return true;
}

bool LineStyle::setHexColorRGBA()
{
	std::ostringstream a, b, g, r;
	hexColor.clear();

    r << std::hex << red << std::flush;
	hexColor = r.str();
	if(red == 0)
		hexColor.append("0");

    g << std::hex << green << std::flush;
	hexColor.append(g.str());
	if(green == 0)
		hexColor.append("0");
	

    b << std::hex << blue << std::flush;
	hexColor.append(b.str());
	if(blue == 0)
		hexColor.append("0");

    
	a << std::hex << alpha << std::flush;
	hexColor.append( a.str() );
	if(alpha == 0)
		hexColor.append("0");

    	
	return true;
   
}

std::string LineStyle::asOGRLineStyle()
{
    std::ostringstream retstr; 
    setHexColorRGBA();
    retstr <<  "PEN(c:#" << hexColor << ",w:" << width << "px)" << std::flush; 
    return retstr.str();
}

bool LineStyle::printLineStyle()
{
	std::cout << "\t<LineStyle>" << std::endl;
	std::cout << "\t\t<color>" << hexColor << "</color>" << std::endl;
	std::cout << "\t\t<width>" << width << "</width>" << std::endl;
	std::cout << "\t</LineStyle>" << std::endl;
	return true;
}

bool LineStyle::writeLineStyle(VSILFILE *fileOut)
{
	VSIFPrintfL(fileOut, "\t<LineStyle>\n");
	VSIFPrintfL(fileOut, "\t\t<color>%s</color>\n", hexColor.c_str());
	VSIFPrintfL(fileOut, "\t\t<width>%lf</width>\n", width);
	VSIFPrintfL(fileOut, "\t</LineStyle>\n");
	return true;
}
