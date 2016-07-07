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

#include "windInputTable.h"

windInputTable::windInputTable(QWidget *parent) : QWidget(parent)
{
  nRuns = 64;
  diurnalChecked = false;
  stabilityChecked = false;
  speedLabel = new QLabel(tr("Speed"));
  dirLabel = new QLabel(tr("Direction"));
  
  inputSpeedUnits = new QComboBox;
  inputSpeedUnits->addItem(tr("mph"), 0);
  inputSpeedUnits->addItem(tr("m/s"), 1);
  inputSpeedUnits->addItem(tr("kph"), 2);
  inputSpeedUnits->setEditable(false);
  inputSpeedUnits->setCurrentIndex(0);

  speed = new QDoubleSpinBox*[nRuns];
  dir  = new QSpinBox*[nRuns];

  baseLayout = new QGridLayout;
  baseLayout->addWidget(speedLabel, 0, 0);
  baseLayout->addWidget(dirLabel, 0, 1);
  baseLayout->addWidget(inputSpeedUnits, 1, 0);

  setMinimumHeight(480);
  
  //set up speed and direction input

  for(int i = 0;i < nRuns;i++)
    {
      speed[i] = new QDoubleSpinBox;
      speed[i]->setRange(0.0, 500.0);
      speed[i]->setValue(0);

      dir[i] = new QSpinBox;
      dir[i]->setRange(0, 360);
      speed[i]->setValue(0);

      baseLayout->addWidget(speed[i], i + 2, 0);
      baseLayout->addWidget(dir[i], i + 2, 1);
    }

  //set up diurnal
  timeLabel = new QLabel(tr("Time"));
  dateLabel = new QLabel(tr("Date"));
  cloudLabel = new QLabel(tr("Cloud Cover(%)"));
  airTempLabel = new QLabel(tr("Air Temp."));;

  airTempUnits = new QComboBox;
  airTempUnits->addItem(tr("F"));
  airTempUnits->addItem(tr("C"));
  airTempUnits->setEditable(false);
  airTempUnits->setCurrentIndex(0);

  diurnalLayout = new QGridLayout;
  diurnalLayout->addWidget(timeLabel, 0, 0);
  diurnalLayout->addWidget(dateLabel, 0, 1);
  diurnalLayout->addWidget(cloudLabel, 0, 2);
  diurnalLayout->addWidget(airTempLabel, 0, 3);
  
  diurnalLayout->addWidget(airTempUnits, 1, 3);

  time = new QTimeEdit* [nRuns];
  date = new QDateEdit* [nRuns];
  cloudCover = new QSpinBox *[nRuns];
  airTemp = new QSpinBox *[nRuns];
  
  for(int i = 0;i < nRuns;i++)
    {
      time[i] = new QTimeEdit(QTime::currentTime());
      time[i]->setDisplayFormat("HH:mm");
      date[i] = new QDateEdit(QDate::currentDate());
      date[i]->setCalendarPopup(true);
      cloudCover[i] = new QSpinBox;
      cloudCover[i]->setRange(0, 100);
      airTemp[i] = new QSpinBox;
      airTemp[i]->setRange(-40, 200);
      airTemp[i]->setValue(72);

      diurnalLayout->addWidget(time[i], i + 4, 0);
      diurnalLayout->addWidget(date[i], i + 4, 1);
      diurnalLayout->addWidget(cloudCover[i], i + 4, 2);
      diurnalLayout->addWidget(airTemp[i], i + 4, 3);
    }

  enableDiurnalCells(false);
  enableStabilityCells(false);

  mainLayout = new QHBoxLayout;
  mainLayout->addLayout(baseLayout);
  mainLayout->addLayout(diurnalLayout);

  setLayout(mainLayout);
}

void windInputTable::enableStabilityCells(bool checked)
{
    stabilityChecked = checked;
    this->windInputUpdate();
}
        

void  windInputTable::enableDiurnalCells(bool checked)
{
    diurnalChecked = checked;
    this->windInputUpdate();
}

void windInputTable::windInputUpdate()
{
    if(this->stabilityChecked == true && this->diurnalChecked == false)
    {
        airTempUnits->setEnabled(false);
        for(int i = 0;i < nRuns;i++)
	    {	  
	        time[i]->setEnabled(true);
	        date[i]->setEnabled(true);
	        cloudCover[i]->setEnabled(true);
	        airTemp[i]->setEnabled(false);
	    }
    }
    else if(this->stabilityChecked == false && this->diurnalChecked == true)
    {
        airTempUnits->setEnabled(true);
        for(int i = 0;i < nRuns;i++)
	    {	  
	        time[i]->setEnabled(true);
	        date[i]->setEnabled(true);
	        cloudCover[i]->setEnabled(true);
	        airTemp[i]->setEnabled(true);
	    }
    }
    else if(this->stabilityChecked == true && this->diurnalChecked == true)
    {
        airTempUnits->setEnabled(true);
        for(int i = 0;i < nRuns;i++)
	    {	  
	        time[i]->setEnabled(true);
	        date[i]->setEnabled(true);
	        cloudCover[i]->setEnabled(true);
	        airTemp[i]->setEnabled(true);
	    }
    }
    else if(this->stabilityChecked == false && this->diurnalChecked == false)
    {
        airTempUnits->setEnabled(false);
        for(int i = 0;i < nRuns;i++)
	    {	  
	        time[i]->setEnabled(false);
	        date[i]->setEnabled(false);
	        cloudCover[i]->setEnabled(false);
	        airTemp[i]->setEnabled(false);
	    }
    }
}

void windInputTable::clear() {
  for (int i = 0; i < nRuns; i++) {
    speed[i]->setValue(0.0);
    dir[i]->setValue(0.0);
    cloudCover[i]->setValue(0);
    airTemp[i]->setValue(72);
  }
}
