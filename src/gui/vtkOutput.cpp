/******************************************************************************
 *  toggle nerdtree on/off
 *
 * $Id$
 *
 * Project:  WindNinja Qt GUI
 * Purpose:  vtk output selection widget
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

#include "vtkOutput.h"

vtkOutput::vtkOutput(QWidget *parent) : QWidget(parent)
{
  vtkGroupBox = new QGroupBox(tr("Create VTK Files (*.vtk)"), this);
  vtkGroupBox->setCheckable(true);
  vtkGroupBox->setChecked(false);

  vtkLabel = new QLabel(tr("Write VTK surface and volume files"), this);
  vtkWarningLabel = new QLabel(tr("*vtk files are for advanced users and are\n   rarely used by fire managers/modelers."), this);
  vtkWarningLabel->setDisabled(true);
  //writeVolumeCheckBox = new QCheckBox(tr("Volume File"), this); 
  //writeSurfaceCheckBox = new QCheckBox(tr("Surface File"), this);
  
  ninjafoamConflictLabel = new QLabel(tr("*.vtk files are not written for the"
   " mometum solver output. Instead, the\n"
   " OpenFOAM case directory is left behind in the same directory as your DEM.\n"
   " This case directory (NINJAFOAM_*) contains the 3-D files which can be\n"
   " viewed in a program called ParaView. See the OpenFOAM documentation for\n"
   " more details.\n"
   " \nNote that these files are for advanced users and are rarely used by fire\n"
   "managers/modelers.\n"
   ), this);
  ninjafoamConflictLabel->setHidden(true);
  
  //vtkLayout = new QVBoxLayout;
  //vtkLayout->addWidget(vtkLabel);
  //vtkLayout->addWidget(writeVolumeCheckBox);
  //vtkLayout->addWidget(writeSurfaceCheckBox);
  //vtkLayout->addStretch();
  //vtkLayout->addWidget(vtkWarningLabel);
  //vtkGroupBox->setLayout(vtkLayout);

  layout = new QVBoxLayout;

  layout->addWidget(vtkGroupBox);
  layout->addWidget(vtkLabel);
  layout->addWidget(vtkWarningLabel);
  layout->addWidget(ninjafoamConflictLabel);
  layout->addStretch();
  setLayout(layout);
  //layout->addStretch();
}
  
