/******************************************************************************
 * 
 * $Id$
 *
 * Project:  WindNinja Qt GUI
 * Purpose:  OpenGL implementation for viewing DEM inputs
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

#include "diurnalInput.h"

/**
 * \brief Construct and layout the diurnalInput widget.
 *
 * This is only a checkable option now.
 *
 * \param parent parent widget
 */
diurnalInput::diurnalInput(QWidget *parent) : QWidget(parent)
{
    diurnalGroupBox = new QGroupBox(tr("Use Diurnal Wind"));
    diurnalGroupBox->setCheckable(true);
    diurnalGroupBox->setChecked(false);
    diurnalLayout = new QVBoxLayout;

    ninjafoamConflictLabel = new QLabel(tr("The diurnal wind option is not currently available for the momentum solver.\n"
        ), this);
    ninjafoamConflictLabel->setHidden(true);

    diurnalGroupBox->setLayout(diurnalLayout);

    layout = new QVBoxLayout;
    layout->addWidget(diurnalGroupBox);
    layout->addWidget(ninjafoamConflictLabel);
    layout->addStretch();
    setLayout(layout);
}

