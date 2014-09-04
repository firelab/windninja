/******************************************************************************
 * 
 * $Id: stabilityInput.cpp 1304 2012-01-20 21:07:12Z kyle.shannon $
 *
 * Project:  WindNinja Qt GUI
 * Purpose:  NinjaFOAM interface
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

#include "ninjafoamInput.h"

/** 
 * Construct and layout the ninjafoamInput widget.  This is only a checkable
 * option now.
 * 
 * @param parent parent widget
 */
ninjafoamInput::ninjafoamInput(QWidget *parent) : QWidget(parent)
{
    ninjafoamGroupBox = new QGroupBox(tr("Use Momentum Solver"));
    ninjafoamGroupBox->setCheckable(true);
    ninjafoamGroupBox->setChecked(false);
    
    ninjafoamLayout = new QVBoxLayout;
  
    ninjafoamGroupBox->setLayout(ninjafoamLayout);
    
    ninjafoamBcGroupBox = new QGroupBox(tr("Use Non-Equilibrium Boundary Conditions"));
    ninjafoamBcGroupBox->setCheckable(true);
    ninjafoamBcGroupBox->setChecked(false);
    ninjafoamBcGroupBox->setEnabled(false);
    
    ninjafoamBcLayout = new QVBoxLayout;
    
    ninjafoamBcGroupBox->setLayout(ninjafoamBcLayout);
    
    layout = new QVBoxLayout;
    layout->addWidget(ninjafoamGroupBox);
    layout->addWidget(ninjafoamBcGroupBox);
    layout->addStretch();
    setLayout(layout);

    
}
