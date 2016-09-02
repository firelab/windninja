/******************************************************************************
 *
 * Project:  WindNinja Qt GUI
 * Purpose:  PDF output selection widgetx
 * Author:   Kyle Shannon <kyle at pobox dot com>
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

#include "pdfOutput.h"

pdfOutput::pdfOutput(QWidget *parent) : QWidget(parent)
{
    pdfGroupBox = new QGroupBox(tr("Create Geospatial PDF Files (*.pdf)"));
    pdfGroupBox->setCheckable(true);
    pdfGroupBox->setChecked(false);

    vectorGroupBox = new QGroupBox(tr("Vectors"));

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

    pdfResGroupBox = new QGroupBox(tr("Resolution"));

    pdfResSpinBox = new QDoubleSpinBox(this);
    pdfResSpinBox->setRange(1, 5000);
    pdfResSpinBox->setDecimals(2);
    pdfResSpinBox->setAccelerated(true);
    pdfResSpinBox->setValue(200);
    pdfResSpinBox->setDisabled(true);

    pdfMetersRadioButton = new QRadioButton(tr("Meters"));
    pdfMetersRadioButton->setChecked(true);
    pdfMetersRadioButton->setDisabled(true);
    pdfFeetRadioButton = new QRadioButton(tr("Feet"));
    pdfFeetRadioButton->setDisabled(true);

    useMeshResCheckBox = new QCheckBox(tr("Use Mesh Resolution"));
    useMeshResCheckBox->setChecked(true);

    backgroundLabel = new QLabel(tr("Basemap"), this);

    backgroundComboBox = new QComboBox(this);
    backgroundComboBox->addItem(tr("TopoFire topo maps"));
    backgroundComboBox->addItem(tr("Hillshade"));

    // Size names dictated by https://en.wikipedia.org/wiki/Paper_size
    sizeLabel = new QLabel(tr("Size"), this);
    sizeComboBox = new QComboBox(this);
    sizeComboBox->addItem(tr("Letter-8 1/2 x 11"));
    sizeComboBox->addItem(tr("Legal - 8 1/2 x 14"));
    sizeComboBox->addItem(tr("Tabloid - 11 x 17"));

    portraitRadioButton = new QRadioButton(tr("Portrait"), this);
    landscapeRadioButton = new QRadioButton(tr("Landscape"), this);
    portraitRadioButton->setChecked(true);

    orientLayout = new QVBoxLayout;
    orientLayout->addWidget(portraitRadioButton);
    orientLayout->addWidget(landscapeRadioButton);

    sizeLayout = new QHBoxLayout;
    sizeLayout->addWidget(sizeLabel);
    sizeLayout->addWidget(sizeComboBox);
    sizeLayout->addLayout(orientLayout);

    //connect checkbox with spin box
    connect(useMeshResCheckBox, SIGNAL(toggled(bool)), 
      pdfResSpinBox, SLOT(setDisabled(bool)));
    connect(useMeshResCheckBox, SIGNAL(toggled(bool)), 
      pdfFeetRadioButton, SLOT(setDisabled(bool)));
    connect(useMeshResCheckBox, SIGNAL(toggled(bool)), 
      pdfMetersRadioButton, SLOT(setDisabled(bool)));

    backgroundLayout = new QHBoxLayout;
    backgroundLayout->addWidget(backgroundLabel);
    backgroundLayout->addWidget(backgroundComboBox);
    backgroundLayout->addStretch();

    resLayout = new QGridLayout;
    resLayout->addWidget(pdfResSpinBox, 0, 0);
    resLayout->addWidget(pdfMetersRadioButton, 0, 1);
    resLayout->addWidget(pdfFeetRadioButton, 0, 2);
    resLayout->addWidget(useMeshResCheckBox, 1, 0);

    pdfResGroupBox->setLayout(resLayout);

    pageLayout = new QVBoxLayout;
    pageLayout->addWidget(vectorGroupBox);
    pageLayout->addLayout(backgroundLayout);
    pageLayout->addLayout(sizeLayout);
    pageLayout->addWidget(pdfResGroupBox);
    pageLayout->addStretch();

    pdfGroupBox->setLayout(pageLayout);

    mainLayout = new QVBoxLayout;
    mainLayout->addWidget(pdfGroupBox);
    mainLayout->addStretch();

    setLayout(mainLayout);
}
