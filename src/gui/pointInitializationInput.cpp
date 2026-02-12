 /******************************************************************************
 *
 * $Id$
 *
 * Project:  WindNinja Qt GUI
 * Purpose:  Handles GUI related logic for Point Initialization Page
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

#include "pointInitializationInput.h"

PointInitializationInput::PointInitializationInput(Ui::MainWindow* ui, QObject* parent)
    : QObject(parent),
    ui(ui)
{    
    ui->pointInitializationDataTimeStackedWidget->setCurrentIndex(0);
    ui->weatherStationDataSourceStackedWidget->setCurrentIndex(0);
    ui->weatherStationDataTimeStackedWidget->setCurrentIndex(0);
    ui->weatherStationDataStartDateTimeEdit->setDateTime(QDateTime::currentDateTime());
    ui->weatherStationDataEndDateTimeEdit->setDateTime(QDateTime::currentDateTime());

    updateDateTime();

    connect(ui->pointInitializationGroupBox, &QGroupBox::toggled, this, &PointInitializationInput::pointInitializationGroupBoxToggled);
    connect(ui->pointInitializationDownloadDataButton, &QPushButton::clicked, this, &PointInitializationInput::pointInitializationDownloadDataButtonClicked);
    connect(ui->weatherStationDataDownloadCancelButton, &QPushButton::clicked, this, &PointInitializationInput::weatherStationDataDownloadCancelButtonClicked);
    connect(ui->weatherStationDataSourceComboBox, &QComboBox::currentIndexChanged, this, &PointInitializationInput::weatherStationDataSourceComboBoxCurrentIndexChanged);
    connect(ui->weatherStationDataTimeComboBox, &QComboBox::currentIndexChanged, this, &PointInitializationInput::weatherStationDataTimeComboBoxCurrentIndexChanged);
    connect(ui->weatherStationDataDownloadButton, &QPushButton::clicked, this, &PointInitializationInput::weatherStationDataDownloadButtonClicked);
    connect(ui->pointInitializationSelectAllButton, &QPushButton::clicked, this, &PointInitializationInput::pointInitializationSelectAllButtonClicked);
    connect(ui->pointInitializationSelectNoneButton, &QPushButton::clicked, this, &PointInitializationInput::pointInitializationSelectNoneButtonClicked);
    connect(ui->pointInitializationTreeView, &QTreeView::expanded, this, &PointInitializationInput::folderExpanded);
    connect(ui->pointInitializationTreeView, &QTreeView::collapsed, this, &PointInitializationInput::folderCollapsed);
    connect(ui->weatherStationDataTimestepsSpinBox, &QSpinBox::valueChanged, this, &PointInitializationInput::weatherStationDataTimestepsSpinBoxValueChanged);
    connect(ui->timeZoneComboBox, &QComboBox::currentIndexChanged, this, &PointInitializationInput::updateDateTime);
    connect(this, &PointInitializationInput::updateState, &AppState::instance(), &AppState::updatePointInitializationInputState);

    connect(this, &PointInitializationInput::updateProgressMessageSignal, this, &PointInitializationInput::updateProgressMessage, Qt::QueuedConnection);
}

void PointInitializationInput::pointInitializationGroupBoxToggled(bool toggled)
{
    AppState& state = AppState::instance();

    state.isPointInitializationToggled = toggled;
    if (toggled)
    {
        ui->domainAverageGroupBox->setChecked(false);
        ui->weatherModelGroupBox->setChecked(false);
        state.isDomainAverageInitializationToggled = ui->domainAverageGroupBox->isChecked();
        state.isWeatherModelInitializationToggled = ui->weatherModelGroupBox->isChecked();
    }

    emit updateState();
}

void PointInitializationInput::pointInitializationDownloadDataButtonClicked()
{
    ui->weatherStationDataSourceComboBox->setCurrentIndex(0);
    ui->weatherStationDataTimeComboBox->setCurrentIndex(0);

    ui->downloadFromStationIDLineEdit->clear();
    ui->downloadFromDEMSpinBox->setValue(0);

    ui->inputsStackedWidget->setCurrentIndex(17);
}

void PointInitializationInput::weatherStationDataDownloadCancelButtonClicked()
{
    ui->pointInitializationTreeView->collapseAll();
    ui->inputsStackedWidget->setCurrentIndex(7);
}

void PointInitializationInput::updateProgressMessage(const QString message)
{
//    QMessageBox::critical(
//        nullptr,
//        QApplication::tr("Error"),
//        message
//    );
    progress->setLabelText(message);
    progress->setWindowTitle(tr("Error"));
    progress->setCancelButtonText(tr("Close"));
    progress->setAutoClose(false);
    progress->setAutoReset(false);
    progress->setRange(0, 1);
    progress->setValue(progress->maximum());
}

static void comMessageHandler(const char *pszMessage, void *pUser)
{
    PointInitializationInput *self = static_cast<PointInitializationInput*>(pUser);

    std::string msg = pszMessage;
    if( msg.substr(msg.size()-1, 1) == "\n")
    {
        msg = msg.substr(0, msg.size()-1);
    }

    size_t pos;
    size_t startPos;
    size_t endPos;
    std::string clipStr;

    if( msg.find("Exception caught: ") != msg.npos || msg.find("ERROR: ") != msg.npos )
    {
        if( msg.find("Exception caught: ") != msg.npos )
        {
            pos = msg.find("Exception caught: ");
            startPos = pos+18;
        }
        else // if( msg.find("ERROR: ") != msg.npos )
        {
            pos = msg.find("ERROR: ");
            startPos = pos+7;
        }
        clipStr = msg.substr(startPos);
        //std::cout << "clipStr = \"" << clipStr << "\"" << std::endl;
        //emit self->updateProgressMessageSignal(QString::fromStdString(clipStr));
        //emit self->writeToConsoleSignal(QString::fromStdString(clipStr));
        if( clipStr == "Cannot determine exception type." )
        {
            emit self->updateProgressMessageSignal(QString::fromStdString("StationFetch ended with unknown error"));
            emit self->writeToConsoleSignal(QString::fromStdString("unknown StationFetch error"), Qt::red);
        }
        else
        {
            emit self->updateProgressMessageSignal(QString::fromStdString("StationFetch ended in error:\n"+clipStr));
            emit self->writeToConsoleSignal(QString::fromStdString("StationFetch error: "+clipStr), Qt::red);
        }
    }
    else if( msg.find("Warning: ") != msg.npos )
    {
        if( msg.find("Warning: ") != msg.npos )
        {
            pos = msg.find("Warning: ");
            startPos = pos+9;
        }
        clipStr = msg.substr(startPos);
        //std::cout << "clipStr = \"" << clipStr << "\"" << std::endl;
        //emit self->updateProgressMessageSignal(QString::fromStdString(clipStr));
        //emit self->writeToConsoleSignal(QString::fromStdString(clipStr));
        emit self->updateProgressMessageSignal(QString::fromStdString("StationFetch ended in warning:\n"+clipStr));
        emit self->writeToConsoleSignal(QString::fromStdString("StationFetch warning: "+clipStr), Qt::yellow);
    }
    else
    {
        emit self->updateProgressMessageSignal(QString::fromStdString(msg));
        emit self->writeToConsoleSignal(QString::fromStdString(msg));
    }
}

void PointInitializationInput::weatherStationDataDownloadButtonClicked()
{
    emit writeToConsoleSignal("Fetching station data...");

    progress = new QProgressDialog("Fetching Station Data...", QString(), 0, 0, ui->centralwidget);
    progress->setWindowModality(Qt::WindowModal);
    progress->setCancelButton(nullptr);
    progress->setMinimumDuration(0);
    progress->setAutoClose(true);
    progress->show();

    NinjaToolsH* ninjaTools = NinjaMakeTools();

    char **papszOptions = NULL;
    NinjaErr ninjaErr = NinjaSetToolsComMessageHandler(ninjaTools, &comMessageHandler, this, papszOptions);
    if(ninjaErr != NINJA_SUCCESS)
    {
        qDebug() << "NinjaSetToolsComMessageHandler(): ninjaErr =" << ninjaErr;
    }

    QString DEMTimeZone = ui->timeZoneComboBox->currentText();
    QByteArray DEMTimeZoneBytes = ui->timeZoneComboBox->currentText().toUtf8();
    QDateTime start = ui->downloadBetweenDatesStartTimeDateTimeEdit->dateTime();
    QDateTime end = ui->downloadBetweenDatesEndTimeDateTimeEdit->dateTime();

    QVector<int> year   = {start.date().year(),   end.date().year()};
    QVector<int> month  = {start.date().month(),  end.date().month()};
    QVector<int> day    = {start.date().day(),    end.date().day()};
    QVector<int> hour   = {start.time().hour(),   end.time().hour()};
    QVector<int> minute = {start.time().minute(), end.time().minute()};
    QVector<int> outYear(2), outMonth(2), outDay(2), outHour(2), outMinute(2);

    // this one should break so hard, the code won't even know what happened
    // yeah, a qt error this time, "ASSERT failure in QList::operator[]: "index out of range""
    // well, that's confusing. This breaking only happens if doing "qDebug() << outTime[1];" rather than just [0],
    // comment that out and it runs to completion fine for everything, it just uses the first time for everything.
    // looks like it just uses the single time, as either a latestTime or a time-series file format,
    //  depending on the choice of the input download type, latestTime or time-series
    /*QVector<int> year   = {start.date().year(),   end.date().year()};
    QVector<int> month  = {start.date().month(),  end.date().month()};
    QVector<int> day    = {start.date().day(),    end.date().day()};
    QVector<int> hour   = {start.time().hour(),   end.time().hour()};
    QVector<int> minute = {start.time().minute(), end.time().minute()};
    QVector<int> outYear(1), outMonth(1), outDay(1), outHour(1), outMinute(1);*/

    // this set will probably work fine
    // yup, worked fine. The time-series became a time-series from and to the same exact time
    /*QVector<int> year   = {start.date().year(),   start.date().year()};
    QVector<int> month  = {start.date().month(),  start.date().month()};
    QVector<int> day    = {start.date().day(),    start.date().day()};
    QVector<int> hour   = {start.time().hour(),   start.time().hour()};
    QVector<int> minute = {start.time().minute(), start.time().minute()};
    QVector<int> outYear(2), outMonth(2), outDay(2), outHour(2), outMinute(2);*/

    // this set will probably work fine
    // yup, worked fine. The time-series became a time-series from and to the same exact time
    /*QVector<int> year   = {end.date().year(),   end.date().year()};
    QVector<int> month  = {end.date().month(),  end.date().month()};
    QVector<int> day    = {end.date().day(),    end.date().day()};
    QVector<int> hour   = {end.time().hour(),   end.time().hour()};
    QVector<int> minute = {end.time().minute(), end.time().minute()};
    QVector<int> outYear(2), outMonth(2), outDay(2), outHour(2), outMinute(2);*/

    // the latestTime fetch just uses the first time from the list
    // the time-series fetch throws an error as expected, but the message seems to drop what is going on to cause the error
    /*QVector<int> year   = {start.date().year(),   start.date().year()-1};
    QVector<int> month  = {start.date().month(),  start.date().month()};
    QVector<int> day    = {start.date().day(),    start.date().day()};
    QVector<int> hour   = {start.time().hour(),   start.time().hour()};
    QVector<int> minute = {start.time().minute(), start.time().minute()};
    QVector<int> outYear(2), outMonth(2), outDay(2), outHour(2), outMinute(2);*/

    // should die pretty hard
    // well, it runs this part fine, just runs normally when run by the later fetch (because the fetch SHOULD die, but does not right now for this case),
    // unless it is a latestTime fetch in which case it just uses the first time from the list
    /*QVector<int> year   = {start.date().year(),   start.date().year()};
    QVector<int> month  = {start.date().month(),  start.date().month()};
    QVector<int> day    = {start.date().day(),    start.date().day()};
    QVector<int> hour   = {start.time().hour(),   start.time().hour()-1};
    QVector<int> minute = {start.time().minute(), start.time().minute()};
    QVector<int> outYear(2), outMonth(2), outDay(2), outHour(2), outMinute(2);*/

    // the latestTime fetch just uses the first time from the list
    // the time-series fetch throws an error as expected, but the message seems to drop what is going on to cause the error
    /*QVector<int> year   = {start.date().year(),   start.date().year()};
    QVector<int> month  = {start.date().month(),  start.date().month()};
    QVector<int> day    = {start.date().day(),    start.date().day()};
    QVector<int> hour   = {start.time().hour(),   start.time().hour()-2};  // had to use 3 instead of 2 for this to work, when I was right at and close to the hour
    QVector<int> minute = {start.time().minute(), start.time().minute()};
    QVector<int> outYear(2), outMonth(2), outDay(2), outHour(2), outMinute(2);*/

    // the latestTime fetch just uses the first time from the list
    // the time-series fetch throws an error as expected, but the message seems to drop what is going on to cause the error
    /*QVector<int> year   = {end.date().year()+1,   end.date().year()};
    QVector<int> month  = {end.date().month(),  end.date().month()};
    QVector<int> day    = {end.date().day(),    end.date().day()};
    QVector<int> hour   = {end.time().hour(),   end.time().hour()};
    QVector<int> minute = {end.time().minute(), end.time().minute()};
    QVector<int> outYear(2), outMonth(2), outDay(2), outHour(2), outMinute(2);*/

    // the latestTime fetch just uses the first time from the list
    // the time-series fetch throws an error as expected, but the message seems to drop what is going on to cause the error
    /*QVector<int> year   = {end.date().year(),   end.date().year()};
    QVector<int> month  = {end.date().month(),  end.date().month()};
    QVector<int> day    = {end.date().day(),    end.date().day()};
    QVector<int> hour   = {end.time().hour()+1,   end.time().hour()};
    QVector<int> minute = {end.time().minute(), end.time().minute()};
    QVector<int> outYear(2), outMonth(2), outDay(2), outHour(2), outMinute(2);*/

    ninjaErr = NinjaGetTimeList(
        ninjaTools,
        year.data(), month.data(), day.data(),
        hour.data(), minute.data(),
        outYear.data(), outMonth.data(), outDay.data(),
        outHour.data(), outMinute.data(),
        2, DEMTimeZoneBytes.data()
        );
    //ninjaErr = NinjaGetTimeList(
    //    ninjaTools,
    //    year.data(), month.data(), day.data(),
    //    hour.data(), minute.data(),
    //    outYear.data(), outMonth.data(), outDay.data(),
    //    outHour.data(), outMinute.data(),
    //    1, DEMTimeZoneBytes.data()
    //    );  // this one shouldn't hurt anything, but want to see what happens  // gives an error, for both latestTime and time-series: "Day of month value is out of range 1..31".
    //ninjaErr = NinjaGetTimeList(
    //    ninjaTools,
    //    year.data(), month.data(), day.data(),
    //    hour.data(), minute.data(),
    //    outYear.data(), outMonth.data(), outDay.data(),
    //    outHour.data(), outMinute.data(),
    //    2, "fudge"
    //    );  // should break, but probably won't, will probably be treated like the dem timezone  // turns out it breaks HARD, a smart pointer failing on assert somewhere along the pipeline, not sure if that occurs here, or later down the pipeline. Definitely a break not necessarily related directly with the dem, breaks probably because of something sized wrong because of the dem being off, or it breaks because it IS trying to read the dem even though it shouldn't. And it gets past the try/catch error handling stuff, hrm.
    if(ninjaErr != NINJA_SUCCESS)
    {
        qDebug() << "NinjaGetTimeList: ninjaErr =" << ninjaErr;

        progress->setWindowTitle(tr("Error"));
        progress->setCancelButtonText("Close");
        progress->setAutoClose(false);
        progress->setAutoReset(false);
        progress->setRange(0, 1);
        progress->setValue(progress->maximum());

        // do cleanup before the return, similar to finishedSolve()

//        ninjaErr = NinjaDestroyTools(ninjaTools, papszOptions);
//        if(ninjaErr != NINJA_SUCCESS)
//        {
//            printf("NinjaDestroyTools: ninjaErr = %d\n", ninjaErr);
//        }

        //futureWatcher->deleteLater();

        return;
    }

    if(ui->weatherStationDataTimeComboBox->currentIndex() == 1) // TODO: Add proper error handling for a bad time duration (someone downloads too much data)
    {
        //outYear[0] = outYear[1]-2;  // doing more than a year worth of time SHOULD be what triggers it, this is NOT a check on the times themselves  // hrm, it returns an error code of 8, but no ninjaCom seems to be sent so it leaves it hanging without a proper message. runs fine for a latestTime simulation.
        //outYear[1] = outYear[0]+2;  // not sure if it can even handle when the endYear goes past the current year  // same result as the above test.
        ninjaErr = NinjaCheckTimeDuration(ninjaTools, outYear.data(), outMonth.data(), outDay.data(), outHour.data(), outMinute.data(), 2, papszOptions);
        if(ninjaErr != NINJA_SUCCESS)
        {
            qDebug() << "NinjaCheckTimeDuration ninjaErr=" << ninjaErr;

            progress->setWindowTitle(tr("Error"));
            progress->setCancelButtonText("Close");
            progress->setAutoClose(false);
            progress->setAutoReset(false);
            progress->setRange(0, 1);
            progress->setValue(progress->maximum());

            // do cleanup before the return, similar to finishedSolve()

//            ninjaErr = NinjaDestroyTools(ninjaTools, papszOptions);
//            if(ninjaErr != NINJA_SUCCESS)
//            {
//                printf("NinjaDestroyTools: ninjaErr = %d\n", ninjaErr);
//            }

            //futureWatcher->deleteLater();

            return;
        }
    }

    bool fetchLatestFlag = ui->weatherStationDataTimeComboBox->currentIndex() ? 0 : 1;
    QString outputPath = ui->outputDirectoryLineEdit->text();
    QString elevationFile = ui->elevationInputFileLineEdit->property("fullpath").toString();

    futureWatcher = new QFutureWatcher<int>(this);
    QFuture<int> future;
    if(ui->weatherStationDataSourceComboBox->currentIndex() == 0)
    {
        QString units = ui->downloadFromDEMComboBox->currentText();
        double buffer = ui->downloadFromDEMSpinBox->value();
        future = QtConcurrent::run(&PointInitializationInput::fetchStationFromBbox, this,
                                   ninjaTools,
                                   outYear, outMonth, outDay, outHour, outMinute,
                                   elevationFile, buffer, units,
                                   DEMTimeZone, fetchLatestFlag, outputPath);
    }
    else
    {
        QString stationList = ui->downloadFromStationIDLineEdit->text();
        future = QtConcurrent::run(&PointInitializationInput::fetchStationByName, this,
                                   ninjaTools,
                                   outYear, outMonth, outDay, outHour, outMinute,
                                   elevationFile, stationList,
                                   DEMTimeZone, fetchLatestFlag, outputPath);
    }
    futureWatcher->setFuture(future);

    connect(futureWatcher, &QFutureWatcher<int>::finished, this, &PointInitializationInput::fetchStationDataFinished);
}

int PointInitializationInput::fetchStationFromBbox(NinjaToolsH* ninjaTools,
                                                   QVector<int> year,
                                                   QVector<int> month,
                                                   QVector<int> day,
                                                   QVector<int> hour,
                                                   QVector<int> minute,
                                                   QString elevationFile,
                                                   double buffer,
                                                   QString units,
                                                   QString osTimeZone,
                                                   bool fetchLatestFlag,
                                                   QString outputPath)
{
    // apparently if it is a latestTime run, changing the times means nothing, they get ignored
    // though looks like the values that find themselves in there before editing for a latestTime run,
    // are   endDateTime of current time         , in UTC
    // and startDateTime of current time - 1 hour, in UTC
    // meaning that setting a latestTime run to a multi-time series, just runs fine and acts like a multi-time series
    // hrm, when redownloading of the same type, multi-time series, but differing times, the same folder and sometimes filenames end up getting used, from the first download attempt. Seems like badly defined behavior, might be related to the stations info being a set of static variables? I'm not sure what is going on there, the inputs to the function ARE the proper updated set of times.
    // hrm, in all cases, failing or otherwise, the wxStation folder ends up still getting written, does not end up getting cleaned up after the failing of the run.

    //fetchLatestFlag = TRUE;  // on a single-time series, what will happen?  // Turns out it runs fine, just uses the start time of the multi-time series. So kind of a useless test.
    //fetchLatestFlag = FALSE;  // try on a multi-time series, with good inputs or no, what will happen?  // Turns out it runs fine, because the default values are a time series that is normally ignored. kind of behaves like a useful test, though that depends on how the inputs could change.

    //year[1] = year[0]-1;  // throws an error as expected, though the message seems to drop what is going on to cause the error. The folder is still created, doesn't get cleaned up after the download fails.
    //hour[1] = hour[0]-1;  // need to carefully set the date to the same date for both times for this test to properly work. apparently this test SHOULD break, but something must be up in how it is handled within the ninja code because it ends up just downloading a single dateTime, of the earliest of the two sets of times.
    //hour[1] = hour[0]-2;  // throws an error as expected, though the message seems to drop what is going on to cause the error, but had to use 3 instead of 2 for this to work, when I was right at and close to the hour. need to carefully set the date to the same date for both times for this test to properly work.
    //year[0] = year[1]+1;  // throws an error as expected, though the message seems to drop what is going on to cause the error
    //hour[0] = hour[1]+1;  // need to carefully set the date to the same date for both times for this test to properly work. apparently this test SHOULD break, but something must be up in how it is handled within the ninja code because it ends up just downloading a single dateTime, of the earliest of the two sets of times. Seems to work better when not close to the hour.
    //hour[0] = hour[1]+2;  // throws an error as expected. need to carefully set the date to the same date for both times for this test to properly work.
    //elevationFile = "fudge";  // throws error as expected, after a message of "ERROR 4: fudge: No such file or directory", BUT, the message seems to drop what is going on to cause the error
    //buffer = -1;  // runs fine, no error messages, just downloads the data within the area of the dem
    //units = "fudge";  // runs fine, no error messages, just downloads the data within the area of the dem
    //osTimeZone = "fudge";  // runs fine, no error messages, seems to just download the data assuming the timezone of the dem

    //year[0] = 9999;  // throws an error as expected, though the message seems to drop what is going on to cause the error
    //hour[0] = 9999;  // throws an error as expected, though the message seems to drop what is going on to cause the error
    //year[1] = 9999;  // throws an error as expected, though the message seems to drop what is going on to cause the error
    //hour[1] = 9999;  // um, this one did NOT throw an error, but actually ran successfully, creating data as if the endTime was the same thing as the startTime
    //year[0] = -1;  // throws an error as expected, and actually a good informative error this time
    //hour[0] = -1;  // um, this one did NOT throw an error, but actually ran successfully, creating data as if the startHour was the same thing as the endHour
    //year[1] = -1;  // throws an error as expected, and actually a good informative error this time
    //hour[1] = -1;  // um, this one did NOT throw an error, but actually ran successfully, creating data as if the endHour was the same thing as the startHour
    //year[0] = 0;  // throws an error as expected, and actually a good informative error this time
    //year[1] = 0;  // throws an error as expected, and actually a good informative error this time

    //qDebug() << "year[0] month[0] day[0] hour[0] minute[0] =" << year[0] << month[0] << day[0] << hour[0] << minute[0];
    //qDebug() << "year[1] month[1] day[1] hour[1] minute[1] =" << year[1] << month[1] << day[1] << hour[1] << minute[1];

    char ** options = NULL;
    NinjaErr ninjaErr = NinjaFetchStationFromBBox(
        ninjaTools,
        year.data(), month.data(), day.data(),
        hour.data(), minute.data(), year.size(),
        elevationFile.toUtf8().constData(), buffer,
        units.toUtf8().constData(), osTimeZone.toUtf8().constData(),
        fetchLatestFlag, outputPath.toUtf8().constData(),
        false, options
        );

    if (ninjaErr != NINJA_SUCCESS)
    {
        qDebug() << "NinjaFetchStationFromBbox: ninjaErr =" << ninjaErr;
    }

    return ninjaErr;
}

int PointInitializationInput::fetchStationByName(NinjaToolsH* ninjaTools,
                                                 QVector<int> year,
                                                 QVector<int> month,
                                                 QVector<int> day,
                                                 QVector<int> hour,
                                                 QVector<int> minute,
                                                 QString elevationFile,
                                                 QString stationList,
                                                 QString osTimeZone,
                                                 bool fetchLatestFlag,
                                                 QString outputPath)
{
    //stationList = "KMSO,PNTM8";  // this is a working list, what the list SHOULD be, and it works great. But apparently doing so with a SPACE between the commas of the stations, even though that is what is shown in the GUI as an example, DOES break the code.
    //stationList = "KMSO, PNTM8";  // um, this should NOT thrown an error, but it does. So I guess something is wrong with how the files are parsed. Runs fine for a single station. The error message also drops telling what is going on, so annoying.

    // apparently if it is a latestTime run, changing the times means nothing, they get ignored
    // though looks like the values that find themselves in there before editing for a latestTime run,
    // are   endDateTime of current time         , in UTC
    // and startDateTime of current time - 1 hour, in UTC
    // meaning that setting a latestTime run to a multi-time series, just runs fine and acts like a multi-time series
    // hrm, when redownloading of the same type, multi-time series, but differing times, the same folder and sometimes filenames end up getting used, from the first download attempt. Seems like badly defined behavior, might be related to the stations info being a set of static variables? I'm not sure what is going on there, the inputs to the function ARE the proper updated set of times.
    // hrm, in all cases, failing or otherwise, the wxStation folder ends up still getting written, does not end up getting cleaned up after the failing of the run.

    //fetchLatestFlag = TRUE;  // on a single-time series, what will happen?  // Turns out it runs fine, just uses the start time of the multi-time series. So kind of a useless test.
    //fetchLatestFlag = FALSE;  // try on a multi-time series, with good inputs or no, what will happen?  // Turns out it runs fine, because the default values are a time series that is normally ignored. kind of behaves like a useful test, though that depends on how the inputs could change.

    //year[1] = year[0]-1;  // throws an error as expected, though the message seems to drop what is going on to cause the error. The folder is still created, doesn't get cleaned up after the download fails.
    //hour[1] = hour[0]-1;  // need to carefully set the date to the same date for both times for this test to properly work. apparently this test SHOULD break, but something must be up in how it is handled within the ninja code because it ends up just downloading a single dateTime, of the earliest of the two sets of times.
    //hour[1] = hour[0]-2;  // throws an error as expected, though the message seems to drop what is going on to cause the error, but had to use 3 instead of 2 for this to work, when I was right at and close to the hour. need to carefully set the date to the same date for both times for this test to properly work.
    //year[0] = year[1]+1;  // throws an error as expected, though the message seems to drop what is going on to cause the error
    //hour[0] = hour[1]+1;  // need to carefully set the date to the same date for both times for this test to properly work. apparently this test SHOULD break, but something must be up in how it is handled within the ninja code because it ends up just downloading a single dateTime, of the earliest of the two sets of times. Seems to work better when not close to the hour.
    //hour[0] = hour[1]+2;  // throws an error as expected. need to carefully set the date to the same date for both times for this test to properly work.
    //elevationFile = "fudge";  // um, this is supposed to have thrown an error, but it didn't, data somehow downloaded fine with fudge in the name. Oh I see, it just uses the stations that are chosen to be downloaded.
    //stationList = "fudge";  // throws error as expected, though the message seems to drop what is going on to cause the error.
    //osTimeZone = "fudge";  // runs fine, no error messages, seems to just download the data assuming the timezone of the dem

    //year[0] = 9999;  // throws an error as expected, though the message seems to drop what is going on to cause the error
    //hour[0] = 9999;  // throws an error as expected, though the message seems to drop what is going on to cause the error
    //year[1] = 9999;  // throws an error as expected, though the message seems to drop what is going on to cause the error
    //hour[1] = 9999;  // um, this one did NOT throw an error, but actually ran successfully, creating data as if the endTime was the same thing as the startTime
    //year[0] = -1;  // throws an error as expected, and actually a good informative error this time
    //hour[0] = -1;  // um, this one did NOT throw an error, but actually ran successfully, creating data as if the startHour was the same thing as the endHour
    //year[1] = -1;  // throws an error as expected, and actually a good informative error this time
    //hour[1] = -1;  // um, this one did NOT throw an error, but actually ran successfully, creating data as if the endHour was the same thing as the startHour
    //year[0] = 0;  // throws an error as expected, and actually a good informative error this time
    //year[1] = 0;  // throws an error as expected, and actually a good informative error this time

    //qDebug() << "year[0] month[0] day[0] hour[0] minute[0] =" << year[0] << month[0] << day[0] << hour[0] << minute[0];
    //qDebug() << "year[1] month[1] day[1] hour[1] minute[1] =" << year[1] << month[1] << day[1] << hour[1] << minute[1];

    char ** options = NULL;
    NinjaErr ninjaErr = NinjaFetchStationByName(
        ninjaTools,
        year.data(), month.data(), day.data(),
        hour.data(), minute.data(), year.size(),
        elevationFile.toUtf8().constData(), stationList.toUtf8().constData(),
        osTimeZone.toUtf8().constData(), fetchLatestFlag,
        outputPath.toUtf8().constData(), false, options
        );

    if (ninjaErr != NINJA_SUCCESS)
    {
        qDebug() << "NinjaFetchStationFromBbox: ninjaErr =" << ninjaErr;
    }

    return ninjaErr;
}

void PointInitializationInput::fetchStationDataFinished()
{
    // get the return value of the QtConcurrent::run() function
    int result = futureWatcher->future().result();

    if(result == NINJA_SUCCESS)
    {
        emit writeToConsoleSignal("Finished fetching station data.", Qt::darkGreen);

        if (progress)
        {
            progress->close();
            progress->deleteLater();
            progress = nullptr;
        }

        ui->inputsStackedWidget->setCurrentIndex(7);

    } else
    {
        emit writeToConsoleSignal("Failed to fetch station data.");
    }

    // delete the futureWatcher every time, whether success or failure
    if (futureWatcher)
    {
        futureWatcher->deleteLater();
        futureWatcher = nullptr;
    }
}

void PointInitializationInput::weatherStationDataSourceComboBoxCurrentIndexChanged(int index)
{
    ui->weatherStationDataSourceStackedWidget->setCurrentIndex(index);
}

void PointInitializationInput::weatherStationDataTimeComboBoxCurrentIndexChanged(int index)
{
    ui->weatherStationDataTimeStackedWidget->setCurrentIndex(index);
}

void PointInitializationInput::updateTreeView()
{
    AppState& state = AppState::instance();
    state.isStationFileSelectionValid = false;
    emit updateState();

    stationFileSystemModel = new QFileSystemModel(this);
    QString path = ui->elevationInputFileLineEdit->property("fullpath").toString();
    QFileInfo fileInfo(path);

    stationFileSystemModel->setRootPath(fileInfo.absolutePath());
    stationFileSystemModel->setNameFilters({"*.csv", "WXSTATIONS-*"});
    stationFileSystemModel->setFilter(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot);
    stationFileSystemModel->setNameFilterDisables(false);


    ui->pointInitializationTreeView->setModel(stationFileSystemModel);
    ui->pointInitializationTreeView->setRootIndex(stationFileSystemModel->index(fileInfo.absolutePath()));
    ui->pointInitializationTreeView->setSelectionMode(QAbstractItemView::MultiSelection);
    ui->pointInitializationTreeView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->pointInitializationTreeView->setAnimated(true);
    ui->pointInitializationTreeView->header()->setSectionResizeMode(QHeaderView::Stretch);
    ui->pointInitializationTreeView->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->pointInitializationTreeView->header()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    ui->pointInitializationTreeView->setUniformRowHeights(true);
    ui->pointInitializationTreeView->hideColumn(1);
    ui->pointInitializationTreeView->hideColumn(2);

    connect(ui->pointInitializationTreeView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &PointInitializationInput::pointInitializationTreeViewItemSelectionChanged);
}

void PointInitializationInput::pointInitializationTreeViewItemSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    AppState& state = AppState::instance();
    QModelIndexList selectedRows = ui->pointInitializationTreeView->selectionModel()->selectedRows();

    stationFiles.clear();
    stationFileTypes.clear();

    maxStationTime = QDateTime();
    minStationTime = QDateTime();

    state.isStationFileSelected = false;
    if (selectedRows.count() > 0)
    {
        state.isStationFileSelected = true;
    }

    for(int i = 0; i < selectedRows.count(); i++)
    {
        if(stationFileSystemModel->isDir(selectedRows[i]))
        {
            ui->pointInitializationTreeView->selectionModel()->select(selectedRows[i], QItemSelectionModel::Deselect | QItemSelectionModel::Rows);
            return;
        }

        QString recentFileSelected = stationFileSystemModel->filePath(selectedRows[i]);
        stationFiles.push_back(recentFileSelected);
        //qDebug() << "[GUI-Point] Selected file path:" << recentFileSelected;

        QByteArray filePathBytes = recentFileSelected.toUtf8();
        const char* filePath = filePathBytes.constData();
        char** options = nullptr;
        int stationHeader = NinjaGetWxStationHeaderVersion(filePath, options);
        //qDebug() << "[GUI-Point] Station Header: " << stationHeader;

        bool timeSeriesFlag = true;
        if (stationHeader != 1)
        {
            GDALDataset* hDS = (GDALDataset*) GDALOpenEx(
                filePath,
                GDAL_OF_VECTOR | GDAL_OF_READONLY,
                NULL,
                NULL,
                NULL
            );

            OGRLayer* poLayer = hDS->GetLayer(0);
            poLayer->ResetReading();
            qint64 lastIndex = poLayer->GetFeatureCount();
            //qDebug() << "[GUI-Point] Number of Time Entries:" << lastIndex;

            OGRFeature* poFeature = poLayer->GetFeature(1);         // Skip header, row 1 is first time in series
            QString startDateTime(poFeature->GetFieldAsString(15)); // Time should be in 15th (last) column (0-14)
            //qDebug() << "[GUI-Point] Station start time:" << startDateTime;

            poFeature = poLayer->GetFeature(lastIndex);             // last time in series
            QString stopDateTime(poFeature->GetFieldAsString(15));
            //qDebug() << "[GUI-Point] Station end Time:" << stopDateTime;

            if (startDateTime.isEmpty() && stopDateTime.isEmpty()) // No time series
            {
                //qDebug() << "[GUI-Point] File cannot be used for Time Series";
                timeSeriesFlag = false;
                stationFileTypes.push_back(0);
            }
            else if (!startDateTime.isEmpty() && !stopDateTime.isEmpty()) // Some type of time series
            {
                //qDebug() << "[GUI-Point] File can be used for Time Series, suggesting time series parameters...";
                readStationTime(startDateTime, stopDateTime);
                stationFileTypes.push_back(1);
            }
        }

        ui->pointInitializationDataTimeStackedWidget->setCurrentIndex(timeSeriesFlag ? 0 : 1);

        if (!timeSeriesFlag)
        {
            QDateTime dateModified = QFileInfo(recentFileSelected).birthTime();
            ui->weatherStationDataLabel->setText("Simulation time set to: " + dateModified.toString());
            ui->weatherStationDataLabel->setProperty("simulationTime", dateModified);
        }
        ui->pointInitializationTreeView->setProperty("timeSeriesFlag", timeSeriesFlag);
    }

    state.isStationFileSelectionValid = true;
    for (int i = 0; i < stationFileTypes.size(); i++)
    {
        if (stationFileTypes[i] != stationFileTypes[0]) {
            state.isStationFileSelectionValid = false;
            break;
        }
    }
    emit updateState();
}

void PointInitializationInput::pointInitializationSelectAllButtonClicked()
{
    QItemSelectionModel *selectionModel = ui->pointInitializationTreeView->selectionModel();

    QModelIndex folderIndex;
    if (openStationFolders.isEmpty())
    {
        QFileInfo fileInfo(ui->elevationInputFileLineEdit->property("fullpath").toString());
        folderIndex = stationFileSystemModel->index(fileInfo.absolutePath());
    }
    else
    {
        folderIndex = stationFileSystemModel->index(openStationFolders.back());
    }

    int rowCount = stationFileSystemModel->rowCount(folderIndex);
    QItemSelection selection;
    for (int i = 0; i < rowCount; i++) {
        QModelIndex stationFile = stationFileSystemModel->index(i, 0, folderIndex);

        if (stationFileSystemModel->isDir(stationFile))
            continue;

        selection.select(stationFile, stationFile);
    }

    selectionModel->select(selection, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
}

void PointInitializationInput::readStationTime(QString startDateTime, QString stopDateTime)
{
    QTimeZone timeZone(ui->timeZoneComboBox->currentText().toUtf8());

    QDateTime startTimeUTC = QDateTime::fromString(startDateTime, Qt::ISODate);
    QDateTime endTimeUTC   = QDateTime::fromString(stopDateTime, Qt::ISODate);

    QDateTime demStartTime = startTimeUTC.toTimeZone(timeZone);
    QDateTime demEndTime   = endTimeUTC.toTimeZone(timeZone);

    if (minStationTime.isNull() || demStartTime < minStationTime)
    {
        minStationTime = demStartTime;
    }
    if (maxStationTime.isNull() || demEndTime > maxStationTime)
    {
        maxStationTime = demEndTime;
    }

    ui->weatherStationMinTimeLabel->setText(
        "Current Min Time: " +
        minStationTime.toString("MM/dd/yy hh:mm") +
        " " + timeZone.abbreviation(demStartTime)
        );
    ui->weatherStationMaxTimeLabel->setText(
        "Current Max Time: " +
        maxStationTime.toString("MM/dd/yy hh:mm") +
        " " + timeZone.abbreviation(demEndTime)
        );

    QDateTime start = QDateTime(
        minStationTime.date(),
        minStationTime.time(),
        QTimeZone::systemTimeZone()
        );
    QDateTime end = QDateTime(
        maxStationTime.date(),
        maxStationTime.time(),
        QTimeZone::systemTimeZone()
        );

    ui->weatherStationDataStartDateTimeEdit->setDateTime(start);
    ui->weatherStationDataEndDateTimeEdit->setDateTime(end);

    ui->weatherStationDataStartDateTimeEdit->setEnabled(true);
    ui->weatherStationDataEndDateTimeEdit->setEnabled(true);

    int timesteps = qMax(2, static_cast<int>(minStationTime.secsTo(maxStationTime) / 3600));
    ui->weatherStationDataTimestepsSpinBox->setValue(timesteps);
    ui->weatherStationDataTimestepsSpinBox->setEnabled(true);
}

void PointInitializationInput::pointInitializationSelectNoneButtonClicked()
{
    ui->pointInitializationTreeView->selectionModel()->clearSelection();
}

void PointInitializationInput::folderExpanded(const QModelIndex &index)
{
    openStationFolders.push_back(stationFileSystemModel->filePath(index));
}

void PointInitializationInput::folderCollapsed(const QModelIndex &index)
{
    QString path = stationFileSystemModel->filePath(index);
    openStationFolders.removeOne(path);
}

void PointInitializationInput::weatherStationDataTimestepsSpinBoxValueChanged(int value)
{
    ui->weatherStationDataEndDateTimeEdit->setDisabled(value == 1);
}

QVector<QString> PointInitializationInput::getStationFiles()
{
    return stationFiles;
}

void PointInitializationInput::updateDateTime()
{
    // Update download date time info
    QTimeZone timeZone(ui->timeZoneComboBox->currentText().toUtf8());

    QDateTime demDateTime = QDateTime::currentDateTime().toTimeZone(timeZone);
    demDateTime = QDateTime(
        demDateTime.date(),
        demDateTime.time(),
        QTimeZone::systemTimeZone()
    );

    ui->downloadBetweenDatesStartTimeDateTimeEdit->setDateTime(demDateTime.addDays(-1));
    ui->downloadBetweenDatesEndTimeDateTimeEdit->setDateTime(demDateTime);

    // Update selected station time series
    QItemSelectionModel *selectionModel = ui->pointInitializationTreeView->selectionModel();
    if (!selectionModel || !selectionModel->hasSelection())
        return;

    pointInitializationTreeViewItemSelectionChanged(
        selectionModel->selection(),
        QItemSelection()
    );
}




