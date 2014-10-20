/******************************************************************************
 *
 * $Id$
 *
 * Project:  WindNinja Qt GUI
 * Purpose:  Shape output option selection widget
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

#include "shapeOutput.h"

/**
 * \brief Widget for shape output.
 *
 * \param parent parent widget
 */
shapeOutput::shapeOutput(QWidget *parent) : QWidget(parent)
{
    shapeGroupBox = new QGroupBox(tr("Create Shape Files (*.shp)"));
    shapeGroupBox->setCheckable(true);
    shapeGroupBox->setChecked(false);

    shapeResGroupBox = new QGroupBox(tr("Resolution"));

    shapeResSpinBox = new QDoubleSpinBox(this);
    shapeResSpinBox->setRange(1, 5000);
    shapeResSpinBox->setDecimals(2);
    shapeResSpinBox->setAccelerated(true);
    shapeResSpinBox->setValue(200);
    shapeResSpinBox->setDisabled(true);

    shapeMetersRadioButton = new QRadioButton(tr("Meters"));
    shapeMetersRadioButton->setChecked(true);
    shapeMetersRadioButton->setDisabled(true);
    shapeFeetRadioButton = new QRadioButton(tr("Feet"));
    shapeFeetRadioButton->setDisabled(true);

    useMeshResCheckBox = new QCheckBox(tr("Use Mesh Resolution"));
    useMeshResCheckBox->setChecked(true);

    //connect spinbox and checkbox
    connect(useMeshResCheckBox, SIGNAL(toggled(bool)), 
            shapeResSpinBox, SLOT(setDisabled(bool)));
    connect(useMeshResCheckBox, SIGNAL(toggled(bool)),
            shapeMetersRadioButton, SLOT(setDisabled(bool)));
    connect(useMeshResCheckBox, SIGNAL(toggled(bool)),
            shapeFeetRadioButton, SLOT(setDisabled(bool)));

    shapeResLayout = new QGridLayout;
    shapeResLayout->addWidget(shapeResSpinBox, 0, 0);
    shapeResLayout->addWidget(shapeMetersRadioButton, 0, 1);
    shapeResLayout->addWidget(shapeFeetRadioButton, 0, 2);
    shapeResLayout->addWidget(useMeshResCheckBox, 1, 0);

    shapeResGroupBox->setLayout(shapeResLayout);

    shapeLayout = new QVBoxLayout;
    shapeLayout->addWidget(shapeResGroupBox);
    shapeGroupBox->setLayout(shapeLayout);

    mainLayout = new QVBoxLayout;
    mainLayout->addWidget(shapeGroupBox);
    mainLayout->addStretch();

    setLayout(mainLayout);
}


