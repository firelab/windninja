/******************************************************************************
 *
 * $Id$ 
 *
 * Project:  WindNinja Qt GUI
 * Purpose:  Tree display for selecting input/output widgets.
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

#ifndef WINDNINJATREE_H
#define WINDNINJATREE_H

#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QSplitter>
#include <QStackedWidget>

#include "surfaceInput.h"
#include "diurnalInput.h"

#ifdef STABILITY
#include "stabilityInput.h"
#endif

#ifdef NINJAFOAM
#include "ninjafoamInput.h"
#include "nativeSolverInput.h"
#endif

#include "windInput.h"
#include "pointInput.h"
#include "weatherModel.h"
#include "outputMetaData.h"
#include "googleOutput.h"
#include "fbOutput.h"
#include "shapeOutput.h"
#include "pdfOutput.h"
#include "vtkOutput.h"
#include "solvePage.h"

class WindNinjaTree : public QWidget
{
  Q_OBJECT

 public:
  
  WindNinjaTree(QWidget *parent = 0);
  ~WindNinjaTree();
  //icons
  QIcon check, cross, caution, blue, radio, sun;
  //tree for navigating
  QTreeWidget *tree;

  QSplitter *splitter;
  //items
  QTreeWidgetItem *mainItem;
  QTreeWidgetItem *solverMethodItem;
  QTreeWidgetItem *inputItem;
  QTreeWidgetItem *surfaceItem;
  
  //make widgetItem for variable stuff...
  QTreeWidgetItem *windItem;
  QTreeWidgetItem *spdDirItem;
  QTreeWidgetItem *pointItem;
  QTreeWidgetItem *modelItem;
  QTreeWidgetItem *diurnalItem;
#ifdef STABILITY
  QTreeWidgetItem *stabilityItem;
#endif
#ifdef NINJAFOAM
  QTreeWidgetItem *ninjafoamItem;
  QTreeWidgetItem *nativeSolverItem;
#endif
  
  //output file items...
  QTreeWidgetItem *outputItem;
  QTreeWidgetItem *googleItem;
  QTreeWidgetItem *fbItem;
  QTreeWidgetItem *shapeItem;
  QTreeWidgetItem *pdfItem;
  QTreeWidgetItem *vtkItem;

  QTreeWidgetItem *solveItem;
  
  void createTree();
  void createIcons();
  void createInputItems();
  void createSolverMethodItems();
  void createOutputItems();
  void createStack();
  void createConnections();

  //add stack widget to hold the ui widgets

  QStackedWidget *stack;

  surfaceInput *surface;
  diurnalInput *diurnal;
#ifdef STABILITY
  stabilityInput *stability;
#endif
#ifdef NINJAFOAM
  ninjafoamInput *ninjafoam;
  nativeSolverInput *nativesolver;
#endif
  windInput *wind;
  pointInput *point;
  weatherModel *weather;
  outputMetaData *output;
  googleOutput *google;
  fbOutput *fb;
  shapeOutput *shape;
  pdfOutput *pdf;
  vtkOutput *vtk;
  solvePage *solve;

  //layout the whole thing
  QVBoxLayout *layout;

 public slots:
  void updateInterface();

};

#endif /* WINDNINJATREE_H */ 
