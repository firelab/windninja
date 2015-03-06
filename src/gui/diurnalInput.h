/******************************************************************************
 * 
 * $Id$
 *
 * Project:  WindNinja Qt GUI
 * Purpose:  OpenGL implementation for viewing DEM inputs
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

#ifndef DIURNALINPUT_H_
#define DIURNALINPUT_H_

#include <QGroupBox>
#include <QComboBox>
#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QCheckBox>
#include <QHBoxLayout>
#include <QGridLayout>

#include <string>
#include <vector>

#include "gdal_priv.h"
#include "ogr_srs_api.h"

#include "boost/date_time/local_time/local_time.hpp"
#include "boost/date_time/posix_time/posix_time_types.hpp"

#include "latLonWidget.h"
#include "timeZoneWidget.h"

#include "qdebug.h"

class diurnalInput : public QWidget
{
  Q_OBJECT

 public:

  diurnalInput(QWidget *parent = 0);
  QGroupBox *diurnalGroupBox;
  QVBoxLayout *diurnalLayout;
  QVBoxLayout *layout;
  QLabel *ninjafoamConflictLabel;

};

#endif /* DIURNALINPUT_H_ */

