/******************************************************************************
 *
 * $Id$
 *
 * Project:  WindNinja Qt GUI
 * Purpose:  Handles surface inputs for the domain
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

#include "surfaceInput.h"

/**
 * \brief Construct a widget for suface input data
 *
 * \param parent parent widget
 */
surfaceInput::surfaceInput(QWidget *parent) : QWidget(parent)
{
#ifdef NINJAFOAM
    //make ninjafoam case input controls
    foamCaseGroupBox = new QGroupBox(tr("Use Existing Case"));

    foamCaseLineEdit = new QLineEdit;
    foamCaseLineEdit->setReadOnly(true);

    foamCaseOpenToolButton = new QToolButton;
    foamCaseOpenToolButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    foamCaseOpenToolButton->setText(tr("Open Case"));
    foamCaseOpenToolButton->setIcon(QIcon(":folder_page.png"));
#endif //NINJAFOAM

    //make INPUT input controls
    inputFileGroupBox = new QGroupBox(tr("Elevation Input File"));

    inputFileLineEdit = new QLineEdit;
    inputFileLineEdit->setReadOnly(true);

    inputFileOpenToolButton = new QToolButton;
    inputFileOpenToolButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    inputFileOpenToolButton->setText(tr("Open File"));
    inputFileOpenToolButton->setIcon(QIcon(":folder_page.png"));

    downloadDEMButton = new QToolButton;
    downloadDEMButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    downloadDEMButton->setText(tr("Download File"));
    downloadDEMButton->setIcon(QIcon(":server_go.png"));

#ifdef NINJAFOAM
    foamCaseLayout = new QHBoxLayout;
    foamCaseLayout->addWidget(foamCaseLineEdit);
    foamCaseLayout->addWidget(foamCaseOpenToolButton);
#endif

    inputLayout = new QHBoxLayout;
    inputLayout->addWidget(inputFileLineEdit);
    inputLayout->addWidget(inputFileOpenToolButton);
    inputLayout->addWidget(downloadDEMButton);

    //roughness combo box in a group
    roughnessGroupBox = new QGroupBox(tr("Vegetation"));

    roughnessComboBox = new QComboBox;
    roughnessComboBox->addItem("Grass");
    roughnessComboBox->addItem("Brush");
    roughnessComboBox->addItem("Trees");
    roughnessComboBox->setDisabled(true);

    roughnessLabel = new QLabel;
    roughnessLabel->setText("<font color='red'>Vegetation Data Set Using .lcp File</font>");
    roughnessLabel->setEnabled(true);
    roughnessLabel->setVisible(false);

    roughnessLayout = new QHBoxLayout;
    roughnessLayout->addWidget(roughnessComboBox);
    roughnessLayout->addWidget(roughnessLabel);
    roughnessLayout->addStretch();

    //mesh resolution controls
    meshResGroupBox =  new QGroupBox(tr("Mesh Resolution"));

    meshResComboBox = new QComboBox;
    meshResComboBox->addItem(tr("Coarse"));
    meshResComboBox->addItem(tr("Medium"));
    meshResComboBox->addItem(tr("Fine"));
    meshResComboBox->addItem(tr("Custom"));
    meshResComboBox->insertSeparator(3);
    meshResComboBox->setCurrentIndex(2);
    meshResComboBox->setEnabled(false);

    meshResDoubleSpinBox = new QDoubleSpinBox;
    meshResDoubleSpinBox->setRange(1, 50000);
    meshResDoubleSpinBox->setSingleStep(100.0);
    meshResDoubleSpinBox->setDecimals(2);
    meshResDoubleSpinBox->setAccelerated(true);
    meshResDoubleSpinBox->setValue(200);
    meshResDoubleSpinBox->setEnabled(false);

    meshMetersRadioButton = new QRadioButton(tr("Meters"));
    meshMetersRadioButton->setChecked(true);

    meshFeetRadioButton = new QRadioButton(tr("Feet"));

    meshResLayout = new QHBoxLayout;
    meshResLayout->addWidget(meshResComboBox);
    meshResLayout->addWidget(meshResDoubleSpinBox);
    meshResLayout->addWidget(meshMetersRadioButton);
    meshResLayout->addWidget(meshFeetRadioButton);
    meshResLayout->addStretch();

    timeZoneGroupBox = new QGroupBox("Time Zone", this);
    timeZone = new timeZoneWidget(this);
    timeZoneLayout = new QHBoxLayout;
    timeZoneLayout->addWidget(timeZone);
    timeZoneLayout->addStretch();

    //set layouts to boxes
#ifdef NINJAFOAM
    foamCaseGroupBox->setLayout(foamCaseLayout);
#endif
    inputFileGroupBox->setLayout(inputLayout);
    roughnessGroupBox->setLayout(roughnessLayout);
    meshResGroupBox->setLayout(meshResLayout);
    timeZoneGroupBox->setLayout(timeZoneLayout);
    
    //main layout
    mainLayout = new QVBoxLayout;
#ifdef NINJAFOAM
    mainLayout->addWidget(foamCaseGroupBox);
    foamCaseGroupBox->setHidden( true );
#endif
    mainLayout->addWidget(inputFileGroupBox);
    mainLayout->addWidget(roughnessGroupBox);
    mainLayout->addWidget(meshResGroupBox);
    mainLayout->addWidget(timeZoneGroupBox);
    mainLayout->addStretch();

    setLayout(mainLayout);
}

