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
    ninjafoamGroupBox = new QGroupBox(tr("Conservation of Mass and Momentum"));
    ninjafoamGroupBox->setCheckable(true);
    ninjafoamGroupBox->setChecked(false);
    
    ninjafoamLabel = new QLabel(tr("This solver conserves both mass and momentum. It is based on the OpenFOAM\n"
                "CFD toolbox. This solver should give more accurate wind predictions in regions where\n"
                "momentum effects are important, such as on the lee side of terrain obstacles. Because\n"
                "this solver is more computationally intensive than the conservation of mass solver,\n"
                "simulation times will be longer. Typical simulation times for this solver range from\n"
                "10-30 min, but will depend on your domain, resolution, and computational\n"
                "resources. Note that some options (e.g., point initialization and non-neutral\n"
                "stability) are not available for this solver at this time. We plan to make these options\n"
                "available in future releases."
                ), this);
    
    ninjafoamLayout = new QVBoxLayout;
  
    ninjafoamGroupBox->setLayout(ninjafoamLayout);
    
    layout = new QVBoxLayout;
    layout->addWidget(ninjafoamGroupBox);
    layout->addWidget(ninjafoamLabel);
    layout->addStretch();
    setLayout(layout);

    
}
