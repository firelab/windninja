/******************************************************************************
 *
 * $Id$
 *
 * Project:  WindNinja Qt GUI
 * Purpose:  Input for global wind input variables
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

#ifndef METAWINDWIDGET_H
#define METAWINDWIDGET_H

#include <QGroupBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QRadioButton>
#include <QCheckBox>
#include <QLabel>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QSizePolicy>

#include <QToolButton>

#include "gdal_priv.h"
#include "ogr_srs_api.h"

#include "latLonWidget.h"

#ifndef Q_MOC_RUN
#include "ninja.h"
#endif

class metaWindWidget : public QWidget
{
  Q_OBJECT

 public:

  metaWindWidget(QWidget *parent = 0);

  QGroupBox *metaWindGroupBox;
  //input height
  QGroupBox *inputHeightGroupBox;
  QComboBox *inputHeightComboBox;
  QDoubleSpinBox *inputHeightDoubleSpinBox;
  QRadioButton *feetRadioButton, *meterRadioButton;

  //layouts
  QHBoxLayout *inputHeightLayout;
  
  QVBoxLayout *layout;

  public slots:
  void checkInputHeight(int choice);
 
};

#endif /* METAWINDWIDGET_H */
