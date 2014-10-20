/******************************************************************************
 *
 * $Id$
 *
 * Project:  WindNinja
 * Purpose:  Class for creating a kmz file with vectors given 2 grids
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

#include "KmlVector.h"

KmlVector::KmlVector()
: kmlTime(boost::local_time::not_a_date_time),
  wxModelStartTime(boost::local_time::not_a_date_time)
{
	colors = 0;
	splitValue = 0;
	lineWidth = 1.0;
	//makeDefaultStyles();
	speedUnits = velocityUnits::milesPerHour;
	resolution = -1.0;
	timeDateLegendFile = "";
	wxModelName = "";
}

//KmlVector::KmlVector(AsciiGrid<double> *s, AsciiGrid<double> *d, std::string demFileName, std::string kmzFileName, double res)
//{
//	spd = *s;
//	dir = *d;
//
//	if(spd.get_cellSize() == dir.get_cellSize())
//        resolution = spd.get_cellSize();
//	else
//		resolution = 0.0;
//
//	colors = 0;
//	splitValue = 0;
//	makeDefaultStyles();
//	lineWidth = 1.0;
//
//	setDemFile(demFileName);
//}
//
//KmlVector::KmlVector(AsciiGrid<double> *s, AsciiGrid<double> *d, std::string kmzFileName)
//{
//	spd = *s;
//	dir = *d;
//
//	if(spd.get_cellSize() == dir.get_cellSize())
//        resolution = spd.get_cellSize();
//	else
//		resolution = 0.0;
//
//	colors = 0;
//	splitValue = 0;
//	makeDefaultStyles();
//
//	lineWidth = 1.0;
//}
//
//KmlVector::KmlVector(AsciiGrid<double> *s, AsciiGrid<double> *d, std::string kmzFile, double res)
//{
//	spd = *s;
//	dir = *d;
//	if(spd.get_cellSize() == dir.get_cellSize())
//	{
//        resolution = res;
//		spd = s->resample_Grid(res, 1);
//		dir = d->resample_Grid(res, 1);
//	}
//	else
//		resolution = 0.0;
//
//	colors = 0;
//	splitValue = 0;
//	makeDefaultStyles();
//	lineWidth = 1.0;
//}
//KmlVector::KmlVector(std::string sFile, std::string dFile, std::string kFile)
//{
//	spd.read_Grid(sFile);
//	dir.read_Grid(dFile);
//	kmzFile = kFile;
//	if(spd.get_cellSize() == dir.get_cellSize())
//        resolution = spd.get_cellSize();
//	else
//		resolution = 0.0;
//
//	colors = 0;
//	splitValue = 0;
//	makeDefaultStyles();
//	lineWidth = 1.0;
//}
//
//KmlVector::KmlVector(std::string sFile, std::string dFile, std::string kFile, double res)
//{
//	spd.read_Grid(sFile);
//	dir.read_Grid(dFile);
//	kmzFile = kFile;
//	if(spd.get_cellSize() == dir.get_cellSize())
//    {
//        resolution = res;
//		spd = spd.resample_Grid(res, 1);
//		dir = dir.resample_Grid(res, 1);
//	}
//	else
//		resolution = 0.0;
//
//	colors = 0;
//	splitValue = 0;
//	makeDefaultStyles();
//	lineWidth = 1.0;
//}

KmlVector::~KmlVector()
{
    if(colors) {
	for(int i = 0;i < numColors;i++) {
	    delete colors[i];
	}
	delete[]colors;
    }

    if(splitValue)
	delete[]splitValue;

    OCTDestroyCoordinateTransformation( coordTransform );
}

void KmlVector::setSpeedGrid(AsciiGrid<double> &s, velocityUnits::eVelocityUnits units)
{
	speedUnits = units;
	spd = s;
}

void KmlVector::setDirGrid(AsciiGrid<double> &d)
{
	dir = d;
}

#ifdef FRICTION_VELOCITY
void KmlVector::setUstarGrid(AsciiGrid<double> &ust)
{
	ustar = ust;
}
#endif

#ifdef EMISSIONS
void KmlVector::setDustGrid(AsciiGrid<double> &dst)
{
	dust = dst;
}
#endif

void KmlVector::setWxModel(const std::string& modelName, const boost::local_time::local_date_time& startTime)
{
    wxModelName = modelName;
    wxModelStartTime = startTime;
}

bool KmlVector::makeDefaultStyles()
{
	if(colors)
		delete[]colors;

	//ostringstream oss;
	//std::string s;


	colors = new Style*[numColors];
	//for(int i = 0;i < numColors;i++)
	//{
		//oss >> i;
		//s = oss.str();
		//colors[i] = new Style(s,alpha,blue, green, red, width);
	//}
	colors[0] = new Style("blue", 255, 255, 0, 0, lineWidth);
	colors[1] = new Style("green", 255, 0, 255, 0, lineWidth);
	colors[2] = new Style("yellow", 255, 0, 255, 255, lineWidth);
	colors[3] = new Style("orange", 255, 0, 127, 255, lineWidth);
	colors[4] = new Style("red", 255, 0, 0, 255, lineWidth);
	return true;
}

bool KmlVector::setOGR()
{
    char** papszPrj;
    int errorVal = 1;

    if(spd.prjString != "")
	{
	    //try to read the prj std::string locally
	    papszPrj = CSLTokenizeString(spd.prjString.c_str());
	    errorVal = oSourceSRS.importFromESRI(papszPrj);
	    if(errorVal != OGRERR_NONE)
		throw std::logic_error("Cannot complete coordinate transformation, no kmz will be written");

	    errorVal = oTargetSRS.SetWellKnownGeogCS("WGS84");

	    coordTransform = OGRCreateCoordinateTransformation(&oSourceSRS, &oTargetSRS);

	    CSLDestroy(papszPrj);
	    if(coordTransform == NULL)
		throw std::logic_error("Cannot complete coordinate transformation, no kmz will be written");

	    return true;
	}

    throw std::logic_error("Cannot complete coordinate transformation, no kmz will be written");

    return false;
}


bool KmlVector::writeKml()
{
	VSILFILE* fout = 0;
	makeDefaultStyles();
	if((fout = VSIFOpenL(kmlFile.c_str(),"w")) == NULL)
		return false;
	else
	{
		if(setOGR())
		{
			if(splitValue)
				delete[] splitValue;
			splitValue = new double[numColors];

			spd.divide_gridData(splitValue, numColors);

			writeHeader(fout);
			writeRegion(fout);
			writeStyles(fout);
			//writeHtmlLegend(fout);
			writeScreenOverlayLegend(fout);

			if(wxModelName.empty())
			{
			    writeScreenOverlayDateTimeLegend(fout);
			}else{   //If it's a weather model run, write the wxModel info to the legends
			    writeScreenOverlayDateTimeLegendWxModelRun(fout);
			}
			VSIFPrintfL(fout, "<Folder>");
			VSIFPrintfL(fout, "\n\t<name>Wind Speed</name>\n");
			writeVectors(fout);
			VSIFPrintfL(fout, "</Folder>");

            #ifdef FRICTION_VELOCITY
            if(ustarFlag ==true)
			{
			    VSIFPrintfL(fout, "<Folder>");
			    VSIFPrintfL(fout, "\n\t<name>Friction Velocity</name>\n");
			    writeUstar(fout);
			    VSIFPrintfL(fout, "</Folder>");
			}
			#endif

			#ifdef EMISSIONS
			if(dustFlag==true)
            {
			    VSIFPrintfL(fout, "<Folder>");
			    VSIFPrintfL(fout, "\n\t<name>PM10</name>\n");
                writeDust(fout);
                VSIFPrintfL(fout, "</Folder>");
            }
            #endif
            
			VSIFPrintfL(fout, "\n</Document>\n</kml>");

			VSIFCloseL(fout);
			return true;
		}
		else
			return false;
	}
}

bool KmlVector::writeKml(egoogSpeedScaling scaling)
{
	VSILFILE *fout;
	makeDefaultStyles();
	if((fout = VSIFOpenL(kmlFile.c_str(),"w")) == NULL)
		return false;
	else
	{
		if(setOGR())
		{
			if(splitValue)
				delete[] splitValue;
			splitValue = new double[numColors];
			double interval;
			switch(scaling)
			{
				case equal_color:		//divide legend speeds using equal color method (equal numbers of arrows for each color)
					spd.divide_gridData(splitValue, numColors);
					break;
				case equal_interval:	//divide legend speeds using equal interval method (speed breaks divided equally over speed range)
					interval = spd.get_maxValue()/numColors;
					for(int i = 0;i < numColors;i++)
					{
						splitValue[i] = i * interval;
					}
					break;
				default:				//divide legend speeds using equal color method (equal numbers of arrows for each color)
					spd.divide_gridData(splitValue, numColors);
					break;
			}

			writeHeader(fout);
			writeRegion(fout);
			writeStyles(fout);
			//writeHtmlLegend(fout);
			writeScreenOverlayLegend(fout);
			if(wxModelName.empty())
			    writeScreenOverlayDateTimeLegend(fout);
			else
			    writeScreenOverlayDateTimeLegendWxModelRun(fout);
                        VSIFPrintfL(fout, "<Folder>");
			VSIFPrintfL(fout, "\n\t<name>Wind Speed</name>\n");
			writeVectors(fout);
                        VSIFPrintfL(fout, "</Folder>");
            
            #ifdef FRICTION_VELOCITY
            ustarFlag = false;
            
            if(ustar.get_nRows()!=0)
                ustarFlag = true;
            
            if(ustarFlag ==true)
			{
			    VSIFPrintfL(fout, "<Folder>");
			    VSIFPrintfL(fout, "\n\t<name>Friction Velocity</name>\n");
			    writeUstar(fout);
			    VSIFPrintfL(fout, "</Folder>");
			}
            #endif

            #ifdef EMISSIONS
            dustFlag = false;

            if(dust.get_nRows()!=0)
                dustFlag = true;

			if(dustFlag==true)
            {
			    VSIFPrintfL(fout, "<Folder>");
			    VSIFPrintfL(fout, "\n\t<name>PM10</name>\n");
                writeDust(fout);
                VSIFPrintfL(fout, "</Folder>");
            }
            #endif
            
            VSIFPrintfL(fout, "\n</Document>\n</kml>");
            VSIFCloseL(fout);
			return true;
		}
		else
			return false;
	}
}

bool KmlVector::writeHeader(VSILFILE* fileOut)
{
	std::string shortName;
    shortName = CPLGetBasename(kmlFile.c_str());

	VSIFPrintfL(fileOut,"<?xml version=\"1.0\" encoding=\"UTF-8\"?>");
        VSIFPrintfL(fileOut,"\n<kml xmlns=\"http://www.opengis.net/kml/2.2\" xmlns:gx=\"http://www.google.com/kml/ext/2.2\" xmlns:kml=\"http://www.opengis.net/kml/2.2\" xmlns:atom=\"http://www.w3.org/2005/Atom\">");
	VSIFPrintfL(fileOut,"\n<Document>");
	VSIFPrintfL(fileOut,"\n\t<name>%s</name>", shortName.c_str());

	if(!wxModelName.empty())
	{
        VSIFPrintfL(fileOut,"\n\t<description><![CDATA[Click on the blue link above for run information.<br><br>");
        VSIFPrintfL(fileOut,"\n\t\t<b>Forecast model name:</b><br>");
        VSIFPrintfL(fileOut,"\n\t\t%s<br><br>", wxModelName.c_str());
       
        std::ostringstream os;
        boost::local_time::local_time_facet* timeOutputFacet;
        timeOutputFacet = new boost::local_time::local_time_facet();
        //NOTE: WEIRD ISSUE WITH THE ABOVE 2 LINES OF CODE!  DO NOT CALL DELETE ON THIS BECAUSE THE LOCALE OBJECT BELOW DOES.
        //      THIS IS A "PROBLEM" IN THE STANDARD LIBRARY. SEE THESE WEB SITES FOR MORE INFO:
        //      https://collab.firelab.org/software/projects/windninja/wiki/KnownIssues
        //      http://rhubbarb.wordpress.com/2009/10/17/boost-datetime-locales-and-facets/#comment-203

        os.imbue(std::locale(std::locale::classic(), timeOutputFacet));
        timeOutputFacet->format("%A, %B %d, %Y");
        os << wxModelStartTime;

        VSIFPrintfL(fileOut,"\n\t\t<b>First forecast  time:</b><br>");
        VSIFPrintfL(fileOut,"\n\t\t%s<br>", os.str().c_str());

        os.str("");
        timeOutputFacet->format("%H:%M %z (");
        os << wxModelStartTime;

        std::string timeInfo(os.str());

        boost::posix_time::time_facet* timeOutputFacet2;
        timeOutputFacet2 = new boost::posix_time::time_facet();
        //NOTE: WEIRD ISSUE WITH THE ABOVE 2 LINES OF CODE!  DO NOT CALL DELETE ON THIS BECAUSE THE LOCALE OBJECT BELOW DOES.
        //      THIS IS A "PROBLEM" IN THE STANDARD LIBRARY. SEE THESE WEB SITES FOR MORE INFO:
        //      https://collab.firelab.org/software/projects/windninja/wiki/KnownIssues
        //      http://rhubbarb.wordpress.com/2009/10/17/boost-datetime-locales-and-facets/#comment-203

        os.imbue(std::locale(std::locale::classic(), timeOutputFacet2));

        os.str("");
        timeOutputFacet2->format("%H:%M UTC)");
        os << wxModelStartTime.utc_time();

        timeInfo.append(os.str());

        VSIFPrintfL(fileOut,"\n\t\t%s<br><br>", timeInfo.c_str());
        VSIFPrintfL(fileOut,"\n\t]]></description>");
       
	}
	return true;
}

bool KmlVector::writeRegion(VSILFILE *fileOut)
{
	northExtent = (spd.get_yllCorner() + (spd.get_nRows() * spd.get_cellSize()));
	eastExtent = (spd.get_xllCorner() + (spd.get_nCols() * spd.get_cellSize()));
	southExtent = (spd.get_yllCorner());
	westExtent = (spd.get_xllCorner());

	if(coordTransform != 0)
	{
		coordTransform->Transform(1, &westExtent, &southExtent);
		coordTransform->Transform(1, &eastExtent, &northExtent);
	}

    VSIFPrintfL(fileOut, "\n<Region>");
	VSIFPrintfL(fileOut, "\n\t<LatLonAltBox>");
	VSIFPrintfL(fileOut, "\n\t<north>%.10lf</north>", northExtent);
	VSIFPrintfL(fileOut, "\n\t<south>%.10lf</south>", southExtent);
	VSIFPrintfL(fileOut, "\n\t<east>%.10lf</east>", eastExtent);
	VSIFPrintfL(fileOut, "\n\t<west>%.10lf</west>", westExtent);
	VSIFPrintfL(fileOut, "\n\t<minAltitude>0</minAltitude>");
	VSIFPrintfL(fileOut, "\n\t<maxAltitude>0</maxAltitude>");
	VSIFPrintfL(fileOut, "\n\t</LatLonAltBox>");
	VSIFPrintfL(fileOut, "\n\t<Lod>");
	VSIFPrintfL(fileOut, "\n\t<minLodPixels>128</minLodPixels>");
	VSIFPrintfL(fileOut, "\n\t<maxLodPixels>-1</maxLodPixels>");
	VSIFPrintfL(fileOut, "\n\t<minFadeExtent>0</minFadeExtent>");
	VSIFPrintfL(fileOut, "\n\t<maxFadeExtent>0</maxFadeExtent>");
	VSIFPrintfL(fileOut, "\n\t</Lod>");
	VSIFPrintfL(fileOut, "\n</Region>");
    
	return true;
}

bool KmlVector::writeStyles(VSILFILE *fileOut)
{
	VSIFPrintfL(fileOut, "\n");

	for(int i = 0;i < numColors;i++)
	{
		colors[i]->writeStyle(fileOut);
	}
	return true;
}


bool KmlVector::writeHtmlLegend(VSILFILE *fileOut)
{
    VSIFPrintfL(fileOut, "<Placemark>");
	VSIFPrintfL(fileOut, "\n<name>Legend</name>");
	VSIFPrintfL(fileOut, "\n<description><![CDATA[");
	VSIFPrintfL(fileOut, "\n<body bgcolor=\"#000000\">");

	VSIFPrintfL(fileOut, "\n<big><big><big><font color=\"white\"><i>Legend</i><br></font></big></big></big>");
	VSIFPrintfL(fileOut, "\n<big><big><font color=\"white\"><i>Wind Speed (mph)</i><br></font></big></big>");
	//red range
	VSIFPrintfL(fileOut, "\n<big><big><big><big><font color=\"red\">&rarr </font></big></big></big></big>");
	VSIFPrintfL(fileOut, "\n<big><big><font color=\"white\">%.2lf</font></big></big>", splitValue[numColors - 1]);
	VSIFPrintfL(fileOut, "<big><big><font color=\"white\">+</font></big></big><br>");
	//orange range
	VSIFPrintfL(fileOut, "\n<big><big><big><big><font color=\"orange\"> &rarr </font></big></big></big></big>");
	VSIFPrintfL(fileOut, "\n<big><big><font color=\"white\">%.2lf</font></big></big>", splitValue[numColors - 2]);
	VSIFPrintfL(fileOut, "\n<big><big><font color=\"white\"> - %.2lf</font></big></big><br>", splitValue[numColors - 1] - 0.01);
	//yellow range
	VSIFPrintfL(fileOut, "\n<big><big><big><big><font color=\"yellow\">&rarr </font></big></big></big></big>");
	VSIFPrintfL(fileOut, "\n<big><big><font color=\"white\">%.2lf</font></big></big>", splitValue[numColors - 3]);
	VSIFPrintfL(fileOut, "\n<big><big><font color=\"white\"> - %.2lf</font></big></big><br>", splitValue[numColors - 2] - 0.01);
	//green range
	VSIFPrintfL(fileOut, "\n<big><big><big><big><font color=\"green\">&rarr </font></big></big></big></big>");
	VSIFPrintfL(fileOut, "\n<big><big><font color=\"white\">%.2lf</font></big></big>", splitValue[numColors - 4]);
	VSIFPrintfL(fileOut, "\n<big><big><font color=\"white\"> - %.2lf</font></big></big><br>", splitValue[numColors - 3] - 0.01);
	//blue range
	VSIFPrintfL(fileOut, "\n<big><big><big><big><font color=\"blue\">&rarr </font></big></big></big></big>");
	VSIFPrintfL(fileOut, "\n<big><big><font color=\"white\">%.2lf</font></big></big>", 0.0);
	VSIFPrintfL(fileOut, "\n<big><big><font color=\"white\"> - %.2lf </font></big></big><br>", splitValue[numColors - 4] - 0.01);

	VSIFPrintfL(fileOut, "\n]]></description>");
	VSIFPrintfL(fileOut, "\n</Placemark>");

	return true;
}

bool KmlVector::writeScreenOverlayLegend(VSILFILE *fileOut)
{
	//make bitmap
	int legendWidth = 180;
	int legendHeight = int(legendWidth / 0.75);
	BMP legend;

	std::string legendStrings[5];
	ostringstream os;

	double maxxx = spd.get_maxValue();

	for(int i = 0;i < 5;i++)
	{
		os << setiosflags(ios::fixed) << setiosflags(ios::showpoint) << setprecision(2);
		if(i == 0)
//			os[i] << splitValue[4] << " + ";
			os << splitValue[4] << " - " << maxxx;
		else if(i == 4)
			os << "0.00 - " << splitValue[1] - 0.01;
		else if(i != 0)
			os << splitValue[5 - i - 1] << " - " << splitValue[5 - i] - 0.01;

        legendStrings[i] = os.str();
		os.str("");
	}
	legend.SetSize(legendWidth,legendHeight);
	legend.SetBitDepth(8);

	//gray legend
	/*for(int i = 0;i < legendWidth;i++)
	{
		for(int j = 0;j < legendHeight;j++)
		{
			legend(i,j)->Alpha = 0;
			legend(i,j)->Blue = 192;
			legend(i,j)->Green = 192;
			legend(i,j)->Red = 192;
		}
	}*/

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
    /*
	//for black text
	RGBApixel black;
	black.Red = 0;
	black.Green = 0;
	black.Blue = 0;
	black.Alpha = 0;
    */
	//for white text
	RGBApixel white;
	white.Red = 255;
	white.Green = 255;
	white.Blue = 255;
	white.Alpha = 0;

	RGBApixel colors[5];
	//RGBApixel red, orange, yellow, green, blue;
	colors[0].Red = 255;
	colors[0].Green = 0;
	colors[0].Blue = 0;
	colors[0].Alpha = 0;

	colors[1].Red = 255;
	colors[1].Green = 127;
	colors[1].Blue = 0;
	colors[1].Alpha = 0;

	colors[2].Red = 255;
	colors[2].Green = 255;
	colors[2].Blue = 0;
	colors[2].Alpha = 0;

	colors[3].Red = 0;
	colors[3].Green = 255;
	colors[3].Blue = 0;
	colors[3].Alpha = 0;

	colors[4].Red = 0;
	colors[4].Green = 0;
	colors[4].Blue = 255;
	colors[4].Alpha = 0;

	int arrowLength = 40;	//pixels;
	int arrowHeadLength = 10; // pixels;
	int textHeight = 12;	//pixels- 10 for maximum speed of "999.99 - 555.55";
							//12 for normal double digits
	if(splitValue[4] >= 100)
		textHeight = 10;
	int titleTextHeight = int(1.2 * textHeight);
	int titleX, titleY;

    int x1, x2, x3, x4;
	double x;
	int y1, y2, y3, y4;
	double y;

	int textX;
	int textY;

	x = 0.05;
	//y = 0.10;
	y = 0.30;


	titleX = x * legendWidth;
	titleY = (y / 3) * legendHeight;

	switch(speedUnits)
	{
		case velocityUnits::metersPerSecond:	// m/s
			PrintString(legend,"Wind Speed (m/s)", titleX, titleY, titleTextHeight, white);
			break;
		case velocityUnits::milesPerHour:		// mph
			PrintString(legend,"Wind Speed (mph)", titleX, titleY, titleTextHeight, white);
			break;
		case velocityUnits::kilometersPerHour:	// kph
			PrintString(legend,"Wind Speed (kph)", titleX, titleY, titleTextHeight, white);
			break;
		default:				// default is mph
			PrintString(legend,"Wind Speed (mph)", titleX, titleY, titleTextHeight, white);
			break;
	}

	for(int i = 0;i < 5;i++)
	{
		x1 = int(legendWidth * x);
		x2 = x1 + arrowLength;
		y1 = int(legendHeight * y);
		y2 = y1;

		x3 = x2 - arrowHeadLength;
		y3 = y2 + arrowHeadLength;

		x4 = x2 - arrowHeadLength;
		y4 = y2 - arrowHeadLength;

		textX = x2 + 10;
		textY = y2 - int(textHeight * 0.5);


		DrawLine(legend, x1, y1, x2, y2, colors[i]);
		DrawLine(legend, x2, y2, x3, y3, colors[i]);
		DrawLine(legend, x2, y2, x4, y4, colors[i]);

		PrintString(legend, legendStrings[i].c_str(), textX, textY, textHeight, white);


		y += 0.15;
	}

	legend.WriteToFile(legendFile.c_str());

	//printf("\n\nfileOut in writeScreenOverlayLegend = %x\n", fileOut);

	std::string shortName;
	shortName = CPLGetFilename(legendFile.c_str());

    VSIFPrintfL(fileOut, "<ScreenOverlay>");
	VSIFPrintfL(fileOut, "\n<name>Legend</name>");
	VSIFPrintfL(fileOut, "\n<visibility>1</visibility>");
	VSIFPrintfL(fileOut, "\n<color>9bffffff</color>");
	VSIFPrintfL(fileOut, "\n<Snippet maxLines=\"0\"></Snippet>");
	VSIFPrintfL(fileOut, "\n<Icon>");
	VSIFPrintfL(fileOut, "\n<href>%s</href>", shortName.c_str());
	VSIFPrintfL(fileOut, "\n</Icon>");
	VSIFPrintfL(fileOut, "\n<overlayXY x=\"0\" y=\"1\" xunits=\"fraction\" yunits=\"fraction\"/>");
	VSIFPrintfL(fileOut, "\n<screenXY x=\"0\" y=\"1\" xunits=\"fraction\" yunits=\"fraction\"/>");
	VSIFPrintfL(fileOut, "\n<rotationXY x=\"0\" y=\"0\" xunits=\"fraction\" yunits=\"fraction\"/>");
	VSIFPrintfL(fileOut, "\n<size x=\"0\" y=\"0\" xunits=\"fraction\" yunits=\"fraction\"/>");
	VSIFPrintfL(fileOut, "\n</ScreenOverlay>\n");
    
	return true;
}

bool KmlVector::writeScreenOverlayDateTimeLegend(VSILFILE *fileOut)
{

	if(timeDateLegendFile == "")
		return false;

	//make bitmap
	int legendWidth = 285;
	int legendHeight = 52;
	BMP legend;

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
	RGBApixel white;
	white.Red = 255;
	white.Green = 255;
	white.Blue = 255;
	white.Alpha = 0;

	int textHeight = 12;	//pixels- 10 for maximum speed of "999.99 - 555.55";
							//12 for normal double digits
	int titleX, titleY;

	double x;
	double y;

	//print date
	x = 0.05;
	y = 0.15;
	titleX = x * legendWidth;
	titleY = y * legendHeight;

//	ostringstream os;
//	boost::gregorian::date::ymd_type ymd = kmlTime.local_time().date().year_month_day();
//	boost::gregorian::greg_weekday wd = kmlTime.local_time().date().day_of_week();
//
//	os <<  wd.as_long_string() << " " << ymd.month.as_long_string() << " " << ymd.day << ", " << ymd.year;
//	std::string date_string = os.str();

	std::ostringstream os;
	boost::local_time::local_time_facet* timeOutputFacet;
	timeOutputFacet = new boost::local_time::local_time_facet();
	//NOTE: WEIRD ISSUE WITH THE ABOVE 2 LINES OF CODE!  DO NOT CALL DELETE ON THIS BECAUSE THE LOCALE OBJECT BELOW DOES.
	//		THIS IS A "PROBLEM" IN THE STANDARD LIBRARY. SEE THESE WEB SITES FOR MORE INFO:
	//		https://collab.firelab.org/software/projects/windninja/wiki/KnownIssues
	//		http://rhubbarb.wordpress.com/2009/10/17/boost-datetime-locales-and-facets/#comment-203



	os.imbue(std::locale(std::locale::classic(), timeOutputFacet));
	timeOutputFacet->format("%A, %B %d, %Y");

	os << kmlTime;

	PrintString(legend,os.str().c_str(), titleX, titleY, textHeight, white);

	//print time
	x = 0.05;
	y = 0.60;

	titleX = x * legendWidth;
	titleY = y * legendHeight;

	os.str("");
	//timeOutputFacet->format("%H:%M %z (%Q from UTC)");
	timeOutputFacet->format("%H:%M %z (");
	os << kmlTime;

	std::string timeStringLegend(os.str());

	boost::posix_time::time_facet* timeOutputFacet2;
	timeOutputFacet2 = new boost::posix_time::time_facet();
	//NOTE: WEIRD ISSUE WITH THE ABOVE 2 LINES OF CODE!  DO NOT CALL DELETE ON THIS BECAUSE THE LOCALE OBJECT BELOW DOES.
	//      THIS IS A "PROBLEM" IN THE STANDARD LIBRARY. SEE THESE WEB SITES FOR MORE INFO:
	//      https://collab.firelab.org/software/projects/windninja/wiki/KnownIssues
	//      http://rhubbarb.wordpress.com/2009/10/17/boost-datetime-locales-and-facets/#comment-203

	os.imbue(std::locale(std::locale::classic(), timeOutputFacet2));

	os.str("");
	timeOutputFacet2->format("%H:%M UTC)");
	os << kmlTime.utc_time();

	timeStringLegend.append(os.str());

	PrintString(legend,timeStringLegend.c_str(), titleX, titleY, textHeight, white);


	legend.WriteToFile(timeDateLegendFile.c_str());

	std::string shortName;
	shortName = CPLGetFilename(timeDateLegendFile.c_str());

    VSIFPrintfL(fileOut, "<ScreenOverlay>");
	VSIFPrintfL(fileOut, "\n<name>Date-Time</name>");
	VSIFPrintfL(fileOut, "\n<visibility>1</visibility>");
	VSIFPrintfL(fileOut, "\n<color>9bffffff</color>");
	VSIFPrintfL(fileOut, "\n<Snippet maxLines=\"0\"></Snippet>");
	VSIFPrintfL(fileOut, "\n<Icon>");
	VSIFPrintfL(fileOut, "\n<href>%s</href>", shortName.c_str());
	VSIFPrintfL(fileOut, "\n</Icon>");
	VSIFPrintfL(fileOut, "\n<overlayXY x=\"0.5\" y=\"1\" xunits=\"fraction\" yunits=\"fraction\"/>");
	VSIFPrintfL(fileOut, "\n<screenXY x=\"0.5\" y=\"1\" xunits=\"fraction\" yunits=\"fraction\"/>");
	VSIFPrintfL(fileOut, "\n<rotationXY x=\"0\" y=\"0\" xunits=\"fraction\" yunits=\"fraction\"/>");
	VSIFPrintfL(fileOut, "\n<size x=\"0\" y=\"0\" xunits=\"fraction\" yunits=\"fraction\"/>");
	VSIFPrintfL(fileOut, "\n</ScreenOverlay>\n");
    
	return true;
}

bool KmlVector::writeScreenOverlayDateTimeLegendWxModelRun(VSILFILE *fileOut)
{

    if(timeDateLegendFile == "")
        return false;

    //make bitmap
    int legendWidth = 285;
    int legendHeight = 78;
    BMP legend;

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
    RGBApixel white;
    white.Red = 255;
    white.Green = 255;
    white.Blue = 255;
    white.Alpha = 0;

    int textHeight = 12;    //pixels- 10 for maximum speed of "999.99 - 555.55";
                            //12 for normal double digits
    int titleX, titleY;

    double x;
    double y;

    //print wxModel name----------------------------------------------------
    x = 0.05;
    y = 0.10;

    titleX = x * legendWidth;
    titleY = y * legendHeight;

    PrintString(legend,wxModelName.c_str(), titleX, titleY, textHeight, white);

    //print date---------------------------------------------------------
    x = 0.05;
    y = 0.40;
    titleX = x * legendWidth;
    titleY = y * legendHeight;

    std::ostringstream os;
    boost::local_time::local_time_facet* timeOutputFacet;
    timeOutputFacet = new boost::local_time::local_time_facet();
    //NOTE: WEIRD ISSUE WITH THE ABOVE 2 LINES OF CODE!  DO NOT CALL DELETE ON THIS BECAUSE THE LOCALE OBJECT BELOW DOES.
    //      THIS IS A "PROBLEM" IN THE STANDARD LIBRARY. SEE THESE WEB SITES FOR MORE INFO:
    //      https://collab.firelab.org/software/projects/windninja/wiki/KnownIssues
    //      http://rhubbarb.wordpress.com/2009/10/17/boost-datetime-locales-and-facets/#comment-203



    os.imbue(std::locale(std::locale::classic(), timeOutputFacet));
    timeOutputFacet->format("%A, %B %d, %Y");

    os << kmlTime;

    PrintString(legend,os.str().c_str(), titleX, titleY, textHeight, white);

    //print time---------------------------------------------------------
    x = 0.05;
    y = 0.70;

    titleX = x * legendWidth;
    titleY = y * legendHeight;

    os.str("");
    //timeOutputFacet->format("%H:%M %z (%Q from UTC)");
    timeOutputFacet->format("%H:%M %z (");
    os << kmlTime;

    std::string timeStringLegend(os.str());

    boost::posix_time::time_facet* timeOutputFacet2;
    timeOutputFacet2 = new boost::posix_time::time_facet();
    //NOTE: WEIRD ISSUE WITH THE ABOVE 2 LINES OF CODE!  DO NOT CALL DELETE ON THIS BECAUSE THE LOCALE OBJECT BELOW DOES.
    //      THIS IS A "PROBLEM" IN THE STANDARD LIBRARY. SEE THESE WEB SITES FOR MORE INFO:
    //      https://collab.firelab.org/software/projects/windninja/wiki/KnownIssues
    //      http://rhubbarb.wordpress.com/2009/10/17/boost-datetime-locales-and-facets/#comment-203

    os.imbue(std::locale(std::locale::classic(), timeOutputFacet2));

    os.str("");
    timeOutputFacet2->format("%H:%M UTC)");
    os << kmlTime.utc_time();

    timeStringLegend.append(os.str());

    PrintString(legend,timeStringLegend.c_str(), titleX, titleY, textHeight, white);


    legend.WriteToFile(timeDateLegendFile.c_str());

    //printf("\n\nfileOut in writeScreenOverlayLegend = %x\n", fileOut);

    std::string shortName;
    shortName = CPLGetFilename(timeDateLegendFile.c_str());

    VSIFPrintfL(fileOut, "<ScreenOverlay>");
    VSIFPrintfL(fileOut, "\n<name>Date-Time</name>");
    VSIFPrintfL(fileOut, "\n<visibility>1</visibility>");
    VSIFPrintfL(fileOut, "\n<color>9bffffff</color>");
    VSIFPrintfL(fileOut, "\n<Snippet maxLines=\"0\"></Snippet>");
    VSIFPrintfL(fileOut, "\n<Icon>");
    VSIFPrintfL(fileOut, "\n<href>%s</href>", shortName.c_str());
    VSIFPrintfL(fileOut, "\n</Icon>");
    VSIFPrintfL(fileOut, "\n<overlayXY x=\"0.5\" y=\"1\" xunits=\"fraction\" yunits=\"fraction\"/>");
    VSIFPrintfL(fileOut, "\n<screenXY x=\"0.5\" y=\"1\" xunits=\"fraction\" yunits=\"fraction\"/>");
    VSIFPrintfL(fileOut, "\n<rotationXY x=\"0\" y=\"0\" xunits=\"fraction\" yunits=\"fraction\"/>");
    VSIFPrintfL(fileOut, "\n<size x=\"0\" y=\"0\" xunits=\"fraction\" yunits=\"fraction\"/>");
    VSIFPrintfL(fileOut, "\n</ScreenOverlay>\n");
    
    return true;
}

#ifdef FRICTION_VELOCITY
bool KmlVector::writeUstar(FILE *fileOut)
{
	double xPoint, yPoint;
	double xCenter, yCenter;
	double left_x, right_x, lower_y, upper_y;
	double u = 0;
	double cSize;
	int nR;
	int nC;
	double upper, lower, upper_mid, lower_mid, mid;
	std::string icon;

	ustar_png = "ustar_png.png";

	cSize = ustar.get_cellSize();
	nR = ustar.get_nRows();
	nC = ustar.get_nCols();

	lower = ustar.get_minValue();
	upper = ustar.get_maxValue();
	lower_mid = lower + (ustar.get_maxValue() - ustar.get_minValue())/4;
	upper_mid = upper - (ustar.get_maxValue() - ustar.get_minValue())/4;
	mid = upper_mid - (ustar.get_maxValue() - ustar.get_minValue())/4;

    //---------------make single png for overlay------------------
    std::string outFilename = "ustar_png.png";
    std::string scalarLegendFilename = "ustar_legend";
    std::string legendTitle = "Friction Velocity";
    std::string legendUnits = "(m/s)";
    bool writeLegend = TRUE;

    ustar.ascii2png(outFilename, legendTitle, legendUnits, scalarLegendFilename, writeLegend);

    ustar.get_cellPosition(0, 0, &xCenter, &yCenter); //sw
    left_x = xCenter - cSize/2; //west
    lower_y = yCenter - cSize/2; //south
    ustar.get_cellPosition(nR-1, nC-1, &xCenter, &yCenter); //ne
    right_x = xCenter + cSize/2; //east
    upper_y = yCenter + cSize/2;  //north

	coordTransform->Transform(1, &right_x, &upper_y);
	coordTransform->Transform(1, &left_x, &lower_y);
	coordTransform->Transform(1, &xCenter, &yCenter);

	int pos;
	std::string shortName;
	pos = ustar_png.find_last_of('\\');
	if(pos == -1)
	  pos = ustar_png.find_last_of('/');

	shortName = ustar_png.substr(pos + 1, ustar_png.size());

	VSIFPrintfL(fileOut, "<GroundOverlay>");
	VSIFPrintfL(fileOut, "\n\t<name>Friction Velocity</name>");
	VSIFPrintfL(fileOut, "\n\t<ExtendedData>");
	VSIFPrintfL(fileOut, "\n\t\t<Data name=\"Ustar\">");
	VSIFPrintfL(fileOut, "\n\t\t\t<value>2</value>");
	VSIFPrintfL(fileOut, "\n\t\t</Data>");
	VSIFPrintfL(fileOut, "\n\t</ExtendedData>");

	VSIFPrintfL(fileOut, "\n\t<altitude>0</altitude>");
	VSIFPrintfL(fileOut, "\n\t<altitudeMode>clampToGround</altitudeMode>");
	//VSIFPrintfL(fileOut, "\n\t\t<color>88ffffff</color>");

	VSIFPrintfL(fileOut, "\n\t<Icon>");
	VSIFPrintfL(fileOut, "\n\t\t<href>%s</href>", shortName.c_str());  //ustar_png.png
	VSIFPrintfL(fileOut, "\n\t</Icon>");

	VSIFPrintfL(fileOut, "\n\t<LatLonBox>");
	VSIFPrintfL(fileOut, "\n\t\t<north>%.10lf</north>", upper_y);
	VSIFPrintfL(fileOut, "\n\t\t<south>%.10lf</south>", lower_y);
	VSIFPrintfL(fileOut, "\n\t\t<east>%.10lf</east>", right_x);
	VSIFPrintfL(fileOut, "\n\t\t<west>%.10lf</west>", left_x);
    VSIFPrintfL(fileOut, "\n\t\t<rotation>0</rotation>");
	VSIFPrintfL(fileOut, "\n\t</LatLonBox>");

	VSIFPrintfL(fileOut, "\n</GroundOverlay>\n");

	//---add legend----------------------------------------------
    ustar_legend = "./ustar_legend";
	//int pos;
	//std::string shortName;
	pos = ustar_legend.find_last_of('\\');
	if(pos == -1)
	  pos = ustar_legend.find_last_of('/');

	shortName = ustar_legend.substr(pos + 1, ustar_legend.size());

	VSIFPrintfL(fileOut, "<ScreenOverlay>");
	VSIFPrintfL(fileOut, "\n<name>Legend</name>");
	VSIFPrintfL(fileOut, "\n<visibility>1</visibility>");
	VSIFPrintfL(fileOut, "\n<color>9bffffff</color>");
	VSIFPrintfL(fileOut, "\n<Snippet maxLines=\"0\"></Snippet>");
	VSIFPrintfL(fileOut, "\n<Icon>");
	VSIFPrintfL(fileOut, "\n<href>%s</href>", shortName.c_str());
	VSIFPrintfL(fileOut, "\n</Icon>");
	VSIFPrintfL(fileOut, "\n<overlayXY x=\"0\" y=\"0\" xunits=\"fraction\" yunits=\"fraction\"/>");
	VSIFPrintfL(fileOut, "\n<screenXY x=\"0\" y=\"0\" xunits=\"fraction\" yunits=\"fraction\"/>");
	VSIFPrintfL(fileOut, "\n<rotationXY x=\"0\" y=\"0\" xunits=\"fraction\" yunits=\"fraction\"/>");
	VSIFPrintfL(fileOut, "\n<size x=\"0\" y=\"0\" xunits=\"fraction\" yunits=\"fraction\"/>");
	VSIFPrintfL(fileOut, "\n</ScreenOverlay>\n");

	return true;
}
#endif

#ifdef EMISSIONS
bool KmlVector::writeDust(FILE *fileOut)
{
	double xPoint, yPoint;
	double xCenter, yCenter;
	double left_x, right_x, lower_y, upper_y;
	double u = 0;
	double cSize;
	int nR;
	int nC;
	double upper, lower, upper_mid, lower_mid, mid;
	std::string icon;

	cSize = dust.get_cellSize();
	nR = dust.get_nRows();
	nC = dust.get_nCols();

	lower = dust.get_minValue();
	upper = dust.get_maxValue();
	lower_mid = lower + (dust.get_maxValue() - dust.get_minValue())/4;
	upper_mid = upper - (dust.get_maxValue() - dust.get_minValue())/4;
	mid = upper_mid - (dust.get_maxValue() - dust.get_minValue())/4;

    //---------------make png for overlay------------------
    std::string outFilename = "dust_png.png";
    std::string scalarLegendFilename = "dust_legend";
    std::string legendTitle = "PM10";
    std::string legendUnits = "(mg/m2/s)";
    bool writeLegend = true;
    
    dust.ascii2png(outFilename, legendTitle, legendUnits, scalarLegendFilename, writeLegend);

    dust_png = "dust_png.png";

    dust.get_cellPosition(0, 0, &xCenter, &yCenter); //sw
    left_x = xCenter - cSize/2; //west
    lower_y = yCenter - cSize/2; //south
    dust.get_cellPosition(nR-1, nC-1, &xCenter, &yCenter); //ne
    right_x = xCenter + cSize/2; //east
    upper_y = yCenter + cSize/2;  //north


	coordTransform->Transform(1, &right_x, &upper_y);
	coordTransform->Transform(1, &left_x, &lower_y);
	coordTransform->Transform(1, &xCenter, &yCenter);

	std::string shortName;
	int pos;
	pos = dust_png.find_last_of('\\');
	if(pos == -1)
	  pos = dust_png.find_last_of('/');
	shortName = dust_png.substr(pos + 1, dust_png.size());

	VSIFPrintfL(fileOut, "<GroundOverlay>");
	VSIFPrintfL(fileOut, "\n\t<name>PM10</name>");
	VSIFPrintfL(fileOut, "\n\t<ExtendedData>");
	VSIFPrintfL(fileOut, "\n\t\t<Data name=\"PM10\">");
	VSIFPrintfL(fileOut, "\n\t\t\t<value>2</value>");
	VSIFPrintfL(fileOut, "\n\t\t</Data>");
	VSIFPrintfL(fileOut, "\n\t</ExtendedData>");

	VSIFPrintfL(fileOut, "\n\t<altitude>0</altitude>");
	VSIFPrintfL(fileOut, "\n\t<altitudeMode>clampToGround</altitudeMode>");
	//VSIFPrintfL(fileOut, "\n\t\t<color>88ffffff</color>");

	VSIFPrintfL(fileOut, "\n\t<Icon>");
	VSIFPrintfL(fileOut, "\n\t\t<href>%s</href>", shortName.c_str());  //dust_png.png
	VSIFPrintfL(fileOut, "\n\t</Icon>");

	VSIFPrintfL(fileOut, "\n\t<LatLonBox>");
	VSIFPrintfL(fileOut, "\n\t\t<north>%.10lf</north>", upper_y);
	VSIFPrintfL(fileOut, "\n\t\t<south>%.10lf</south>", lower_y);
	VSIFPrintfL(fileOut, "\n\t\t<east>%.10lf</east>", right_x);
	VSIFPrintfL(fileOut, "\n\t\t<west>%.10lf</west>", left_x);
    VSIFPrintfL(fileOut, "\n\t\t<rotation>0</rotation>");
	VSIFPrintfL(fileOut, "\n\t</LatLonBox>");

	VSIFPrintfL(fileOut, "\n</GroundOverlay>\n");

	//---add legend----------------------------------------------
    dust_legend = "dust_legend";
	pos = dust_legend.find_last_of('\\');
	if(pos == -1)
	  pos = dust_legend.find_last_of('/');

	shortName = dust_legend.substr(pos + 1, dust_legend.size());

	VSIFPrintfL(fileOut, "<ScreenOverlay>");
	VSIFPrintfL(fileOut, "\n<name>Legend</name>");
	VSIFPrintfL(fileOut, "\n<visibility>1</visibility>");
	VSIFPrintfL(fileOut, "\n<color>9bffffff</color>");
	VSIFPrintfL(fileOut, "\n<Snippet maxLines=\"0\"></Snippet>");
	VSIFPrintfL(fileOut, "\n<Icon>");
	VSIFPrintfL(fileOut, "\n<href>%s</href>", shortName.c_str());
	VSIFPrintfL(fileOut, "\n</Icon>");
	VSIFPrintfL(fileOut, "\n<overlayXY x=\"0\" y=\"0\" xunits=\"fraction\" yunits=\"fraction\"/>");
	VSIFPrintfL(fileOut, "\n<screenXY x=\"0\" y=\"0\" xunits=\"fraction\" yunits=\"fraction\"/>");
	VSIFPrintfL(fileOut, "\n<rotationXY x=\"0\" y=\"0\" xunits=\"fraction\" yunits=\"fraction\"/>");
	VSIFPrintfL(fileOut, "\n<size x=\"0\" y=\"0\" xunits=\"fraction\" yunits=\"fraction\"/>");
	VSIFPrintfL(fileOut, "\n</ScreenOverlay>\n");

	return true;
}
#endif

bool KmlVector::writeVectors(VSILFILE *fileOut)
{
	//int b, g, y, o, r;
	//b = g = y = o = r = 0;

	double xPoint, yPoint;
	double xCenter, yCenter;
	double xTip, yTip, xTail, yTail, xHeadLeft, xHeadRight, yHeadLeft, yHeadRight;
	double theta;
	double yScale = 0.5;
	double xScale = yScale * 0.4;
	double s = 0;
	//double d = 0;
	double cSize;
	int nR;
	int nC;

	cSize = spd.get_cellSize();
	nR = spd.get_nRows();
	nC = spd.get_nCols();


	//double PI = acos(-1.0);
	geTheta = 0;
	for(int i = 0;i < nR;i++)
	{
		for(int j = 0;j < nC;j++)
		{
			yScale = 0.5;
			s = spd(i,j);
			geTheta = dir(i,j);
			theta = dir(i,j) + 180.0;

			if(s <= splitValue[1])
				yScale *= 0.40;
			else if(s <= splitValue[2])
				yScale *= 0.60;
			else if(s <= splitValue[3])
				yScale *= 0.80;
			else if(s <= splitValue[4])
				yScale *= 1.0;
			xScale = yScale * 0.40;

			spd.get_cellPosition(i, j, &xCenter, &yCenter);

			//xCenter = (cSize / 2.0) + (j * cSize + spd.get_xllCorner());
			//yCenter = (cSize / 2.0) + ((nR - i - 1) * cSize) + spd.get_yllCorner();

			if(theta > 360)
			{
				theta -= 360;
			}
			theta = 360 - theta;

			theta = theta * (PI / 180);

			if( areEqual( s, 0.0 ) ) {
			    double square_size = 16;
			    xTip = xCenter - cSize / square_size;
			    yTip = yCenter + cSize / square_size;
			    xTail = xCenter + cSize / square_size;
			    yTail = yCenter + cSize / square_size;
			    xHeadLeft = xCenter - cSize / square_size;
			    yHeadLeft = yCenter - cSize / square_size;
			    xHeadRight = xCenter + cSize / square_size;
			    yHeadRight = yCenter - cSize / square_size;
			}
			else {
			    xPoint = 0;
			    yPoint = (cSize * yScale);

			    //compute tip coordinates
			    xTip = (xPoint * cos(theta)) - (yPoint * sin(theta));
			    yTip = (xPoint * sin(theta)) + (yPoint * cos(theta));
			    //compute tail coordinates
			    xTail = -xTip;
			    yTail = -yTip;

			    //compute right and left coordinates for head
			    xPoint = (cSize * xScale);
			    yPoint = (cSize * yScale)-(cSize * xScale);

			    xHeadRight = (xPoint * cos(theta)) - (yPoint * sin(theta));
			    yHeadRight = (xPoint * sin(theta)) + (yPoint * cos(theta));

			    xPoint = -(cSize * xScale);
			    yPoint = (cSize * yScale) - (cSize * xScale);
			    xHeadLeft = (xPoint * cos(theta)) - (yPoint * sin(theta));
			    yHeadLeft = (xPoint * sin(theta)) + (yPoint * cos(theta));

			    //shift to global coordinates
			    xTip+=xCenter;
			    yTip+=yCenter;
			    xTail+=xCenter;
			    yTail+=yCenter;
			    xHeadRight += xCenter;
			    yHeadRight += yCenter;
			    xHeadLeft += xCenter;
			    yHeadLeft += yCenter;
			}
			coordTransform->Transform(1, &xTip, &yTip);
			coordTransform->Transform(1, &xTail, &yTail);
			coordTransform->Transform(1, &xHeadRight, &yHeadRight);
			coordTransform->Transform(1, &xHeadLeft, &yHeadLeft);

			if(s != spd.get_noDataValue() && theta != dir.get_noDataValue())
			{
                VSIFPrintfL(fileOut, "<Placemark>");
				//fprintf(fileOut, "\n<Icon><href>ffs_icon.ico</href></Icon>");
				VSIFPrintfL(fileOut, "\n\t<name>Cell %d,%d</name>", i, j);
				//fprintf(fileOut, "\n\t<description>Speed= %lf mph,Angle= %lf deg</description>", s, theta);
				VSIFPrintfL(fileOut, "\n\t<ExtendedData>");
				VSIFPrintfL(fileOut, "\n\t\t<Data name=\"Speed\">");
				VSIFPrintfL(fileOut, "\n\t\t\t<value>%lf</value>", s);
				VSIFPrintfL(fileOut, "\n\t\t</Data>");
				VSIFPrintfL(fileOut, "\n\t\t<Data name=\"Angle\">");
				VSIFPrintfL(fileOut, "\n\t\t\t<value>%lf</value>", geTheta);
				VSIFPrintfL(fileOut, "\n\t\t</Data>");
				VSIFPrintfL(fileOut, "\n\t</ExtendedData>");
				VSIFPrintfL(fileOut, "\n\t<styleUrl>");
				if(s <= splitValue[1])
				{
					VSIFPrintfL(fileOut, "#blue");
					//b++;
				}
				else if(s <= splitValue[2])
				{
					VSIFPrintfL(fileOut, "#green");
					//g++;
				}
				else if(s <= splitValue[3])
				{
					VSIFPrintfL(fileOut, "#yellow");
					//y++;
				}
				else if(s <= splitValue[4])
				{
					VSIFPrintfL(fileOut, "#orange");
					//o++;
				}
				else
				{
					VSIFPrintfL(fileOut, "#red");
					//r++;
				}
				VSIFPrintfL(fileOut, "</styleUrl>");

				if( areEqual( s, 0.0 ) ) {
                    VSIFPrintfL(fileOut, "\n\t<LineString>");
				    VSIFPrintfL(fileOut, "\n\t<extrude>0</extrude>");
				    VSIFPrintfL(fileOut, "\n\t<altitudeMode>relativeToGround</altitudeMode>");
				    VSIFPrintfL(fileOut, "\n\t<coordinates>\n");
				    VSIFPrintfL(fileOut, "\t\t%.10lf,%.10lf,%lf\n", xTip, yTip, (cSize / 8));
				    VSIFPrintfL(fileOut, "\t\t%.10lf,%.10lf,%lf\n", xTail, yTail, (cSize / 8));
				    VSIFPrintfL(fileOut, "\t\t%.10lf,%.10lf,%lf\n",xHeadRight, yHeadRight, (cSize / 8));
				    VSIFPrintfL(fileOut, "\t\t%.10lf,%.10lf,%lf\n", xHeadLeft, yHeadLeft, (cSize / 8));
				    VSIFPrintfL(fileOut, "\t\t%.10lf,%.10lf,%lf\n", xTip, yTip, (cSize / 8));
				    VSIFPrintfL(fileOut, "\t</coordinates>\n");
				    VSIFPrintfL(fileOut, "\t</LineString>\n");
				    VSIFPrintfL(fileOut, "</Placemark>\n");
				}
				else {
                    VSIFPrintfL(fileOut, "\n\t<LineString>");
				    VSIFPrintfL(fileOut, "\n\t<extrude>0</extrude>");
				    VSIFPrintfL(fileOut, "\n\t<altitudeMode>relativeToGround</altitudeMode>");
				    VSIFPrintfL(fileOut, "\n\t<coordinates>\n");
				    VSIFPrintfL(fileOut, "\t\t%.10lf,%.10lf,%lf\n",xHeadRight, yHeadRight, (cSize / 8));
				    VSIFPrintfL(fileOut, "\t\t%.10lf,%.10lf,%lf\n",xTip, yTip, (cSize / 8));
				    VSIFPrintfL(fileOut, "\t\t%.10lf,%.10lf,%lf\n", xHeadLeft, yHeadLeft, (cSize / 8));
				    VSIFPrintfL(fileOut, "\t\t%.10lf,%.10lf,%lf\n", xTip, yTip, (cSize / 8));
				    VSIFPrintfL(fileOut, "\t\t%.10lf,%.10lf,%lf\n", xTail, yTail, (cSize / 8));
				    VSIFPrintfL(fileOut, "\t</coordinates>\n");
				    VSIFPrintfL(fileOut, "\t</LineString>\n");
				    VSIFPrintfL(fileOut, "</Placemark>\n");
				}
			}
		}
	}
	//std::cout << endl << r << endl << o << endl << y << endl << g << endl << b << endl;

	return true;
}

/**
*@brief Uses GDAL VSI to make .kmz files
*/
bool KmlVector::makeKmz()
{
  std::string args;

  std::vector<std::string>filesToZip;
  std::vector<std::string>filesInZip;

  filesToZip.push_back(kmlFile);
  filesToZip.push_back(legendFile);

  filesInZip.push_back(getShortName(kmlFile));
  filesInZip.push_back(getShortName(legendFile));

  #ifdef FRICTION_VELOCITY
  if(ustarFlag==1)
  {
      filesToZip.push_back(ustar_png);
      filesToZip.push_back(ustar_legend);
      filesInZip.push_back(getShortName(ustar_png));
      filesInZip.push_back(getShortName(ustar_legend));
  }
  #endif

  #ifdef EMISSIONS
  if(dustFlag==1)
  {
      filesToZip.push_back(dust_png);
      filesToZip.push_back(dust_legend);
      filesInZip.push_back(getShortName(dust_png));
      filesInZip.push_back(getShortName(dust_legend));
  }
  #endif

  if(timeDateLegendFile != "")
    {
      filesToZip.push_back(timeDateLegendFile);
      filesInZip.push_back(getShortName(timeDateLegendFile));
    }

  /* Check for an existing archive */
  if(CPLCheckForFile((char*)kmzFile.c_str(), NULL))
  {
      VSIUnlink(kmzFile.c_str());
  }


  for(int x=0; x<filesToZip.size(); x++)
  {
    VSILFILE *fin;
    VSILFILE *fout;

    fin = VSIFOpenL(filesToZip[x].c_str(), "r");
    vsi_l_offset offset;
    VSIFSeekL(fin, 0, SEEK_END);
    offset = VSIFTellL(fin);

    VSIRewindL(fin);
    char *data = (char*)CPLMalloc(offset * sizeof(char));
    VSIFReadL(data, offset, 1, fin);
    VSIFCloseL(fin);

    std::string archive = "/vsizip/";
    archive.append(kmzFile);
    archive.append("/");
    archive.append(filesInZip[x]);

    fout = VSIFOpenL(archive.c_str(), "w");
    VSIFWriteL(data, offset, 1, fout);
    VSIFCloseL(fout);

    CPLFree(data);

  }

  return true;
}
bool KmlVector::removeKmlFile()
{

    VSIUnlink(kmlFile.c_str());
    VSIUnlink(legendFile.c_str());
    if(timeDateLegendFile != "")
        VSIUnlink(timeDateLegendFile.c_str());
    
    #ifdef FRICTION_VELOCITY
    if(ustar_png.c_str() != ""){
        VSIUnlink(ustar_png.c_str());
        VSIUnlink((ustar_png + ".aux.xml").c_str());
    }
    if(ustar_legend.c_str() !="")
        VSIUnlink(ustar_legend.c_str());
    #endif
    #ifdef EMISSIONS
    if(dust_png.c_str() != ""){
        VSIUnlink(dust_png.c_str());
        VSIUnlink((dust_png + ".aux.xml").c_str());
    }
    if(dust_legend.c_str() !="")
        VSIUnlink(dust_legend.c_str());
    #endif
    return true;
}

std::string KmlVector::getShortName(std::string file)
{
    return std::string(CPLGetFilename(file.c_str()));
}

/**This function is used during OpenFOAM runs.
 * It is needed because the "dict" files in OpenFOAM can't have quotation marks.
 * @param prj PROJ4 representation of the projection string.
 * @return False on failure to convert, else true.
 */
bool KmlVector::setProj4(std::string prj)
{
	OGRSpatialReference osrs;
	//ret = osrs.importFromProj4(prj.c_str());
	//ret = osrs.SetFromUserInput("+proj=utm +zone=12 +datum=WGS84 +units=m +no_defs");
	//ret = osrs.SetFromUserInput("proj=ut");
	if(osrs.importFromProj4(prj.c_str()) != OGRERR_NONE)
		return false;

	char *wkt = NULL;
	osrs.exportToWkt(&wkt);
	spd.set_prjString(wkt);
	dir.set_prjString(wkt);

	return true;
}
