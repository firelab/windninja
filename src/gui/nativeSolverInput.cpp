/******************************************************************************
 * 
 * $Id: stabilityInput.cpp 1304 2012-01-20 21:07:12Z kyle.shannon $
 *
 * Project:  WindNinja Qt GUI
 * Purpose:  native solver interface
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

#include "nativeSolverInput.h"

/** 
 * Construct and layout the nativeSolverInput widget.  This is only a checkable
 * option now.
 * 
 * @param parent parent widget
 */
nativeSolverInput::nativeSolverInput(QWidget *parent) : QWidget(parent)
{
    nativeSolverGroupBox = new QGroupBox(tr("Conservation of Mass"));
    nativeSolverGroupBox->setCheckable(true);
    nativeSolverGroupBox->setChecked(true);
    
    nativeSolverLabel = new QLabel(tr("This is the native WindNinja solver. It solves a conservation of mass equation,\n"
        "but not a conservation of momentum equation. This solver is fast-running, \n"
        "but may give less accurate wind predictions in regions where momentum effects are\n"
        "important, for example on the lee side of terrain obstacles.\n"
        ), this);
    
    nativeSolverLayout = new QVBoxLayout;
  
    nativeSolverGroupBox->setLayout(nativeSolverLayout);
    
    layout = new QVBoxLayout;
    layout->addWidget(nativeSolverGroupBox);
    layout->addWidget(nativeSolverLabel);
    layout->addStretch();
    setLayout(layout);

    
}
