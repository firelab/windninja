/******************************************************************************
 *
 * $Id$
 *
 * Project:  WindNinja Qt GUI
 * Purpose:  Output wind option widget
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

#include "outputHeightWidget.h"

outputHeightWidget::outputHeightWidget(QWidget *parent) : QWidget(parent)
{
  //create output height sections for tree/gui 
  outputHeightGroupBox = new QGroupBox(tr("Output Height"));

  outputHeightComboBox = new QComboBox;
  outputHeightComboBox->addItem(tr("20ft-US"));
  outputHeightComboBox->addItem(tr("10m-SI"));
  outputHeightComboBox->addItem(tr("Custom"));
  
  outputHeightDoubleSpinBox = new QDoubleSpinBox;
  outputHeightDoubleSpinBox->setRange(0, 10000); //Increased from 100
  outputHeightDoubleSpinBox->setValue(20.00);
  outputHeightDoubleSpinBox->setAccelerated(true);
  outputHeightDoubleSpinBox->setDisabled(true);

  feetRadioButton = new QRadioButton(tr("Feet"));
  feetRadioButton->setChecked(true);
  
  meterRadioButton = new QRadioButton(tr("Meters"));
  
  layout = new QHBoxLayout;
  layout->addWidget(outputHeightComboBox);
  layout->addWidget(outputHeightDoubleSpinBox);
  layout->addWidget(feetRadioButton);
  layout->addWidget(meterRadioButton);
  layout->addStretch();
  
  outputHeightGroupBox->setLayout(layout);

  mainLayout = new QVBoxLayout;
  mainLayout->addWidget(outputHeightGroupBox);

  setLayout(mainLayout);
  
  //signal/slot connection
  connect(outputHeightComboBox, SIGNAL(currentIndexChanged(int)), 
	  this, SLOT(checkOutputHeight(int)));
}
  
void outputHeightWidget::checkOutputHeight(int choice)
{
  if(choice == 0)
    {
      outputHeightDoubleSpinBox->setValue(20.00);
      outputHeightDoubleSpinBox->setDisabled(true);
      feetRadioButton->setChecked(true);
      feetRadioButton->setDisabled(true);
      meterRadioButton->setDisabled(true);
    }
  else if(choice == 1)
    {
      outputHeightDoubleSpinBox->setValue(10.00);
      outputHeightDoubleSpinBox->setDisabled(true);
      meterRadioButton->setChecked(true);
      feetRadioButton->setDisabled(true);
      meterRadioButton->setDisabled(true);
    }
  else if(choice ==  2)
    {
      outputHeightDoubleSpinBox->setEnabled(true);
      feetRadioButton->setEnabled(true);
      feetRadioButton->setChecked(true);
      outputHeightDoubleSpinBox->setValue(0.00);
      meterRadioButton->setEnabled(true);
    }
}
