/******************************************************************************
 *
 * $Id$
 *
 * Project:  WindNinja Qt GUI
 * Purpose:  Handles options for wind inputs
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

#include "windInput.h"

windInput::windInput(QWidget *parent) : QWidget(parent)
{
    windGroupBox = new QGroupBox( "Domain Average Wind" );
    windGroupBox->setCheckable( true );
    windGroupBox->setChecked(true);
    metaWind = new metaWindWidget;
    
    windTable = new windInputTable;
    
    scrollArea = new QScrollArea;
    scrollArea->setWidget(windTable);

    clearButton = new QToolButton(this);
    clearButton->setIcon(QIcon(":cancel.png"));
    clearButton->setToolButtonStyle( Qt::ToolButtonTextBesideIcon );
    clearButton->setText("Clear");
    clearButton->setToolTip("Clear all entries");

    windLayout = new QVBoxLayout;
    clearLayout = new QHBoxLayout;
    mainLayout = new QVBoxLayout;

    clearLayout->addStretch();
    clearLayout->addWidget(clearButton);

    windLayout->addWidget(metaWind);
    windLayout->addLayout(clearLayout);
    windLayout->addWidget(scrollArea);

    windGroupBox->setLayout( windLayout );

    mainLayout->addWidget( windGroupBox );
    setLayout(mainLayout);

    connect(clearButton, SIGNAL(clicked()), windTable, SLOT(clear()));
}
