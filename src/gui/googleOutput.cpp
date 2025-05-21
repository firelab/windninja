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

  applyVectorScaling = new QCheckBox("Apply Vector Scaling");
  applyVectorScaling->setToolTip("Enable vector scaling to increase line width with wind speed.");
  applyVectorScaling->setCheckable(true);
  applyVectorScaling->setChecked(false);

  vectorLayout = new QHBoxLayout;
  vectorLayout->addWidget(vectorWidthLabel);
  vectorLayout->addWidget(vectorWidthDoubleSpinBox);
  vectorLayout->addWidget(applyVectorScaling);
  vectorLayout->addStretch();

  vectorGroupBox->setLayout(vectorLayout);

  //vectorGroupBox->setChecked(true);

  legendGroupBox = new QGroupBox(tr("Legend"));
  legendGroupBox->setCheckable(true);
  legendGroupBox->setChecked(true);
   
  uniformRangeRadioButton = new QRadioButton;
  uniformRangeRadioButton->setText(tr("Uniform Range"));
  uniformRangeRadioButton->setChecked(true);
  equalCountRadioButton = new QRadioButton;
  equalCountRadioButton->setText(tr("Equal Count"));

  legendGroupBox->setChecked(true);
 
  legendOptionLayout = new QVBoxLayout;
  legendOptionLayout->addWidget(uniformRangeRadioButton);
  legendOptionLayout->addWidget(equalCountRadioButton);
  // legendOptionLayout->addStretch();
  legendGroupBox->setLayout(legendOptionLayout);

  contourCheckBox = new QCheckBox(tr("Contours (beta)"));

  contourCheckBox->setChecked(false);

  //hide contour check box for now.
  contourCheckBox->setDisabled(true);

  applyConsistentColorScheme = new QCheckBox(tr("Use Consistent Color Scale"));
  applyConsistentColorScheme->setToolTip("Use a consistent color scale across simulations.");
  applyConsistentColorScheme->setCheckable(true);
  applyConsistentColorScheme->setChecked(false);

  optionLayout = new QVBoxLayout;
  optionLayout->addWidget(vectorGroupBox);
  optionLayout->addWidget(legendGroupBox);
  optionLayout->addWidget(applyConsistentColorScheme);
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
  
  //color options
   colorblindBox= new QGroupBox(tr("Alternative Color Schemes"));
   colorblindBox->setCheckable(true);
   colorblindBox->setChecked(false);
   inputColorblindComboBox = new QComboBox();
   inputColorblindComboBox->addItem(tr("Default"));
   inputColorblindComboBox->addItem(tr("ROPGW (Red Orange Pink Green White)"));
   inputColorblindComboBox->addItem(tr("Oranges"));
   inputColorblindComboBox->addItem(tr("Blues"));
   inputColorblindComboBox->addItem(tr("Pinks"));
   inputColorblindComboBox->addItem(tr("Greens"));
   inputColorblindComboBox->addItem(tr("Magic Beans"));
   inputColorblindComboBox->addItem(tr("Pink to Green"));

   colorLayout=new QGridLayout;
   colorLayout->addWidget(inputColorblindComboBox,0,0);
   colorblindBox->setLayout(colorLayout);
  //end color options

  resLayout = new QGridLayout;
  resLayout->addWidget(googleResSpinBox, 0, 0);
  resLayout->addWidget(googleMetersRadioButton, 0, 1);
  resLayout->addWidget(googleFeetRadioButton, 0, 2);
  resLayout->addWidget(useMeshResCheckBox, 1, 0);

  googleResGroupBox->setLayout(resLayout);

  pageLayout = new QVBoxLayout;
  pageLayout->addLayout(optionLayout);
  pageLayout->addWidget(googleResGroupBox);
  pageLayout->addWidget(colorblindBox);
  pageLayout->addStretch();
  
  googleGroupBox->setLayout(pageLayout);
  
  mainLayout = new QVBoxLayout;
  mainLayout->addWidget(googleGroupBox);
  mainLayout->addStretch();
  
  setLayout(mainLayout);
}

void googleOutput::setColorScheme(int choice)
{
    if(choice==1)
    {

    }
}
