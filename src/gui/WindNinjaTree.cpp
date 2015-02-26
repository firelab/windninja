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

#include "WindNinjaTree.h"

WindNinjaTree::WindNinjaTree(QWidget *parent) : QWidget(parent)
{
  createTree();
  createStack();
  createConnections();

  layout = new QVBoxLayout;
  layout->addWidget(tree);
  layout->addWidget(stack);
  setLayout(layout);
  
}
  
WindNinjaTree::~WindNinjaTree()
{

}

void WindNinjaTree::createTree()
{
  createIcons();
  tree = new QTreeWidget;
  tree->setColumnCount(1);
  createInputItems();
#ifdef NINJAFOAM
  createSolverMethodItems();
#endif
  createOutputItems();

  mainItem = new QTreeWidgetItem;
  mainItem->setText(0, tr("WindNinja"));

  solveItem = new QTreeWidgetItem;
  solveItem->setText(0, tr("Solve"));
  solveItem->setIcon(0, blue);
  
  //add items in gui order
  tree->setHeaderItem(mainItem);
#ifdef NINJAFOAM
  tree->addTopLevelItem(solverMethodItem);
  solverMethodItem->setExpanded(true);
  solverMethodItem->setSelected(true);
#endif
  tree->addTopLevelItem(inputItem);
  inputItem->setExpanded(true);
#ifndef NINJAFOAM
  surfaceItem->setSelected(true);
#endif
  windItem->setExpanded(true);
  tree->addTopLevelItem(diurnalItem);
#ifdef STABILITY
  tree->addTopLevelItem(stabilityItem);
#endif
  tree->addTopLevelItem(outputItem);
  outputItem->setExpanded(true);
  tree->addTopLevelItem(solveItem);
  solveItem->setExpanded(true);

  tree->setMinimumHeight(240);
  //tree->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
}

void WindNinjaTree::createIcons()
{
  check.addFile(":tick.png");
  cross.addFile(":cross.png");
  caution.addFile(":jason_caution.png");
  blue.addFile(":bullet_blue.png");
  radio.addFile(":radio_bullet_blue.png");
  sun.addFile(":weather_sun.png");
}

#ifdef NINJAFOAM 
void WindNinjaTree::createSolverMethodItems()
{
  solverMethodItem = new QTreeWidgetItem;
  solverMethodItem->setIcon(0, blue);
  solverMethodItem->setText(0, tr("Solver"));
  
  nativeSolverItem = new QTreeWidgetItem;
  nativeSolverItem->setText(0, tr("Conservation of Mass"));
  nativeSolverItem->setIcon(0, radio);
  
  solverMethodItem->addChild(nativeSolverItem);
  
  ninjafoamItem = new QTreeWidgetItem;
  ninjafoamItem->setText(0, tr("Conservation of Mass and Momentum"));
  ninjafoamItem->setIcon(0, radio);

  solverMethodItem->addChild(ninjafoamItem);
}
#endif //NINJAFOAM

void WindNinjaTree::createInputItems()
{
  inputItem = new QTreeWidgetItem;
  inputItem->setIcon(0, blue);
  inputItem->setText(0, tr("Input"));
  
  //create input children, add them to input item
  //surfaceItem
  surfaceItem = new QTreeWidgetItem;
  surfaceItem->setText(0, tr("Surface Input"));
  surfaceItem->setIcon(0, blue);

  diurnalItem = new QTreeWidgetItem;
  diurnalItem->setText(0, tr("Diurnal Input"));
  diurnalItem->setIcon(0, blue);

#ifdef STABILITY
  stabilityItem = new QTreeWidgetItem;
  stabilityItem->setText(0, tr("Stability Input (Beta)"));
  stabilityItem->setIcon(0, blue);
#endif

  windItem = new QTreeWidgetItem;
  windItem->setText(0, tr("Wind Input"));
  windItem->setIcon(0, blue);

  spdDirItem = new QTreeWidgetItem;
  spdDirItem->setText(0, tr("Domain Average Wind"));
  spdDirItem->setIcon(0, radio);

  pointItem = new QTreeWidgetItem;
  pointItem->setText(0, tr("Point Initialization"));
  pointItem->setIcon(0, radio);

  modelItem = new QTreeWidgetItem;
  modelItem->setText(0, tr("Weather Model"));
  modelItem->setIcon(0, radio);

  windItem->addChild(spdDirItem);
  windItem->addChild(pointItem);
  windItem->addChild(modelItem);
  
  inputItem->addChild(surfaceItem);
  inputItem->addChild(diurnalItem);
#ifdef STABILITY
  inputItem->addChild(stabilityItem);
#endif
  inputItem->addChild(windItem);
}

void WindNinjaTree::createOutputItems()
{
  outputItem = new QTreeWidgetItem;
  outputItem->setText(0, tr("Output"));
  outputItem->setIcon(0, blue);
    
  googleItem = new QTreeWidgetItem;
  googleItem->setText(0, tr("Google Earth"));
  googleItem->setIcon(0, blue);
  fbItem = new QTreeWidgetItem;
  fbItem->setText(0, tr("Fire Behavior"));
  fbItem->setIcon(0, blue);
  shapeItem = new QTreeWidgetItem;
  shapeItem->setText(0, tr("Shape Files"));
  shapeItem->setIcon(0, blue);
  vtkItem = new QTreeWidgetItem;
  vtkItem->setText(0, tr("VTK Files"));
  vtkItem->setIcon(0, blue);

  outputItem->addChild(googleItem);
  outputItem->addChild(fbItem);
  outputItem->addChild(shapeItem);
  outputItem->addChild(vtkItem); 

}

void WindNinjaTree::createStack()
{
#ifdef NINJAFOAM
  ninjafoam = new ninjafoamInput;
  nativesolver = new nativeSolverInput;
#endif
  surface = new surfaceInput;
  diurnal = new diurnalInput;
#ifdef STABILITY
  stability = new stabilityInput;
#endif

  wind = new windInput;
  point = new pointInput;
  weather = new weatherModel;
  output = new outputMetaData;
  google = new googleOutput;
  fb = new fbOutput;
  shape = new shapeOutput;
  vtk = new vtkOutput;
  solve = new solvePage;
  
  stack = new QStackedWidget;
  
#ifdef NINJAFOAM
  stack->addWidget(ninjafoam);
  stack->addWidget(nativesolver);
#endif
  stack->addWidget(surface);
  stack->addWidget(diurnal);
#ifdef STABILITY
  stack->addWidget(stability);
#endif

  stack->addWidget(wind);
  stack->addWidget(point);
  stack->addWidget(weather);
  stack->addWidget(output);
  stack->addWidget(google);
  stack->addWidget(fb);
  stack->addWidget(shape);
  stack->addWidget(vtk);
  stack->addWidget(solve);

  stack->setMinimumWidth(480);
}

void WindNinjaTree::createConnections()
{
  connect(tree, SIGNAL(itemSelectionChanged()), this, 
	  SLOT(updateInterface()));
}

void WindNinjaTree::updateInterface()
{
  QTreeWidgetItem *item;
  item = tree->currentItem();
  //figure out which item is selected, then change the stack widget to the proper
  //input page
  if(item == inputItem)
    stack->setCurrentWidget(surface);
  else if(item == surfaceItem)
    stack->setCurrentWidget(surface);
  else if(item == diurnalItem)
    stack->setCurrentWidget(diurnal);
#ifdef STABILITY
  else if(item == stabilityItem)
      stack->setCurrentWidget(stability);
#endif
#ifdef NINJAFOAM
  else if(item == nativeSolverItem)
      stack->setCurrentWidget(nativesolver);
  else if(item == ninjafoamItem)
      stack->setCurrentWidget(ninjafoam);
  else if(item == solverMethodItem)
      stack->setCurrentWidget(ninjafoam);
#endif
  else if(item == windItem)
    stack->setCurrentWidget(wind);
  else if(item == spdDirItem)
    stack->setCurrentWidget(wind);
  else if(item == pointItem)
    stack->setCurrentWidget(point);
  else if(item == modelItem)
    stack->setCurrentWidget(weather);
  else if(item == outputItem)
      stack->setCurrentWidget(output);
  else if(item == googleItem)
    stack->setCurrentWidget(google);
  else if(item == fbItem)
    stack->setCurrentWidget(fb);
  else if(item == shapeItem)
    stack->setCurrentWidget(shape);
  else if(item == vtkItem)
    stack->setCurrentWidget(vtk);
  else if(item == solveItem)
    stack->setCurrentWidget(solve);
}
