 /******************************************************************************
 *
 * $Id$
 *
 * Project:  WindNinja Qt GUI
 * Purpose:  Hands GUI related logic for the Weather Model Page
 * Author:   Mason Willman <mason.willman@usda.gov>
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

#ifndef WEATHERMODELINPUT_H
#define WEATHERMODELINPUT_H

#include "ui_mainwindow.h"
#include "windninja.h"
#include "appstate.h"
#include "QTimeZone"
#include "QFileSystemModel"
#include <QStandardItemModel>
#include <QFuture>
#include <QFutureWatcher>
#include <QProgressDialog>
#include <QObject>

namespace Ui {
class MainWindow;
}

class WeatherModelInput : public QObject
{
    Q_OBJECT
public:
    explicit WeatherModelInput(Ui::MainWindow* ui, QObject* parent = nullptr);

signals:
    void requestRefresh();

public slots:
    void updateTreeView();


private slots:
    void weatherModelDownloadButtonClicked();
    void weatherModelFileTreeViewItemSelectionChanged(const QItemSelection &selected);
    void weatherModelTimeSelectAllButtonClicked();
    void weatherModelTimeSelectNoneButtonClicked();
    void weatherModelGroupBoxToggled(bool toggled);
    void weatherModelComboBoxCurrentIndexChanged(int index);

private:
    Ui::MainWindow *ui;
    NinjaToolsH* ninjaTools;
    NinjaErr ninjaErr;
    QFileSystemModel *fileModel;
    QStandardItemModel *timeModel;
    QProgressDialog *progress;
    QFutureWatcher<int> *futureWatcher;
};

#endif // WEATHERMODELINPUT_H
