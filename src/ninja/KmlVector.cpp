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
        coordTransform = NULL;
        turbulenceFlag = false; 
}

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

void KmlVector::setTurbulenceGrid(AsciiGrid<double> &turb, velocityUnits::eVelocityUnits units)
{
	speedUnits = units;
	turbulence = turb;
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

bool KmlVector::makeDefaultStyles(string cScheme, bool vec_scaling)
{

	if(colors)
		delete[]colors;

//    colors[4] = new Style("red", 255, 27, 31, 166, 4.0*arrowWidth); //highest windspeed
//    colors[3] = new Style("orange", 255, 114, 162, 198, 3.0*arrowWidth); //2nd highest
//    colors[2] = new Style("yellow", 255, 216, 204, 222, 1.75*arrowWidth);// moderate
//    colors[1] = new Style("green", 255, 141, 236,229, 1.5*arrowWidth); //moderate low
//    colors[0] = new Style("blue", 255, 229, 243, 239, arrowWidth); //very low

    bool scaling=vec_scaling;

    double blueWidth;
    double greenWidth;
    double yellowWidth;
    double orangeWidth;
    double redWidth;

    if(scaling==true)
    {
        redWidth=4.0*lineWidth;
        orangeWidth=3.0*lineWidth;
        yellowWidth=1.75*lineWidth;
        greenWidth=1.5*lineWidth;
        blueWidth=1.0*lineWidth;
    }
    if(scaling==false)
    {
        redWidth=lineWidth;
        orangeWidth=lineWidth;
        yellowWidth=lineWidth;
        greenWidth=lineWidth;
        blueWidth=lineWidth;
    }

	colors = new Style*[numColors];

    //Alpha, B G R
    if (cScheme=="default")
    {
        colors[0] = new Style("blue", 255, 255, 0, 0, blueWidth);
        colors[1] = new Style("green", 255, 0, 255, 0, greenWidth);
        colors[2] = new Style("yellow", 255, 0, 255, 255, yellowWidth);
        colors[3] = new Style("orange", 255, 0, 127, 255, orangeWidth);
        colors[4] = new Style("red", 255, 0, 0, 255, redWidth);

    }
    if (cScheme=="oranges")
    {
        colors[0] = new Style("blue", 255, 217, 240, 254, blueWidth);
        colors[1] = new Style("green", 255, 138, 204, 253, greenWidth);
        colors[2] = new Style("yellow", 255, 89, 141, 252, yellowWidth);
        colors[3] = new Style("orange", 255, 51, 74, 227, orangeWidth);
        colors[4] = new Style("red", 255, 0, 0, 179, redWidth);

    }
    if(cScheme=="blues")
    {
        colors[0] = new Style("blue", 255, 254, 243, 239, blueWidth);
        colors[1] = new Style("green", 255, 231, 215, 189, greenWidth);
        colors[2] = new Style("yellow", 255, 214, 174, 107, yellowWidth);
        colors[3] = new Style("orange", 255, 189, 130, 49, orangeWidth);
        colors[4] = new Style("red", 255, 156, 81, 8, redWidth);
    }
    if (cScheme=="greens")
    {
        colors[0] = new Style("blue", 255, 233, 248, 237, blueWidth);
        colors[1] = new Style("green", 255, 179, 228, 186, greenWidth);
        colors[2] = new Style("yellow", 255, 118, 196, 116, yellowWidth);
        colors[3] = new Style("orange", 255, 84, 163, 49, orangeWidth);
        colors[4] = new Style("red", 255, 44, 109, 0, redWidth);
    }
    if (cScheme=="pinks")
    {
        colors[0] = new Style("blue", 255, 246, 238, 241, blueWidth);
        colors[1] = new Style("green", 255, 216, 181, 215, greenWidth);
        colors[2] = new Style("yellow", 255, 176, 101, 223, yellowWidth);
        colors[3] = new Style("orange", 255, 119, 28,221, orangeWidth);
        colors[4] = new Style("red", 255, 67, 0, 152, redWidth);
    }
    if (cScheme=="magic_beans")
    {
        colors[4] = new Style("red", 255, 32, 0, 202, redWidth);
        colors[3] = new Style("orange", 255, 130, 165, 244, orangeWidth);
        colors[2] = new Style("yellow", 255, 247, 247, 247, yellowWidth);
        colors[1] = new Style("green", 255, 222, 197,146, greenWidth);
//        colors[0] = new Style("blue", 255, 176, 113, 5, blueWidth); //For some reason
        //Google earth will not render with a Red = 5, works fine with 0 or 30 though...
        colors[0] = new Style("blue", 255, 176, 113, 30, blueWidth);
    }
    if (cScheme=="pink_to_green")
    {
        colors[4] = new Style("red", 255, 148, 50, 123, redWidth);
        colors[3] = new Style("orange", 255, 207, 165, 194, orangeWidth);
        colors[2] = new Style("yellow", 255, 247, 247, 247, yellowWidth);
        colors[1] = new Style("green", 255, 160, 219,166, greenWidth);
        colors[0] = new Style("blue", 255, 55, 136, 0, blueWidth);
    }
    if (cScheme=="ROPGW") //Red Orange Pink Green White
    {// Alpha BGR
        colors[4] = new Style("red", 255, 27, 31, 166, redWidth); //highest windspeed
        colors[3] = new Style("orange", 255, 114, 162, 198, orangeWidth); //2nd highest
        colors[2] = new Style("yellow", 255, 216, 204, 222, yellowWidth);// moderate
        colors[1] = new Style("green", 255, 141, 236,229, greenWidth); //moderate low
        colors[0] = new Style("blue", 255, 229, 243, 239,blueWidth); //very low
    }
    return true;
}

bool KmlVector::setOGR() {
  int rc = OGRERR_NONE;
  if(spd.prjString != "") {
    char *p = strdup(spd.prjString.c_str());
    char *q = p;
    rc = oSourceSRS.importFromWkt(&p);
    free((void*)q);
    if(rc != OGRERR_NONE) {
      throw std::logic_error("cannot create SRS from DEM, kmz creation failed");
    }
    rc = oTargetSRS.importFromEPSG(4326);
    if(rc != OGRERR_NONE) {
      throw std::logic_error("cannot create SRS for EPSG:4326, kmz creation failed");
    }
#ifdef GDAL_COMPUTE_VERSION
#if GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION(3,0,0)
    oTargetSRS.SetAxisMappingStrategy(OAMS_TRADITIONAL_GIS_ORDER);
#endif /* GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION(3,0,0) */
#endif /* GDAL_COMPUTE_VERSION */
    coordTransform = OGRCreateCoordinateTransformation(&oSourceSRS, &oTargetSRS);
    if(coordTransform == NULL) {
      throw std::logic_error("failed to create coordinate transform, kmz creation failed");
    }
    return true;
  }
  throw std::logic_error("failed to setup coordinate transform, kmz creation failed");
  return false;
}


bool KmlVector::writeKml(std::string cScheme, bool vector_scaling)
{
	VSILFILE* fout = 0;
        makeDefaultStyles(cScheme,vector_scaling);
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
            writeScreenOverlayLegend(fout,cScheme);

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


            if(turbulenceFlag)
            {
                VSIFPrintfL(fout, "<Folder>");
                VSIFPrintfL(fout, "\n\t<name>Average Velocity Fluctuations</name>\n");
                writeTurbulence(fout);
                VSIFPrintfL(fout, "</Folder>");
            }

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

bool KmlVector::writeKml(egoogSpeedScaling scaling, string cScheme,bool vector_scaling)
{
	VSILFILE *fout;

    makeDefaultStyles(cScheme,vector_scaling);
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
            writeScreenOverlayLegend(fout,cScheme);
			if(wxModelName.empty())
			    writeScreenOverlayDateTimeLegend(fout);
			else
			    writeScreenOverlayDateTimeLegendWxModelRun(fout);
                        VSIFPrintfL(fout, "<Folder>");
			VSIFPrintfL(fout, "\n\t<name>Wind Speed</name>\n");
			writeVectors(fout);
                        VSIFPrintfL(fout, "</Folder>");


            if(turbulenceFlag)
            {
                VSIFPrintfL(fout, "<Folder>");
                VSIFPrintfL(fout, "\n\t<name>Average Velocity Fluctuations</name>\n");
                writeTurbulence(fout);
                VSIFPrintfL(fout, "</Folder>");
            }
            
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

bool KmlVector::writeScreenOverlayLegend(VSILFILE *fileOut,std::string cScheme)
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
//	RGBApixel red, orange, yellow, green, blue;
    if(cScheme=="default")
    {
        colors[0].Red = 255;//max wind
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

    }
    if (cScheme=="oranges")
    {
        colors[0].Red = 179; //0=Highest wind speed: its reversed from above...
        colors[0].Green = 0;
        colors[0].Blue = 0;
        colors[0].Alpha = 0;

        colors[1].Red = 227;
        colors[1].Green = 74;
        colors[1].Blue = 51;
        colors[1].Alpha = 0;

        colors[2].Red = 252;
        colors[2].Green = 141;
        colors[2].Blue = 89;
        colors[2].Alpha = 0;

        colors[3].Red = 253;
        colors[3].Green = 204;
        colors[3].Blue = 138;
        colors[3].Alpha = 0;

        colors[4].Red = 254;
        colors[4].Green = 240;
        colors[4].Blue = 217;
        colors[4].Alpha = 0;
    }
    if (cScheme=="blues")
    {
        colors[4].Red = 239; //0=Highest wind speed: its reversed from above...
        colors[4].Green = 243;
        colors[4].Blue = 255;
        colors[4].Alpha = 0;

        colors[3].Red = 189;
        colors[3].Green = 215;
        colors[3].Blue = 231;
        colors[3].Alpha = 0;

        colors[2].Red = 107;
        colors[2].Green = 174;
        colors[2].Blue = 214;
        colors[2].Alpha = 0;

        colors[1].Red = 49;
        colors[1].Green = 130;
        colors[1].Blue = 189;
        colors[1].Alpha = 0;

        colors[0].Red = 8;
        colors[0].Green = 81;
        colors[0].Blue = 156;
        colors[0].Alpha = 0;
    }
    if (cScheme=="greens")
    {
        colors[4].Red = 237; //0=Highest wind speed: its reversed from above...
        colors[4].Green = 248;
        colors[4].Blue = 233;
        colors[4].Alpha = 0;

        colors[3].Red = 186;
        colors[3].Green = 228;
        colors[3].Blue = 179;
        colors[3].Alpha = 0;

        colors[2].Red = 116;
        colors[2].Green = 196;
        colors[2].Blue = 118;
        colors[2].Alpha = 0;

        colors[1].Red = 49;
        colors[1].Green = 163;
        colors[1].Blue = 84;
        colors[1].Alpha = 0;

        colors[0].Red = 0;
        colors[0].Green = 109;
        colors[0].Blue = 44;
        colors[0].Alpha = 0;
    }
    if (cScheme=="pinks")
    {
        colors[4].Red = 241; //0=Highest wind speed: its reversed from above...
        colors[4].Green = 238;
        colors[4].Blue = 246;
        colors[4].Alpha = 0;

        colors[3].Red = 215;
        colors[3].Green = 181;
        colors[3].Blue = 216;
        colors[3].Alpha = 0;

        colors[2].Red = 223;
        colors[2].Green = 101;
        colors[2].Blue = 176;
        colors[2].Alpha = 0;

        colors[1].Red = 221;
        colors[1].Green = 28;
        colors[1].Blue = 119;
        colors[1].Alpha = 0;

        colors[0].Red = 152;
        colors[0].Green = 0;
        colors[0].Blue = 67;
        colors[0].Alpha = 0;
    }
    if (cScheme=="magic_beans")
    {
        colors[0].Red = 202; //0=Highest wind speed: its reversed from above...
        colors[0].Green = 0;
        colors[0].Blue = 32;
        colors[0].Alpha = 0;

        colors[1].Red = 244;
        colors[1].Green = 165;
        colors[1].Blue = 130;
        colors[1].Alpha = 0;

        colors[2].Red = 247;
        colors[2].Green = 247;
        colors[2].Blue = 247;
        colors[2].Alpha = 0;

        colors[3].Red = 146;
        colors[3].Green = 197;
        colors[3].Blue = 222;
        colors[3].Alpha = 0;

        colors[4].Red = 5;
        colors[4].Green = 113;
        colors[4].Blue = 176;
        colors[4].Alpha = 0;
    }
    if (cScheme=="pink_to_green")
    {
        colors[0].Red = 123; //0=Highest wind speed: its reversed from above...
        colors[0].Green = 50;
        colors[0].Blue = 148;
        colors[0].Alpha = 0;

        colors[1].Red = 194;
        colors[1].Green = 165;
        colors[1].Blue = 207;
        colors[1].Alpha = 0;

        colors[2].Red = 247;
        colors[2].Green = 247;
        colors[2].Blue = 247;
        colors[2].Alpha = 0;

        colors[3].Red = 146;
        colors[3].Green = 219;
        colors[3].Blue = 160;
        colors[3].Alpha = 0;

        colors[4].Red = 0;
        colors[4].Green = 136;
        colors[4].Blue = 55;
        colors[4].Alpha = 0;
    }
    if (cScheme=="ROPGW")
    {
        colors[0].Red = 166; //0=Highest wind speed: its reversed from above...
        colors[0].Green = 31; //red
        colors[0].Blue = 27;
        colors[0].Alpha = 0;

        colors[1].Red = 198; //orange
        colors[1].Green = 162;
        colors[1].Blue = 114;
        colors[1].Alpha = 0;

        colors[2].Red = 222; //pink
        colors[2].Green = 204;
        colors[2].Blue = 216;
        colors[2].Alpha = 0;

        colors[3].Red = 229; //green
        colors[3].Green = 236;
        colors[3].Blue = 141;
        colors[3].Alpha = 0;

        colors[4].Red = 239;
        colors[4].Green = 243;//White
        colors[4].Blue = 229;
        colors[4].Alpha = 0;
    }

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
		case velocityUnits::knots:	// kts
			PrintString(legend,"Wind Speed (knots)", titleX, titleY, titleTextHeight, white);
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

bool KmlVector::writeTurbulence(FILE *fileOut)
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

	turbulence_png = "turbulence_png.png";

	cSize = turbulence.get_cellSize();
	nR = turbulence.get_nRows();
	nC = turbulence.get_nCols();

	lower = turbulence.get_minValue();
	upper = turbulence.get_maxValue();
	lower_mid = lower + (turbulence.get_maxValue() - turbulence.get_minValue())/4;
	upper_mid = upper - (turbulence.get_maxValue() - turbulence.get_minValue())/4;
	mid = upper_mid - (turbulence.get_maxValue() - turbulence.get_minValue())/4;

    //---------------make single png for overlay------------------
    std::string outFilename = "turbulence_png.png";
    std::string scalarLegendFilename = "turbulence_legend";
    std::string legendTitle = "Speed Fluctuation";
    std::string legendUnits = "";
    switch(speedUnits)
    {
            case velocityUnits::metersPerSecond:	// m/s
                    legendUnits = "(m/s)";
                    break;
            case velocityUnits::milesPerHour:		// mph
                    legendUnits = "(mph)";
                    break;
            case velocityUnits::kilometersPerHour:	// kph
                    legendUnits = "(kph)";
                    break;
            case velocityUnits::knots:	// kts
                    legendUnits = "(knots)";
        break;
            default:				// default is mph
                    legendUnits = "(mph)";
                    break;
    }
    bool writeLegend = TRUE;

    turbulence.ascii2png(outFilename, legendTitle, legendUnits, scalarLegendFilename, writeLegend);

    turbulence.get_cellPosition(0, 0, &xCenter, &yCenter); //sw
    left_x = xCenter - cSize/2; //west
    lower_y = yCenter - cSize/2; //south
    turbulence.get_cellPosition(nR-1, nC-1, &xCenter, &yCenter); //ne
    right_x = xCenter + cSize/2; //east
    upper_y = yCenter + cSize/2;  //north

	coordTransform->Transform(1, &right_x, &upper_y);
	coordTransform->Transform(1, &left_x, &lower_y);
	coordTransform->Transform(1, &xCenter, &yCenter);

	int pos;
	std::string shortName;
	pos = turbulence_png.find_last_of('\\');
	if(pos == -1)
	  pos = turbulence_png.find_last_of('/');

	shortName = turbulence_png.substr(pos + 1, turbulence_png.size());

	VSIFPrintfL(fileOut, "<GroundOverlay>");
	VSIFPrintfL(fileOut, "\n\t<name>Average Velocity Fluctuations</name>");
	VSIFPrintfL(fileOut, "\n\t<ExtendedData>");
	VSIFPrintfL(fileOut, "\n\t\t<Data name=\"Turbulence\">");
	VSIFPrintfL(fileOut, "\n\t\t\t<value>2</value>");
	VSIFPrintfL(fileOut, "\n\t\t</Data>");
	VSIFPrintfL(fileOut, "\n\t</ExtendedData>");

	VSIFPrintfL(fileOut, "\n\t<altitude>0</altitude>");
	VSIFPrintfL(fileOut, "\n\t<altitudeMode>clampToGround</altitudeMode>");
	//VSIFPrintfL(fileOut, "\n\t\t<color>88ffffff</color>");

	VSIFPrintfL(fileOut, "\n\t<Icon>");
	VSIFPrintfL(fileOut, "\n\t\t<href>%s</href>", shortName.c_str());  //turbulence_png.png
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
        turbulence_legend = "./turbulence_legend";
	//int pos;
	//std::string shortName;
	pos = turbulence_legend.find_last_of('\\');
	if(pos == -1)
	  pos = turbulence_legend.find_last_of('/');

	shortName = turbulence_legend.substr(pos + 1, turbulence_legend.size());

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

  if(turbulenceFlag)
  {
      filesToZip.push_back(turbulence_png);
      filesToZip.push_back(turbulence_legend);
      filesInZip.push_back(getShortName(turbulence_png));
      filesInZip.push_back(getShortName(turbulence_legend));
  }

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
    if(turbulence_png.c_str() != ""){
        VSIUnlink(turbulence_png.c_str());
        VSIUnlink((turbulence_png + ".aux.xml").c_str());
    }
    if(turbulence_legend.c_str() !="")
        VSIUnlink(turbulence_legend.c_str());
    
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
