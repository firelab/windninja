/******************************************************************************
 *
 * $Id$
 *
 * Project:  WindNinja Qt GUI
 * Purpose:  Handles solve parameters and fires the solver
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

#include "solvePage.h"

solvePage::solvePage(QWidget *parent) : QWidget(parent)
{
  numProcLabel = new QLabel(tr("Number of Processors"));

  numProcSpinBox = new QSpinBox;
  numProcSpinBox->setRange(1, 16);
  numProcSpinBox->setValue(1);
  numProcSpinBox->setAccelerated(true);

  numProcLabel->setBuddy(numProcSpinBox);

  solveToolButton = new QToolButton;
  solveToolButton->setText("Solve");
  solveToolButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
  solveToolButton->setIcon(QIcon(":cog_go.png"));
  solveToolButton->setDisabled(true);

  numProcessors = getNumProcessors();
  if(numProcessors < 0)
    {
      availProcString = "Running Serial";
      numProcessors = 1;
    }
  else
     availProcString = "Available Processors: " + QString::number(numProcessors);

  availProcLabel =  new QLabel(availProcString);

  numProcSpinBox->setMaximum(numProcessors);
  numProcSpinBox->setValue(numProcessors);

  outputDirLabel = new QLabel( this );
  outputDirLabel->setText( "Output Directory" );
  outputDirLineEdit = new QLineEdit( this );
  outputDirLineEdit->setReadOnly( true );

  outputDirToolButton = new QToolButton( this );
  outputDirToolButton->setText( "Save output in..." );
  outputDirToolButton->setToolButtonStyle( Qt::ToolButtonTextBesideIcon );
  outputDirToolButton->setIcon( QIcon( ":folder.png" ) );

  openOutputPathButton = new QToolButton( this );
  openOutputPathButton->setText( "Open Output Files Path" );
  openOutputPathButton->setToolButtonStyle( Qt::ToolButtonTextBesideIcon );
  openOutputPathButton->setIcon( QIcon( ":folder.png" ) );
  openOutputPathButton->setDisabled( true );

  CaseFileBox = new QCheckBox(tr("Generate Casefile"), this);
  CaseFileBox->setChecked(true);

  layout = new QVBoxLayout;
  layout->addWidget(availProcLabel);

  pageLayout = new QHBoxLayout;
  pageLayout->addWidget(numProcLabel);
  pageLayout->addWidget(numProcSpinBox);
  pageLayout->addWidget(solveToolButton);
  pageLayout->addWidget(CaseFileBox);
  pageLayout->addStretch();

  outputPathLayout = new QHBoxLayout;
  outputPathLayout->addWidget( outputDirLineEdit );
  outputPathLayout->addWidget( outputDirToolButton );
  outputPathLayout->addWidget( openOutputPathButton );

  connect(outputDirToolButton, SIGNAL( clicked() ),
      this, SLOT( chooseOutputDir() ) );

  layout->addLayout(pageLayout);
  layout->addWidget(outputDirLabel);
  layout->addLayout( outputPathLayout );
  layout->addStretch();
  setLayout(layout);
}

void solvePage::setOutputDir(QString dir) {
  outputDirLineEdit->setText( dir );
}

void solvePage::chooseOutputDir() {
  QString dir = QFileDialog::getExistingDirectory( this,
    tr("Open Output Directory"), outputDirLineEdit->text(), QFileDialog::ShowDirsOnly );
  if( dir == "" )
  {
    dir = outputDirLineEdit->text();
  }
  outputDirLineEdit->setText( dir );
}

QString solvePage::outputDirectory() {
  return outputDirLineEdit->text();
}

int solvePage::getNumProcessors()
{
  int procs = -1;
#ifdef _OPENMP
  procs = omp_get_num_procs();
#endif
  return procs;
}
