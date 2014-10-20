/******************************************************************************
 *
 * $Id$
 *
 * Project:  WindNinja Qt GUI
 * Purpose:  Google earth output selection widgetx
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

#include "googleOutput.h"

googleOutput::googleOutput(QWidget *parent) : QWidget(parent)
{
  googleGroupBox = new QGroupBox(tr("Create Google Earth Files (*.kmz)"));
  googleGroupBox->setCheckable(true);
  googleGroupBox->setChecked(false);

  vectorGroupBox = new QGroupBox(tr("Vectors"));
  //vectorGroupBox->setCheckable(true);
  
  vectorWidthLabel = new QLabel(tr("Line Width"));

  vectorWidthDoubleSpinBox = new QDoubleSpinBox;
  vectorWidthDoubleSpinBox->setRange(1.0, 10.0);
  vectorWidthDoubleSpinBox->setDecimals(1);
  vectorWidthDoubleSpinBox->setValue(1.0);
  vectorWidthDoubleSpinBox->setSingleStep(0.1);
  vectorWidthDoubleSpinBox->setAccelerated(true);

  vectorLayout = new QHBoxLayout;
  vectorLayout->addWidget(vectorWidthLabel);
  vectorLayout->addWidget(vectorWidthDoubleSpinBox);
  vectorLayout->addStretch();

  vectorGroupBox->setLayout(vectorLayout);

  legendGroupBox = new QGroupBox(tr("Legend"));
  legendGroupBox->setCheckable(true);
  legendGroupBox->setChecked(true);
   
  uniformRangeRadioButton = new QRadioButton;
  uniformRangeRadioButton->setText(tr("Uniform Range"));
  uniformRangeRadioButton->setChecked(true);
  equalCountRadioButton = new QRadioButton;
  equalCountRadioButton->setText(tr("Equal Count"));
   
  contourCheckBox = new QCheckBox(tr("Contours (beta)"));
  
  //vectorGroupBox->setChecked(true);

  legendGroupBox->setChecked(true);
 
  legendOptionLayout = new QVBoxLayout;
  legendOptionLayout->addWidget(uniformRangeRadioButton);
  legendOptionLayout->addWidget(equalCountRadioButton);
  // legendOptionLayout->addStretch();
  legendGroupBox->setLayout(legendOptionLayout);

  contourCheckBox->setChecked(false);

  //hide contour check box for now.
  contourCheckBox->setDisabled(true);
  
  optionLayout = new QVBoxLayout;
  optionLayout->addWidget(vectorGroupBox);
  optionLayout->addWidget(legendGroupBox);
  //optionLayout->addWidget(contourCheckBox);
  optionLayout->addStretch();
  
  googleResGroupBox = new QGroupBox(tr("Resolution"));
  
  googleResSpinBox = new QDoubleSpinBox(this);
  googleResSpinBox->setRange(1, 5000);
  googleResSpinBox->setDecimals(2);
  googleResSpinBox->setAccelerated(true);
  googleResSpinBox->setValue(200);
  googleResSpinBox->setDisabled(true);
  //googleResSpinBox->setMaximumSize(googleResSpinBox->sizeHint());
  
  googleMetersRadioButton = new QRadioButton(tr("Meters"));
  googleMetersRadioButton->setChecked(true);
  googleMetersRadioButton->setDisabled(true);
  googleFeetRadioButton = new QRadioButton(tr("Feet"));
  googleFeetRadioButton->setDisabled(true);

  useMeshResCheckBox = new QCheckBox(tr("Use Mesh Resolution"));
  useMeshResCheckBox->setChecked(true);

  //connect checkbox with spin box
  connect(useMeshResCheckBox, SIGNAL(toggled(bool)), 
	  googleResSpinBox, SLOT(setDisabled(bool)));
  connect(useMeshResCheckBox, SIGNAL(toggled(bool)), 
	  googleFeetRadioButton, SLOT(setDisabled(bool)));
  connect(useMeshResCheckBox, SIGNAL(toggled(bool)), 
	  googleMetersRadioButton, SLOT(setDisabled(bool)));
  
  
  resLayout = new QGridLayout;
  resLayout->addWidget(googleResSpinBox, 0, 0);
  resLayout->addWidget(googleMetersRadioButton, 0, 1);
  resLayout->addWidget(googleFeetRadioButton, 0, 2);
  resLayout->addWidget(useMeshResCheckBox, 1, 0);

  googleResGroupBox->setLayout(resLayout);

  pageLayout = new QVBoxLayout;
  pageLayout->addLayout(optionLayout);
  pageLayout->addWidget(googleResGroupBox);
  pageLayout->addStretch();
  
  googleGroupBox->setLayout(pageLayout);
  
  mainLayout = new QVBoxLayout;
  mainLayout->addWidget(googleGroupBox);
  mainLayout->addStretch();
  
  setLayout(mainLayout);
}
