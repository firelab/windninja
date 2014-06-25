/******************************************************************************
 *
 * $Id$
 *
 * Project:  WindNinja Qt GUI
 * Purpose:  Fire behavior output selection widget
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

#include "fbOutput.h"

/**
 * \brief Create the widget for fire behavior outputs.
 *
 * \param parent parent widget
 */
fbOutput::fbOutput(QWidget *parent) : QWidget(parent)
{
    fbGroupBox = new QGroupBox(tr("Create Fire Behavior Files (*.asc)"), this);
    fbGroupBox->setCheckable(true);
    fbGroupBox->setChecked(false);

    fbResGroupBox = new QGroupBox(tr("Resolution"), this);

    fbResSpinBox = new QDoubleSpinBox(this);
    fbResSpinBox->setRange(1, 5000);
    fbResSpinBox->setDecimals(2);
    fbResSpinBox->setAccelerated(true);
    fbResSpinBox->setValue(200);
    fbResSpinBox->setDisabled(true);

    fbMetersRadioButton = new QRadioButton(tr("Meters"), this);
    fbMetersRadioButton->setChecked(true);
    fbMetersRadioButton->setDisabled(true);
    fbFeetRadioButton = new QRadioButton(tr("Feet"));
    fbFeetRadioButton->setDisabled(true);

    useMeshResCheckBox = new QCheckBox(tr("Use Mesh Resolution"), this);
    useMeshResCheckBox->setChecked(true);

    atmFileCheckBox = new QCheckBox(tr("Create *.atm file(s)"),this);

    //connect spinbox and checkbox
    connect(useMeshResCheckBox, SIGNAL(toggled(bool)), 
            fbResSpinBox, SLOT(setDisabled(bool)));
    connect(useMeshResCheckBox, SIGNAL(toggled(bool)), 
            fbMetersRadioButton, SLOT(setDisabled(bool)));
    connect(useMeshResCheckBox, SIGNAL(toggled(bool)), 
            fbFeetRadioButton, SLOT(setDisabled(bool)));

    fbResLayout = new QGridLayout;
    fbResLayout->addWidget(fbResSpinBox, 0, 0);
    fbResLayout->addWidget(fbMetersRadioButton, 0, 1);
    fbResLayout->addWidget(fbFeetRadioButton, 0, 2);
    fbResLayout->addWidget(useMeshResCheckBox, 1, 0);

    fbResGroupBox->setLayout(fbResLayout);

    fbLayout = new QVBoxLayout;
    fbLayout->addWidget(fbResGroupBox);
    fbLayout->addWidget(atmFileCheckBox);

    fbGroupBox->setLayout(fbLayout);

    mainLayout = new QVBoxLayout;
    mainLayout->addWidget(fbGroupBox);
    mainLayout->addStretch();

    setLayout(mainLayout);
}

