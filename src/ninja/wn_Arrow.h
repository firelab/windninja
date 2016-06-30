/******************************************************************************
 * wn_OGRVector.h
 *
 * @description :
 *     Provides a drawable (on GUI) reprentation of simulation data points
 *     for use in simulation output.
 * 
 * @author : Levi Malott <levi.malott@mst.edu>
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

#ifndef WN_ARROW_H
#define WN_ARROW_H

#include "ninjaUnits.h"
#include "ninjaMathUtility.h"
#include "constants.h"


#include "ogr_core.h"
#include "ogr_spatialref.h"
#include "gdal.h"

/**
 * @brief Provides the functionality to convert simulation data points into
 * features or geometries with corresponding styles.
 */
class WN_Arrow
{
public:
    /**
     * Constructor
     */
    WN_Arrow();
    WN_Arrow(    const double &x,         const double &y,
                 const double &speed,     const double &direction,
                 const double &cell_size, const double *thresholds,
                 const unsigned int nsplits );
    void asGeometry( OGRGeometryH & hLine);

    /**
     * Destructor
     */
    ~WN_Arrow();

    
private:
    void   _computeVectorPoints();

    double   m_x;
    double   m_y;
    double   m_speed;
    double   m_dir;
    double   m_cell_size;
    const double * m_thresholds;
    unsigned int   m_nsplits;
    double m_xtip, m_ytip, m_xtail, m_ytail,
           m_xhead_left,   m_xhead_right, 
           m_yhead_left,   m_yhead_right;

};



#endif // WN_ARROW_H
