/******************************************************************************
 *
 * $Id$
 *
 * Project:  WindNinja Qt GUI
 * Purpose:  Wind input table for specifying wind speed and direction, as well
 *           as the diurnal specifics.
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

#ifndef WINDINPUTTABLE_H
#define WINDINPUTTABLE_H

#include <QScrollArea>


#include <QLabel>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QSpinBox>

#include <QTimeEdit>
#include <QDateEdit>

#include <QHBoxLayout>
#include <QGridLayout>

class windInputTable : public QWidget
{
  Q_OBJECT
 public:
  windInputTable(QWidget *parent = 0);
  
  int nRuns;
  
  //base inputs
  QLabel *speedLabel, *dirLabel;

  QComboBox *inputSpeedUnits;

  QDoubleSpinBox **speed;
  QSpinBox **dir;
  
  QGridLayout *baseLayout;

  //diurnal input
  QLabel *timeLabel, *dateLabel, *cloudLabel, *airTempLabel;

  QComboBox *airTempUnits;
  
  QTimeEdit **time;
  QDateEdit **date;
  QSpinBox **cloudCover, **airTemp;

 public slots:
  void enableDiurnalCells(bool checked);
  void enableStabilityCells(bool checked);
  void clear();

 public:
  QGridLayout *diurnalLayout;
  
  QHBoxLayout *mainLayout;

private:
    bool diurnalChecked;
    bool stabilityChecked;
    void windInputUpdate();
};

#endif /* WINDINPUTTABLE_H */
