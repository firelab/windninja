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

#include "metaWindWidget.h"

metaWindWidget::metaWindWidget(QWidget *parent) : QWidget(parent)
{
  metaWindGroupBox = new QGroupBox(tr("Options"));
  
   //input Height widgets
  inputHeightGroupBox = new QGroupBox(tr("Input Wind Height"));

  inputHeightComboBox = new QComboBox;
  inputHeightComboBox->addItem(tr("20ft-US"));
  inputHeightComboBox->addItem(tr("10m-SI"));
  inputHeightComboBox->addItem(tr("Custom"));
  
  inputHeightDoubleSpinBox = new QDoubleSpinBox;
  inputHeightDoubleSpinBox->setRange(0, 100000); //Increased from 100
  inputHeightDoubleSpinBox->setValue(20.00);
  inputHeightDoubleSpinBox->setAccelerated(true);
  inputHeightDoubleSpinBox->setDisabled(true);
  
  feetRadioButton = new QRadioButton(tr("Feet"));
  feetRadioButton->setChecked(true);
  feetRadioButton->setDisabled(true);
  meterRadioButton = new QRadioButton(tr("Meters"));
  meterRadioButton->setDisabled(true);
  
  inputHeightLayout = new QHBoxLayout;
  inputHeightLayout->addWidget(inputHeightComboBox);
  inputHeightLayout->addWidget(inputHeightDoubleSpinBox);
  inputHeightLayout->addWidget(feetRadioButton);
  inputHeightLayout->addWidget(meterRadioButton);
  inputHeightLayout->addStretch();
  
  inputHeightGroupBox->setLayout(inputHeightLayout);

  connect(inputHeightComboBox, SIGNAL(currentIndexChanged(int)), 
	  this, SLOT(checkInputHeight(int)));

  layout = new QVBoxLayout;
  layout->addWidget(inputHeightGroupBox);
  
  setLayout(layout);
}

void metaWindWidget::checkInputHeight(int choice)
{
  if(choice == 0)
    {
      inputHeightDoubleSpinBox->setValue(20.00);
      inputHeightDoubleSpinBox->setDisabled(true);
      feetRadioButton->setChecked(true);
      feetRadioButton->setDisabled(true);
      meterRadioButton->setDisabled(true);
    }
  else if(choice == 1)
    {
      inputHeightDoubleSpinBox->setValue(10.00);
      inputHeightDoubleSpinBox->setDisabled(true);
      meterRadioButton->setChecked(true);
      feetRadioButton->setDisabled(true);
      meterRadioButton->setDisabled(true);
    }
  else if(choice ==  2)
    {
      inputHeightDoubleSpinBox->setEnabled(true);
      feetRadioButton->setEnabled(true);
      feetRadioButton->setChecked(true);
      inputHeightDoubleSpinBox->setValue(0.00);
      meterRadioButton->setEnabled(true);
    }
}
