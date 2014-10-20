/******************************************************************************
 *
 * $Id$
 *
 * Project:  WindNinja Qt GUI
 * Purpose:  Console output window
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

#include "consoleDockWidget.h"

/**
 * \brief Construct a default console
 *
 * Set the docking options and default text edit options.
 *
 * \param parent Parent of the new widget
 */
ConsoleDockWidget::ConsoleDockWidget(QWidget *parent) : QDockWidget(parent)
{
    setWindowTitle(tr("Console Output"));
    consoleTextEdit = new QTextEdit(this);
    consoleTextEdit->setDocumentTitle(tr("Console Output"));
    consoleTextEdit->setMaximumHeight(80);
    consoleTextEdit->setMinimumWidth(480);
    consoleTextEdit->setSizePolicy(QSizePolicy::Preferred,
                                   QSizePolicy::Maximum);
    setAllowedAreas(Qt::BottomDockWidgetArea | Qt::RightDockWidgetArea |
                    Qt::NoDockWidgetArea);

    //connect dockAreaChanged and checkSize()
    connect(this, SIGNAL(topLevelChanged(bool)), this, SLOT(checkSize(bool)));
 
  setWidget(consoleTextEdit);
}

/**
 * \briefChange the allowable size of the widget if it is floating
 *
 * \param float status
 */
void ConsoleDockWidget::checkSize(bool floating)
{
    if(!floating)
    {
        consoleTextEdit->setMaximumHeight(80);
        consoleTextEdit->setMinimumWidth(480);
        consoleTextEdit->setSizePolicy(QSizePolicy::Preferred,
                                       QSizePolicy::Maximum);
    }
    else
    {
        consoleTextEdit->setMaximumHeight(720);
        consoleTextEdit->setMinimumWidth(480);
        consoleTextEdit->setSizePolicy(QSizePolicy::Preferred,
                                       QSizePolicy::Preferred);
    }
}

