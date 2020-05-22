/******************************************************************************
 *
 * $Id: GoogleMapsInterface.h 1757 2012-08-07 18:40:40Z kyle.shannon $
 *
 * Project:  WindNinja
 * Purpose:  Class for creating an interface between javascript and Qt
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
#ifndef GOOGLE_MAPS_INTERFACE_H_
#define GOOGLE_MAPS_INTERFACE_H_
#include <QtCore/QObject>

class GoogleMapsInterface : public QObject
{
    Q_OBJECT

public:
    GoogleMapsInterface(QObject *parent = 0);

signals:
        void latlngChanged(double lat, double lng);
        void latlngChangedGUI(double lat, double lng);
        void plotUserPoint();
        void zoomExtents();
        void boundsChanged(double northBound, double southBound, double eastBound, double westBound);
        void boundsChangedGUI(double northBound, double southBound, double eastBound, double westBound);
        void bufferChanged();
        void areaSelected(bool selected);
        void geocodeError();
};

#endif /* GOOGLE_MAPS_INTERFACE_H_ */

