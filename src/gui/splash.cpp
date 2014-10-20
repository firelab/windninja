/******************************************************************************
 *
 * $Id$
 *
 * Project:  WindNinja Qt GUI
 * Purpose:  Splash screen that shows a pixmap and n messages for m seconds 
 *           each.
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

#include "splash.h"

/**
 * \brief Create the splash screen
 *
 * Display an image with text in the upper left corner.  The time is the total
 * each message is displayed.  Total time is time * list.size()
 *
 * \param pixmap Image to display
 * \param list of strings to display
 * \param time to display each message in milliseconds
 */
splashScreen::splashScreen(const QPixmap &pixmap, QStringList list, int time)
    : QSplashScreen(pixmap)
{
    stringList = list;
    messageTime = time;
    nMessages = stringList.size();
    i = 0;
    messageTimer = new QTimer;
    alignment = Qt::AlignLeft | Qt::AlignTop;
    setFocus(Qt::OtherFocusReason);
}

/**
 * \brief Show the splash screen
 *
 * Show the image and start the timer, increment through the messages
 */
void splashScreen::display()
{
    show();
    messageTimer->start(messageTime);
    connect(messageTimer, SIGNAL(timeout()), this, SLOT(update()));
}

/**
 * \brief Change the label until there are no more labels.
 *
 * Close the splash after the last label
 */
void splashScreen::update()
{
    if(i < nMessages)
    {
        showMessage(stringList[i]);
        i++;
        }
    else
    {
        messageTimer->stop();
        emit done();
        finish(this);
    }
}

/**
 * \brief Close the image on left button mouse event
 *
 * \param event mouse event to process
 */
void splashScreen::mousePressEvent(QMouseEvent *event)
{
    if(event->buttons() & Qt::LeftButton)
    {
        emit done();
        finish(this);
    }
    else
        return;
}

