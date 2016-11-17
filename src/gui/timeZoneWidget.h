/******************************************************************************
 *
 * $Id$
 *
 * Project:  WindNinja
 * Purpose:  Widget for time zone access using boost local_time.
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

#ifndef TIME_ZONE_WIDGET_H
#define TIME_ZONE_WIDGET_H

#include <QMouseEvent>
#include <QComboBox>
#include <QCheckBox>
#include <QLabel>
#include <QString>

#include <QHBoxLayout>
#include <QVBoxLayout>

#include <QDebug>

#ifndef Q_MOC_RUN
#include "boost/date_time/local_time/local_time.hpp"
#include "boost/date_time/posix_time/posix_time_types.hpp"
#endif

namespace blt = boost::local_time;
namespace bpt = boost::posix_time;

#include "ninjaException.h"
#include "ninja_conv.h"

class timeZoneWidget : public QWidget
{
 Q_OBJECT    

 public:
    timeZoneWidget( QWidget *parent = 0 );
    ~timeZoneWidget();		
    QComboBox *tzComboBox;	/**< Select time zone */
    QCheckBox *tzCheckBox;	/**< Display all zones, or US zones */
    QLabel *tzDetailLabel;	/**< Show timezone details */
    QCheckBox *tzDetailCheckBox; /**< Toggle details */
    QStringList tzStringList;	/**< List of zones read from boost */

    void loadTimeZones();
    void loadDefaultTimeZones();

    QHBoxLayout *tzLayout;
    QVBoxLayout *layout;

 public slots:
    void toggleAllTimeZones( bool showAll );
 private slots:
    void updateDetailString( int index );
    void showDetails( bool show );
 private:
    void createConnections();

 signals:
    void tzChanged( QString tz );
};

#endif /* TIME_ZONE_WIDGET_H */
