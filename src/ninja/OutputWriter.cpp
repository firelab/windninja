/******************************************************************************
*
* Filename: OutputWriter.cpp
*
* Project:  WindNinja
* Purpose:  Class to handle output of WindNinja simulations to various GDAL
*           formats
* Author:   Levi Malott, lmnn3@mst.edu
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

#include "OutputWriter.h"

const char * OutputWriter::NAME      = "name";
const char * OutputWriter::SPEED     = "speed";
const char * OutputWriter::DIR       = "dir";
const char * OutputWriter::AV_DIR    = "AV_dir";
const char * OutputWriter::AM_DIR    = "AM_dir";
const char * OutputWriter::QGIS_DIR  = "QGIS_dir";

const double OutputWriter::BOTTOM_MARGIN = 1.5;
const double OutputWriter::TOP_MARGIN = 0.5;
const double OutputWriter::SIDE_MARGIN = 0.5;


OutputWriter::OutputWriter ()
    :ninjaTime(boost::local_time::not_a_date_time)
{
    hSrcDS        = NULL;
    hDstDS        = NULL;
    hDriver       = NULL;
    pafScanline   = NULL;
    hLayer        = NULL;
    hFieldDefn    = NULL;
    hDataSource   = NULL;
    hOGRDriver    = NULL;
    papszOptions  = NULL;
    hSrcSRS       = NULL;
    hDestSRS      = NULL;
    hTransform    = NULL;
    colors        = NULL;
    split_vals    = NULL;
    linewidth     = 1.0;

    angleFromNorth = 0.0;

    speedScaling = equal_interval;
    colorScheme = "default";
    useVectorScaling = false;

    pszOgrFile    = NULL;
    pszLegendFile = NULL;
    pszTmpDemFile = NULL;

    _createTmpFiles();

    _createDefaultStyles();

}  /* -----  end of method OutputWriter::OutputWriter  (constructor)  ----- */


OutputWriter::~OutputWriter ()
{
    OSRDestroySpatialReference( hSrcSRS );
    OSRDestroySpatialReference( hDestSRS );
    OCTDestroyCoordinateTransformation( hTransform );

    _destroyDefaultStyles();
    _deleteSplits();
    _destroyOptions();
    _closeOGRFile();
    _closeDataSets();

    _deleteTmpFiles();
    return;
}  /* -----  end of method OutputWriter::~OutputWriter  ----- */

bool OutputWriter::_createTmpFiles()
{
    const char *pszTmp = NULL;
    pszTmp = CPLGenerateTempFilename( NULL );
    pszTmp = CPLFormFilename( NULL, pszTmp, ".shp" );
    pszOgrFile = CPLStrdup( pszTmp );
    CPLDebug( "NINJA", "Using %s for pdf ogr datasource", pszOgrFile );

    pszTmp = CPLGenerateTempFilename( NULL );
    pszTmp = CPLFormFilename( NULL, pszTmp, ".bmp" );
    pszLegendFile = CPLStrdup( pszTmp );
    CPLDebug( "NINJA", "Using %s for pdf legend dataset", pszLegendFile );

    pszTmp = CPLGenerateTempFilename( NULL );
    pszTmp = CPLFormFilename( NULL, pszTmp, ".bmp" );
    pszDateTimeLegendFile = CPLStrdup( pszTmp );
    CPLDebug( "NINJA", "Using %s for date time legend dataset", pszDateTimeLegendFile );

    return true;
}

void OutputWriter::_deleteTmpFiles()
{
    CPLFree( (void*)pszOgrFile );
    CPLFree( (void*)pszLegendFile );
    CPLFree( (void*)pszDateTimeLegendFile) ;
    if( pszTmpDemFile != NULL )
    {
        GDALDriverH hDrv = GDALGetDriverByName( "GTiff" );
        assert( hDrv );
        GDALDeleteDataset( hDrv, pszTmpDemFile );
    }
    CPLFree( (void*)pszTmpDemFile );
}

void OutputWriter::_createDefaultStyles()
{
    double   blueWidth;
    double  greenWidth;
    double yellowWidth;
    double orangeWidth;
    double    redWidth;
    if(useVectorScaling == true)
    {
           redWidth =  4.0*linewidth;
        orangeWidth =  3.0*linewidth;
        yellowWidth = 1.75*linewidth;
         greenWidth =  1.5*linewidth;
          blueWidth =  1.0*linewidth;
    }
    else
    {
           redWidth = linewidth;
        orangeWidth = linewidth;
        yellowWidth = linewidth;
         greenWidth = linewidth;
          blueWidth = linewidth;
    }

    colors    = new Style*[ NCOLORS ];

    // Alpha, B G R
    if(colorScheme == "default")
    {
        colors[0] = new Style(  "blue", 255, 255,   0,   0,   blueWidth);
        colors[1] = new Style( "green", 255,   0, 255,   0,  greenWidth);
        colors[2] = new Style("yellow", 255,   0, 255, 255, yellowWidth);
        colors[3] = new Style("orange", 255,   0, 127, 255, orangeWidth);
        colors[4] = new Style(   "red", 255,   0,   0, 255,    redWidth);
    }
    if(colorScheme == "oranges")
    {
        colors[0] = new Style(  "blue", 255, 217, 240, 254,   blueWidth);
        colors[1] = new Style( "green", 255, 138, 204, 253,  greenWidth);
        colors[2] = new Style("yellow", 255,  89, 141, 252, yellowWidth);
        colors[3] = new Style("orange", 255,  51,  74, 227, orangeWidth);
        colors[4] = new Style(   "red", 255,   0,   0, 179,    redWidth);
    }
    if(colorScheme == "blues")
    {
        colors[0] = new Style(  "blue", 255, 254, 243, 239,   blueWidth);
        colors[1] = new Style( "green", 255, 231, 215, 189,  greenWidth);
        colors[2] = new Style("yellow", 255, 214, 174, 107, yellowWidth);
        colors[3] = new Style("orange", 255, 189, 130,  49, orangeWidth);
        colors[4] = new Style(   "red", 255, 156,  81,   8,    redWidth);
    }
    if(colorScheme == "greens")
    {
        colors[0] = new Style(  "blue", 255, 233, 248, 237,   blueWidth);
        colors[1] = new Style( "green", 255, 179, 228, 186,  greenWidth);
        colors[2] = new Style("yellow", 255, 118, 196, 116, yellowWidth);
        colors[3] = new Style("orange", 255,  84, 163,  49, orangeWidth);
        colors[4] = new Style(   "red", 255,  44, 109,   0,    redWidth);
    }
    if(colorScheme == "pinks")
    {
        colors[0] = new Style(  "blue", 255, 246, 238, 241,   blueWidth);
        colors[1] = new Style( "green", 255, 216, 181, 215,  greenWidth);
        colors[2] = new Style("yellow", 255, 176, 101, 223, yellowWidth);
        colors[3] = new Style("orange", 255, 119,  28, 221, orangeWidth);
        colors[4] = new Style(   "red", 255,  67,   0, 152,    redWidth);
    }
    if(colorScheme == "magic_beans")
    {
        colors[4] = new Style(   "red", 255,  32,   0, 202,    redWidth);
        colors[3] = new Style("orange", 255, 130, 165, 244, orangeWidth);
        colors[2] = new Style("yellow", 255, 247, 247, 247, yellowWidth);
        colors[1] = new Style( "green", 255, 222, 197, 146,  greenWidth);
        ////colors[0] = new Style("blue", 255, 176, 113, 5, blueWidth); // for some reason
        ////google earth will not render with a red = 5, works fine with 0 or 30 though...
        colors[0] = new Style(  "blue", 255, 176, 113,  30,   blueWidth);
    }
    if(colorScheme == "pink_to_green")
    {
        colors[4] = new Style(   "red", 255, 148,  50, 123,    redWidth);
        colors[3] = new Style("orange", 255, 207, 165, 194, orangeWidth);
        colors[2] = new Style("yellow", 255, 247, 247, 247, yellowWidth);
        colors[1] = new Style( "green", 255, 160, 219, 166,  greenWidth);
        colors[0] = new Style(  "blue", 255,  55, 136,   0,   blueWidth);
    }
    if(colorScheme == "ROPGW") // Red Orange Pink Green White
    {
        colors[4] = new Style(   "red", 255,  27,  31, 166,    redWidth);  // highest windspeed
        colors[3] = new Style("orange", 255, 114, 162, 198, orangeWidth);  // 2nd highest
        colors[2] = new Style("yellow", 255, 216, 204, 222, yellowWidth);  // moderate
        colors[1] = new Style( "green", 255, 141, 236, 229,  greenWidth);  // moderate low
        colors[0] = new Style(  "blue", 255, 229, 243, 239,   blueWidth);  // very low
    }

    return;
}
void OutputWriter::_destroyDefaultStyles()
{
    if( NULL != colors )
    {
        for( int i = 0; i < NCOLORS; i ++ )
        {
            delete colors[i];
        }
        delete [] colors;
    }
    colors = NULL;
}

void OutputWriter::setLineWidth( const float w )
{
   linewidth = ( w >= 0.0f ) ? w : -w;
   if( areEqual( linewidth, 0.0f ) )
   {
       linewidth = 1.0f;
   }
   _destroyDefaultStyles();
   _createDefaultStyles();
}

void OutputWriter::setDPI( const unsigned short d )
{
    dpi = d;
}

void OutputWriter::setSize( const double w, const double h )
{
    width = w;
    height = h;
}

void OutputWriter::setColorScheme(std::string cScheme)
{
    if(cScheme.compare("default") != 0 && cScheme.compare("oranges") != 0 && cScheme.compare("blues") != 0 && cScheme.compare("greens") != 0 && cScheme.compare("pinks") != 0 && cScheme.compare("magic_beans") != 0 && cScheme.compare("pink_to_green") != 0 && cScheme.compare("ROPGW") != 0)
    {
        throw std::runtime_error("Invalid input cScheme '"+cScheme+"' in OutputWriter:setColorScheme()\nvalid choices are: 'default', 'oranges', 'blues', 'greens', 'pinks', 'magic_beans', 'pink_to_green', 'ROPGW'");
    }

    colorScheme = cScheme;
}

#ifdef EMISSIONS
void OutputWriter::setDustGrid(AsciiGrid<double> &d)
{
    dust = d;
    return;
}  /* -----  end of method OutputWriter::setDustGrid  ----- */
#endif

void OutputWriter::setSpeedGrid(AsciiGrid<double> &s, velocityUnits::eVelocityUnits u)
{
    spd = s;
    units = u;

    for (int i = 0; i < spd.get_nRows(); ++i)
    {
        for (int j = 0; j < spd.get_nCols(); ++j)
        {
            if (spd(i, j) > 1e36)
            {
                spd(i, j) = spd.get_noDataValue();
            }
        }
    }

    return;
}  /* -----  end of method OutputWriter::setSpeedGrid  ----- */

void OutputWriter::setAngleFromNorth(const double angFromNorth)
{
    angleFromNorth = angFromNorth;
}

void OutputWriter::setDirGrid(AsciiGrid<double> &d)
{
    dir = d;
    return;
}  /* -----  end of method OutputWriter::setDirGrid  ----- */

void OutputWriter::setSplitVals(const double *splitVals, const unsigned short size)
{
    if(size != NSPLITS)
    {
        throw std::runtime_error("OutputWriter::setSplitVals() input array size does not match OutputWriter numSplits!!");
    }

    _deleteSplits();

    split_vals = new double[NSPLITS];
    for(int i = 0; i < NSPLITS; i++)
    {
        split_vals[i] = splitVals[i];
    }
}

bool OutputWriter::write(std::string outputFilename, std::string driver)
{
    if( 0 == driver.compare( "PDF" ) )
    {
        _writePDF(outputFilename);
    }
    else if ( 0 == driver.compare( "FlatGeoBufZip" ) )
    {
        _writeFlatGeoBufZip(outputFilename);
    }
    else
    {
        throw std::runtime_error("OutputWriter: unrecognized output format");
    }
    return true;
}  /* -----  end of method OutputWriter::write  ----- */

std::string OutputWriter::_getStyleFromSpeed( const double & spd )
{
    std::string style = "none";

    for(int i = 1; i <= NCOLORS; i++)
    {
        if( spd <= split_vals[i] )
        {
            style = colors[i-1]->asOGRStyleString();
            break;
        }
    }
    if( "none" ==  style)
    {
        style = colors[NCOLORS - 1]->asOGRStyleString();
    }
    return style;
}

void OutputWriter::_closeDataSets()
{
    if( NULL != hSrcDS )
    {
        GDALClose(hSrcDS);
        hSrcDS = NULL;
    }
    if( NULL != hDstDS )
    {
        GDALClose(hDstDS);
        hDstDS = NULL;
    }
    _closeOGRFile();
}

void OutputWriter::_destroyOptions()
{
    if( NULL != papszOptions )
    {
        CSLDestroy( papszOptions );
        papszOptions = NULL;
    }
}

void OutputWriter::_closeOGRFile()
{
    if( NULL != hDataSource )
    {
        GDALClose( hDataSource );
        hDataSource = NULL;
    }
}

void OutputWriter::_deleteSplits()
{
    if( NULL != split_vals )
    {
        delete [] split_vals;
        split_vals = NULL;
    }
}

void OutputWriter::_createSplits()
{
    // only create them if they haven't already been set/created,
    // so that setSplitVals() gets precedence over just creating the values
    if(split_vals != NULL)
    {
        return;
    }

    split_vals = new double[NSPLITS];

    switch(speedScaling)
    {
        case equal_color:  // divide legend speeds using equal color method (equal numbers of arrows for each color)
        {
            spd.divide_gridData(split_vals, NSPLITS);
            break;
        }
        case equal_interval:  // divide legend speeds using equal interval method (speed breaks divided equally over speed range)
        {
            double interval = spd.get_maxValue() / (float)(NSPLITS-1);
            //double interval = (spd.get_maxValue() - spd.get_minValue()) / (float)(NSPLITS-1);
            for(int i = 0; i < NSPLITS; i++)
            {
                split_vals[i] = i * interval;
                //split_vals[i] = i * interval + spd.get_minValue();
            }
            break;
        }
        default:  // divide legend speeds using equal color method (equal numbers of arrows for each color)
        {
            spd.divide_gridData(split_vals, NSPLITS);
            break;
        }
    }
}

bool OutputWriter::_createLegend()
{
    //make bitmap
    int legendWidth = LGND_WIDTH;
    int legendHeight = LGND_HEIGHT;
    BMP legend;

    std::string legendStrings[NCOLORS];
    ostringstream os;

    double maxxx = spd.get_maxValue();

    for(int i = 0;i < NCOLORS; i++)
    {
        os << setiosflags(ios::fixed) << setiosflags(ios::showpoint) << setprecision(2);
        if(i == 0)
        {
            //os << split_vals[NSPLITS-2] << " + ";
            os << split_vals[NSPLITS-2] << " - " << split_vals[NSPLITS-1];
        }
        else if(i == NCOLORS-1)
        {
            os << "0.00 - " << split_vals[1] - 0.01;
            //os << split_vals[0] << " - " << split_vals[1] - 0.01;
        }
        else
        {
            os << split_vals[NSPLITS - i - 2] << " - " << split_vals[NSPLITS - i - 1] - 0.01;
        }

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
    RGBApixel white;
    white.Red = 255;
    white.Green = 255;
    white.Blue = 255;
    white.Alpha = 0;

    RGBApixel lcolors[NCOLORS];
    //RGBApixel red, orange, yellow, green, blue;
    if(colorScheme == "default")
    {
        lcolors[0].Red = 255; //max wind
        lcolors[0].Green = 0;
        lcolors[0].Blue = 0;
        lcolors[0].Alpha = 0;

        lcolors[1].Red = 255;
        lcolors[1].Green = 127;
        lcolors[1].Blue = 0;
        lcolors[1].Alpha = 0;

        lcolors[2].Red = 255;
        lcolors[2].Green = 255;
        lcolors[2].Blue = 0;
        lcolors[2].Alpha = 0;

        lcolors[3].Red = 0;
        lcolors[3].Green = 255;
        lcolors[3].Blue = 0;
        lcolors[3].Alpha = 0;

        lcolors[4].Red = 0;
        lcolors[4].Green = 0;
        lcolors[4].Blue = 255;
        lcolors[4].Alpha = 0;
    }
    if(colorScheme == "oranges")
    {
        lcolors[0].Red = 179; //0=Highest wind speed: its reversed from above...
        lcolors[0].Green = 0;
        lcolors[0].Blue = 0;
        lcolors[0].Alpha = 0;

        lcolors[1].Red = 227;
        lcolors[1].Green = 74;
        lcolors[1].Blue = 51;
        lcolors[1].Alpha = 0;

        lcolors[2].Red = 252;
        lcolors[2].Green = 141;
        lcolors[2].Blue = 89;
        lcolors[2].Alpha = 0;

        lcolors[3].Red = 253;
        lcolors[3].Green = 204;
        lcolors[3].Blue = 138;
        lcolors[3].Alpha = 0;

        lcolors[4].Red = 254;
        lcolors[4].Green = 240;
        lcolors[4].Blue = 217;
        lcolors[4].Alpha = 0;
    }
    if(colorScheme == "blues")
    {
        lcolors[4].Red = 239; //0=Highest wind speed: its reversed from above...
        lcolors[4].Green = 243;
        lcolors[4].Blue = 255;
        lcolors[4].Alpha = 0;

        lcolors[3].Red = 189;
        lcolors[3].Green = 215;
        lcolors[3].Blue = 231;
        lcolors[3].Alpha = 0;

        lcolors[2].Red = 107;
        lcolors[2].Green = 174;
        lcolors[2].Blue = 214;
        lcolors[2].Alpha = 0;

        lcolors[1].Red = 49;
        lcolors[1].Green = 130;
        lcolors[1].Blue = 189;
        lcolors[1].Alpha = 0;

        lcolors[0].Red = 8;
        lcolors[0].Green = 81;
        lcolors[0].Blue = 156;
        lcolors[0].Alpha = 0;
    }
    if(colorScheme == "greens")
    {
        lcolors[4].Red = 237; //0=Highest wind speed: its reversed from above...
        lcolors[4].Green = 248;
        lcolors[4].Blue = 233;
        lcolors[4].Alpha = 0;

        lcolors[3].Red = 186;
        lcolors[3].Green = 228;
        lcolors[3].Blue = 179;
        lcolors[3].Alpha = 0;

        lcolors[2].Red = 116;
        lcolors[2].Green = 196;
        lcolors[2].Blue = 118;
        lcolors[2].Alpha = 0;

        lcolors[1].Red = 49;
        lcolors[1].Green = 163;
        lcolors[1].Blue = 84;
        lcolors[1].Alpha = 0;

        lcolors[0].Red = 0;
        lcolors[0].Green = 109;
        lcolors[0].Blue = 44;
        lcolors[0].Alpha = 0;
    }
    if(colorScheme == "pinks")
    {
        lcolors[4].Red = 241; //0=Highest wind speed: its reversed from above...
        lcolors[4].Green = 238;
        lcolors[4].Blue = 246;
        lcolors[4].Alpha = 0;

        lcolors[3].Red = 215;
        lcolors[3].Green = 181;
        lcolors[3].Blue = 216;
        lcolors[3].Alpha = 0;

        lcolors[2].Red = 223;
        lcolors[2].Green = 101;
        lcolors[2].Blue = 176;
        lcolors[2].Alpha = 0;

        lcolors[1].Red = 221;
        lcolors[1].Green = 28;
        lcolors[1].Blue = 119;
        lcolors[1].Alpha = 0;

        lcolors[0].Red = 152;
        lcolors[0].Green = 0;
        lcolors[0].Blue = 67;
        lcolors[0].Alpha = 0;
    }
    if(colorScheme == "magic_beans")
    {
        lcolors[0].Red = 202; //0=Highest wind speed: its reversed from above...
        lcolors[0].Green = 0;
        lcolors[0].Blue = 32;
        lcolors[0].Alpha = 0;

        lcolors[1].Red = 244;
        lcolors[1].Green = 165;
        lcolors[1].Blue = 130;
        lcolors[1].Alpha = 0;

        lcolors[2].Red = 247;
        lcolors[2].Green = 247;
        lcolors[2].Blue = 247;
        lcolors[2].Alpha = 0;

        lcolors[3].Red = 146;
        lcolors[3].Green = 197;
        lcolors[3].Blue = 222;
        lcolors[3].Alpha = 0;

        lcolors[4].Red = 5;
        lcolors[4].Green = 113;
        lcolors[4].Blue = 176;
        lcolors[4].Alpha = 0;
    }
    if(colorScheme == "pink_to_green")
    {
        lcolors[0].Red = 123; //0=Highest wind speed: its reversed from above...
        lcolors[0].Green = 50;
        lcolors[0].Blue = 148;
        lcolors[0].Alpha = 0;

        lcolors[1].Red = 194;
        lcolors[1].Green = 165;
        lcolors[1].Blue = 207;
        lcolors[1].Alpha = 0;

        lcolors[2].Red = 247;
        lcolors[2].Green = 247;
        lcolors[2].Blue = 247;
        lcolors[2].Alpha = 0;

        lcolors[3].Red = 146;
        lcolors[3].Green = 219;
        lcolors[3].Blue = 160;
        lcolors[3].Alpha = 0;

        lcolors[4].Red = 0;
        lcolors[4].Green = 136;
        lcolors[4].Blue = 55;
        lcolors[4].Alpha = 0;
    }
    if(colorScheme == "ROPGW")
    {
        lcolors[0].Red = 166; //0=Highest wind speed: its reversed from above...
        lcolors[0].Green = 31; //red
        lcolors[0].Blue = 27;
        lcolors[0].Alpha = 0;

        lcolors[1].Red = 198; //orange
        lcolors[1].Green = 162;
        lcolors[1].Blue = 114;
        lcolors[1].Alpha = 0;

        lcolors[2].Red = 222; //pink
        lcolors[2].Green = 204;
        lcolors[2].Blue = 216;
        lcolors[2].Alpha = 0;

        lcolors[3].Red = 229; //green
        lcolors[3].Green = 236;
        lcolors[3].Blue = 141;
        lcolors[3].Alpha = 0;

        lcolors[4].Red = 239; //White
        lcolors[4].Green = 243;
        lcolors[4].Blue = 229;
        lcolors[4].Alpha = 0;
    }

    int arrowLength = 40;  //pixels;
    int arrowHeadLength = 10; // pixels;
    int textHeight = 10;  //pixels- 8 for maximum speed of "999.99 - 555.55";
                          //10 for normal double digits
    if(split_vals[NSPLITS-1] >= 100)
        textHeight = 8;
    int titleTextHeight = int(1.2 * textHeight);
    int titleX, titleY;

    int x1, x2, x3, x4;
    double x;
    int y1, y2, y3, y4;
    double y;

    int textX;
    int textY;

    x = 0.05;
    y = 0.30;


    titleX = x * legendWidth;
    titleY = (y / 3) * legendHeight;


    //TODO: Add support for configuring wind speed units
    std::string unitsText = "";
    if(units == velocityUnits::metersPerSecond){
        unitsText = "m/s";
    }
    else if(units == velocityUnits::milesPerHour){
        unitsText = "mph";
    }
    else if(units == velocityUnits::kilometersPerHour){
        unitsText = "kph";
    }
    else if(units == velocityUnits::knots){
        unitsText = "knots";
    }
    else{
        throw std::runtime_error("OutputWriter: velocityUnits set incorrectly.");
    }
    PrintString(legend, CPLSPrintf("Wind Speed (%s)", unitsText.c_str()), titleX, titleY, titleTextHeight, white);
    for(int i = 0;i < NCOLORS;i++)
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


        DrawLine(legend, x1, y1, x2, y2, lcolors[i]);
        DrawLine(legend, x2, y2, x3, y3, lcolors[i]);
        DrawLine(legend, x2, y2, x4, y4, lcolors[i]);

        PrintString(legend, legendStrings[i].c_str(), textX, textY, textHeight, white);


        y += 0.15;
    }

    legend.WriteToFile( pszLegendFile );

    return true;

}

void OutputWriter::_destroyLegend()
{
    GDALDriverH hLegendDrv = GDALGetDriverByName( "BMP" );
    GDALDeleteDataset( hLegendDrv, pszLegendFile );
    return;
}


bool OutputWriter::_createDateTimeLegend(bool wxModel)
{
    //make bitmap
    int legendWidth = 11.25 * wxModelName.size();
    if(legendWidth < 285)
    {
        legendWidth = 285;
    }
    int legendHeight = 52;
    if(wxModel)
    {
        legendHeight = 78;
    }
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

    int textHeight = 12; //pixels- 10 for maximum speed of "999.99 - 555.55";
    //12 for normal double digits
    int titleX, titleY;

    double x;
    double y;

    //print date
    x = 0.05;
    y = 0.15;
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

    os << ninjaTime;

    if(wxModel)
    {
        //print wxModel identifier
        PrintString(legend, wxModelName.c_str(), titleX, titleY, textHeight, white);

        //print date
        x = 0.05;
        y = 0.40;
        titleX = x * legendWidth;
        titleY = y * legendHeight;
        PrintString(legend, os.str().c_str(), titleX, titleY, textHeight, white);

        //prep to print time
        x = 0.05;
        y = 0.70;
    }
    else
    {
        //print date
        PrintString(legend, os.str().c_str(), titleX, titleY, textHeight, white);

        //prep to print time
        x = 0.05;
        y = 0.60;
    }

    //print time

    titleX = x * legendWidth;
    titleY = y * legendHeight;

    os.str("");
    //timeOutputFacet->format("%H:%M %z (%Q from UTC)");
    timeOutputFacet->format("%H:%M %z (");

    os << ninjaTime;

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

    os << ninjaTime.utc_time();

    timeStringLegend.append(os.str());

    PrintString(legend,timeStringLegend.c_str(), titleX, titleY, textHeight, white);

    legend.WriteToFile(pszDateTimeLegendFile);

    std::string shortName;
    shortName = CPLGetFilename(pszDateTimeLegendFile);

    return true;
}

void OutputWriter::_destroyDateTimeLegend()
{
    GDALDriverH hLegendDrv = GDALGetDriverByName( "BMP" );
    GDALDeleteDataset( hLegendDrv, pszDateTimeLegendFile );
    return;
}

void OutputWriter::_openSrcDataSet()
{
    hSrcDS = GDALOpen( demFile.c_str(), GA_ReadOnly );
    if( NULL == hSrcDS )
    {
        throw std::runtime_error("OutputWriter: Failed to open PDF base DEM");
    }
    return;
}

/* --------------------------------------------------------------------------*/
/**
 * @brief Creates an OGR datasource for holding vector features
 *
 * @pre base DEM file is specified
 * @pre speed grid is instantiated
 *
 * @post hTransform contains coordinate transformation from speed grid to raster SRS
 * @post The source raster datasource is open
 * @post The OGR datasource is populated with features from simulation data
 */
/* ----------------------------------------------------------------------------*/
void OutputWriter::_createOGRFile(bool outputLatLon)
{
    int ncols = spd.get_nCols();
    int nrows = spd.get_nRows();
    double x  = 0, y = 0;

    const char* pszSrcWkt = spd.prjString.c_str();

    hSrcSRS = OSRNewSpatialReference(pszSrcWkt);

    if(outputLatLon)
    {
        hDestSRS = OSRNewSpatialReference(NULL);
        OSRImportFromEPSG(hDestSRS, 4326);
    }
    else
    {
        _openSrcDataSet();

        const char* pszDstWkt = GDALGetProjectionRef(hSrcDS);

        hDestSRS = OSRNewSpatialReference(pszDstWkt);

        GDALGetGeoTransform(hSrcDS, adfGeoTransform);

        GDALClose(hSrcDS);
    }

    OSRSetAxisMappingStrategy(hSrcSRS, OAMS_TRADITIONAL_GIS_ORDER);
    OSRSetAxisMappingStrategy(hDestSRS, OAMS_TRADITIONAL_GIS_ORDER);

    hTransform = OCTNewCoordinateTransformation(hSrcSRS, hDestSRS);

    if( NULL == hTransform )
    {
        throw std::runtime_error("OutputWriter: Failed to create coordinate" \
                                 "transformation for PDF output");
    }
    hOGRDriver = OGRGetDriverByName( "ESRI Shapefile" );
    if( NULL == hOGRDriver )
    {
        throw std::runtime_error("OutputWriter: Failed to get OGR Memory driver");
    }

    hDataSource = OGR_Dr_CreateDataSource( hOGRDriver, pszOgrFile, NULL );
    if( NULL == hDataSource )
    {
        throw std::runtime_error("OutputWriter: Failed to create OGR Memory datasource");
    }

    //Create a new layer for the wind features
    hLayer = OGR_DS_CreateLayer( hDataSource, "Wind Vectors" , hDestSRS,
                                 wkbLineString, NULL );
    if( hLayer == NULL )
    {
        throw std::runtime_error("OutputWriter: Failed to create wind vector layer");
    }


    hFieldDefn = OGR_Fld_Create( NAME, OFTString );
    OGR_Fld_SetWidth( hFieldDefn, 32 );
    if( OGRERR_NONE != OGR_L_CreateField( hLayer, hFieldDefn, TRUE ) )
    {
        throw std::runtime_error("OutputWriter: Creating NAME field failed");
    }
    OGR_Fld_Destroy(hFieldDefn);

    hFieldDefn = OGR_Fld_Create( SPEED, OFTReal );
    if( OGRERR_NONE != OGR_L_CreateField( hLayer, hFieldDefn, TRUE ) )
    {
        throw std::runtime_error("OutputWriter: Creating SPEED field failed");
    }
    OGR_Fld_Destroy(hFieldDefn);

    hFieldDefn = OGR_Fld_Create( DIR, OFTInteger );
    if( OGRERR_NONE != OGR_L_CreateField( hLayer, hFieldDefn, TRUE ) )
    {
        throw std::runtime_error("OutputWriter: Create DIR field failed");
    }

    OGR_Fld_Destroy(hFieldDefn);

    hFieldDefn = OGR_Fld_Create( "OGR_STYLE", OFTString );
    OGR_Fld_SetWidth( hFieldDefn, 32 );
    if( OGRERR_NONE != OGR_L_CreateField( hLayer, hFieldDefn, TRUE ) )
    {
        throw std::runtime_error("OutputWriter: Creating OGR_STYLE field failed");
    }
    OGR_Fld_Destroy(hFieldDefn);

    //Add the features to the OGR datasource given by the dir and spd grids
    std::string style;
    for( int i = 0; i < nrows; i++ )
    {
        for( int j = 0; j < ncols; j++ )
        {
            if(spd(i,j) != spd.get_noDataValue() && dir(i,j) != dir.get_noDataValue())
            {
                OGRFeatureH hFeature = OGR_F_Create( OGR_L_GetLayerDefn( hLayer ) );
                OGRGeometryH hLine   = OGR_G_CreateGeometry( wkbLineString );
                WN_Arrow     arrow;

                double dir_prj = dir(i,j);
                double dir_geo = dir_prj;
                if(outputLatLon == true)
                {
                    dir_geo = wrap0to360(dir_prj + angleFromNorth); //convert FROM projected TO geographic coordinates
                }

                spd.get_cellPosition(i, j, &x, &y);
                arrow = WN_Arrow( x, y, spd(i,j), dir_prj, spd.get_cellSize(),
                                  split_vals, NCOLORS);

                std::ostringstream os;
                os << "Cell " << i << "," << j;
                std::string name = os.str();
                OGR_F_SetFieldString( hFeature, OGR_F_GetFieldIndex(hFeature, NAME),
                                      name.c_str() );
                OGR_F_SetFieldInteger( hFeature, OGR_F_GetFieldIndex(hFeature, SPEED),
                                       spd(i,j) );
                OGR_F_SetFieldInteger( hFeature, OGR_F_GetFieldIndex(hFeature, DIR),
                                       (int)dir_geo+0.5);


                arrow.asGeometry( hLine );
                OGR_G_Transform( hLine, hTransform );
                OGR_F_SetGeometry( hFeature, hLine );

                style = _getStyleFromSpeed( spd(i,j) );

                OGR_F_SetStyleString( hFeature, style.c_str() );
                OGR_F_SetFieldString( hFeature, OGR_F_GetFieldIndex(hFeature, "OGR_STYLE"),
                                      style.c_str() );


                if( OGR_L_CreateFeature( hLayer, hFeature ) != OGRERR_NONE )
                {
                    throw std::runtime_error("OutputWriter: error creating features");
                }
                OGR_G_DestroyGeometry( hLine );
                OGR_F_Destroy( hFeature );
            }
        }
    }

    return ;

}  /* -----  end of method OutputWriter::createOGRFields  ----- */

/* --------------------------------------------------------------------------*/
/**
 * @brief Creates a new PDF file from the base DEM and simulation output
 *
 * @Param outputfn - specifies the name of the output PDF file
 *
 * @pre OGR datasource and source raster are created/opened
 * @post All datasources are closed, OGR datasource is deleted from system
 * @post Output PDF is created
 *
 * @Returns True is successful.
 */
/* ----------------------------------------------------------------------------*/
bool OutputWriter::_writePDF (std::string outputfn)
{
    _createSplits();
    _createOGRFile(false);
    _closeOGRFile();
    _createLegend();
    _openSrcDataSet();

    std::string tf_logo_path = FindDataPath( "topofire_logo.png" );
    std::string wn_logo_path = FindDataPath( "wn-splash.png" );
    unsigned int out_x_size = GDALGetRasterXSize( hSrcDS );
    unsigned int out_y_size = GDALGetRasterYSize( hSrcDS );
    /*
    ** This sucks.  It isn't a very good check.  Relief files will be rgb, dem
    ** generated will be single band.  We also need the X and Y dimensions to
    ** scale the legend and the logos.
    */
    int bUseLogo = GDALGetRasterCount( hSrcDS ) > 1 ? TRUE : FALSE;
    hDriver = GDALGetDriverByName( "PDF" );

    if( NULL == hDriver )
    {
        throw std::runtime_error("OutputWriter: Failed to get OGR PDF driver");
    }

    /* We need the dimension to push it against the edge of the page */
    int nNinjaLogoXSize = 0;
    int nNinjaLogoYSize = 0;
    { /* Scoped on purpose */
        GDALDatasetH hDS = GDALOpen( wn_logo_path.c_str(), GA_ReadOnly );
        if( hDS == NULL )
        {
            throw std::runtime_error("OutputWriter: Failed to open windninja logo");
        }
        nNinjaLogoXSize = GDALGetRasterXSize( hDS );
        nNinjaLogoYSize = GDALGetRasterYSize( hDS );
        GDALClose( hDS );
    }

    const char * EXTRA_IMG_FRMT = "%s,%d,%d,%f";

    /* User unit is 1/72" */
    int xMargin = SIDE_MARGIN * 72.0;
    int topMargin = TOP_MARGIN * 72.0;
    int bottomMargin = BOTTOM_MARGIN * 72.0;
    double xRatio = (double)out_x_size / width;
    double yRatio = (double)out_y_size / height;
    double xWidth = (double)out_x_size / (double)dpi;
    double yHeight = (double)out_y_size / (double)dpi;
    xMargin = (width - xWidth) / 2.0 * 72.0;
    bottomMargin = (height - yHeight) / 2.0 * 72.0;
    topMargin = bottomMargin;
    double dfImageYBound = 0.0;
    /* Make the same as the default margin */
    dfImageYBound = MIN( BOTTOM_MARGIN, bottomMargin / 72.0 );
    int nLogoTargetYSize = dpi * dfImageYBound;
    double dfLogoRatio = (double)nLogoTargetYSize / (double)nNinjaLogoYSize;
    double dfLogoWidth = dfLogoRatio * (double)nNinjaLogoXSize / (double)dpi;
    int nXLogoOffset = (width - dfLogoWidth) * 72;
    std::string extra_img_wn = CPLSPrintf( EXTRA_IMG_FRMT, wn_logo_path.c_str(),
                                           nXLogoOffset, 0, dfLogoRatio );

    /* Place the legend at the right margin */
    int nLegendTargetYSize = dpi * dfImageYBound;
    double dfLegendRatio = (double)nLegendTargetYSize / (double)LGND_HEIGHT;
    std::string extra_img_lgnd = CPLSPrintf( EXTRA_IMG_FRMT, pszLegendFile,
                                             36, 0, dfLegendRatio );

    std::string extra_imgs = extra_img_lgnd + "," + extra_img_wn;
    if( bUseLogo )
    {
        // Center the topo fire logo between the other two, also make it about
        // half the size
        // We need to keep track of the legend width in user units for the topo
        double dfLegendWidth = dfLegendRatio * (double)LGND_WIDTH / (double)dpi;
        double dfXCenter = width / 2.0;
        int nXCenter = dfXCenter * 72.0;
        int nTopoTargetYSize = dpi * dfImageYBound * 0.5;
        GDALDatasetH hDS = GDALOpen( tf_logo_path.c_str(), GA_ReadOnly );
        assert( hDS );
        int nTopoXSize = GDALGetRasterXSize( hDS );
        int nTopoYSize = GDALGetRasterYSize( hDS );
        GDALClose( hDS );
        double dfTopoRatio = (double)nTopoTargetYSize / (double)nTopoYSize;
        int nTopoWidth = dfTopoRatio * (double)nTopoXSize;
        dfXCenter -= ((double)nTopoWidth / (double)dpi) / 2.0;
        //int nTopoOffset = xMargin + (dfLegendWidth * 72.0);
        int nTopoOffset = dfXCenter * 72.0;
        std::string extra_img_logo = CPLSPrintf( EXTRA_IMG_FRMT, tf_logo_path.c_str(), nTopoOffset, 0, dfTopoRatio );
        extra_imgs = extra_imgs + "," + extra_img_logo;
    }

    papszOptions = CSLAddNameValue( papszOptions, "OGR_DATASOURCE", pszOgrFile );
    papszOptions = CSLAddNameValue( papszOptions, "OGR_DISPLAY_LAYER_NAMES", "Wind_Vectors");
    papszOptions = CSLAddNameValue( papszOptions, "LAYER_NAME", demFile.c_str() );
    papszOptions = CSLAddNameValue( papszOptions, "EXTRA_IMAGES", extra_imgs.c_str() );
    papszOptions = CSLAddNameValue( papszOptions, "TILED", "YES" );
    papszOptions = CSLAddNameValue( papszOptions, "PREDICTOR", "2" );
    papszOptions = CSLAddNameValue( papszOptions, "DPI", CPLSPrintf("%d", dpi) );
    papszOptions = CSLAddNameValue( papszOptions, "LEFT_MARGIN", CPLSPrintf( "%d", xMargin ) );
    papszOptions = CSLAddNameValue( papszOptions, "RIGHT_MARGIN", CPLSPrintf( "%d", xMargin ) );
    papszOptions = CSLAddNameValue( papszOptions, "TOP_MARGIN", CPLSPrintf( "%d", topMargin ) );
    papszOptions = CSLAddNameValue( papszOptions, "BOTTOM_MARGIN", CPLSPrintf( "%d", bottomMargin ) );

    hDstDS = GDALCreateCopy( hDriver, outputfn.c_str(), hSrcDS, FALSE,
                             papszOptions, NULL, NULL );
    if( NULL == hDstDS )
    {
        throw std::runtime_error("OutputWriter: Error creating output file");
    }

    _closeDataSets();
    _destroyOptions();
    _destroyLegend();

    OGR_Dr_DeleteDataSource( hOGRDriver, pszOgrFile );

    return true;
}  /* -----  end of method OutputWriter::_writePDF  ----- */

bool OutputWriter::_writeFlatGeoBufZip(std::string filename)
{
    const bool hasDateTime = !ninjaTime.is_not_a_date_time();

    _createSplits();
    _createOGRFile(true);
    _createLegend();
    if(hasDateTime)
    {
        _createDateTimeLegend(!wxModelName.empty());
    }

    hDriver = OGRGetDriverByName("FlatGeobuf");
    if ( hDriver == NULL )
    {
        throw std::runtime_error("OutputWriter: FlatGeobuf driver not available.");
    }

    VSIUnlink(filename.c_str());

    std::string baseName = CPLGetBasename(filename.c_str());

    std::string vsiFgbPath = "/vsizip/{" + filename + "}/" + baseName + ".fgb";
    std::string vsiLegendPath = "/vsizip/{" + filename + "}/" + baseName + "_legend.bmp";
    std::string vsiDateTimeLegendPath = "/vsizip/{" + filename + "}/" + baseName + "_datetime.bmp";

    papszOptions = CSLAddNameValue( papszOptions, "SPATIAL_INDEX", "YES" );
    hDstDS = GDALCreateCopy(hDriver, vsiFgbPath.c_str(), hDataSource, FALSE, papszOptions, NULL, NULL);

    if( NULL == hDstDS )
    {
        throw std::runtime_error("OutputWriter: Error creating output file");
    }

    if(pszLegendFile != nullptr && CPLCopyFile(vsiLegendPath.c_str(), pszLegendFile) != 0)
    {
        CPLError(CE_Warning, CPLE_AppDefined, "Failed to add legend file to ZIP archive.");
    }

    if(hasDateTime == true && pszDateTimeLegendFile != nullptr && CPLCopyFile(vsiDateTimeLegendPath.c_str(), pszDateTimeLegendFile) != 0)
    {
        CPLError(CE_Warning, CPLE_AppDefined, "Failed to add date time legend file to ZIP archive.");
    }

    _closeDataSets();
    _destroyOptions();
    _destroyLegend();

    if(hasDateTime)
    {
        _destroyDateTimeLegend();
    }

    OGR_Dr_DeleteDataSource( hOGRDriver, pszOgrFile );

    return true;
}
