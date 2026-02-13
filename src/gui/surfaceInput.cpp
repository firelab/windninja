 /******************************************************************************
 *
 * $Id$
 *
 * Project:  WindNinja Qt GUI
 * Purpose:  Hands GUI related logic for the Surface Input Page
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

#include "surfaceInput.h"

SurfaceInput::SurfaceInput(Ui::MainWindow *ui,
                                   QWebEngineView *webEngineView,
                                   QObject* parent)
    : QObject(parent),
      ui(ui),
      webEngineView(webEngineView)
{
    ui->timeZoneDetailsTextEdit->setVisible(false);
    ui->vegetationStackedWidget->setCurrentIndex(0);

    timeZoneAllZonesCheckBoxClicked();

    connect(ui->boundingBoxNorthLineEdit, &QLineEdit::textChanged, this, &SurfaceInput::boundingBoxLineEditsTextChanged);
    connect(ui->boundingBoxSouthLineEdit, &QLineEdit::textChanged, this, &SurfaceInput::boundingBoxLineEditsTextChanged);
    connect(ui->boundingBoxEastLineEdit, &QLineEdit::textChanged, this, &SurfaceInput::boundingBoxLineEditsTextChanged);
    connect(ui->boundingBoxWestLineEdit, &QLineEdit::textChanged, this, &SurfaceInput::boundingBoxLineEditsTextChanged);

    connect(ui->pointRadiusLatLineEdit,&QLineEdit::textChanged, this, &SurfaceInput::pointRadiusLineEditsTextChanged);
    connect(ui->pointRadiusLonLineEdit,&QLineEdit::textChanged, this, &SurfaceInput::pointRadiusLineEditsTextChanged);
    connect(ui->pointRadiusRadiusLineEdit,&QLineEdit::textChanged, this, &SurfaceInput::pointRadiusLineEditsTextChanged);

    connect(ui->elevationInputFileDownloadButton, &QPushButton::clicked, this, &SurfaceInput::elevationInputFileDownloadButtonClicked);
    connect(ui->elevationInputFileOpenButton, &QPushButton::clicked, this, &SurfaceInput::elevationInputFileOpenButtonClicked);
    connect(ui->elevationInputFileLineEdit, &QLineEdit::textChanged, this, &SurfaceInput::elevationInputFileLineEditTextChanged);

    connect(ui->elevationInputTypeComboBox, &QComboBox::currentIndexChanged, ui->elevationInputTypeStackedWidget, &QStackedWidget::setCurrentIndex);
    connect(ui->meshResolutionComboBox, &QComboBox::currentIndexChanged, this, &SurfaceInput::meshResolutionComboBoxCurrentIndexChanged);
    connect(ui->meshResolutionUnitsComboBox, &QComboBox::currentIndexChanged, this, &SurfaceInput::meshResolutionUnitsComboBoxCurrentIndexChanged);
    connect(ui->timeZoneComboBox, &QComboBox::currentIndexChanged, this, &SurfaceInput::timeZoneComboBoxCurrentIndexChanged);

    connect(ui->surfaceInputDownloadCancelButton, &QPushButton::clicked, this, &SurfaceInput::surfaceInputDownloadCancelButtonClicked);
    connect(ui->surfaceInputDownloadButton, &QPushButton::clicked, this, &SurfaceInput::surfaceInputDownloadButtonClicked);
    connect(ui->elevationInputTypePushButton, &QPushButton::clicked, this, &SurfaceInput::elevationInputTypePushButtonClicked);
    connect(ui->timeZoneAllZonesCheckBox, &QCheckBox::clicked, this, &SurfaceInput::timeZoneAllZonesCheckBoxClicked);
    connect(ui->timeZoneDetailsCheckBox, &QCheckBox::clicked, this, &SurfaceInput::timeZoneDetailsCheckBoxClicked);

    connect(this, &SurfaceInput::updateProgressMessageSignal, this, &SurfaceInput::updateProgressMessage, Qt::QueuedConnection);

    connect(this, &SurfaceInput::updateState, &AppState::instance(), &AppState::updateSurfaceInputState);
}

void SurfaceInput::meshResolutionUnitsComboBoxCurrentIndexChanged(int index)
{
    if(index == 0) // meters
    {
        ui->meshResolutionSpinBox->setValue(ui->meshResolutionSpinBox->value() * 0.3048);
    }
    else  // if(index == 1) // feet
    {
        ui->meshResolutionSpinBox->setValue(ui->meshResolutionSpinBox->value() * 3.28084);
    }
}

void SurfaceInput::elevationInputTypePushButtonClicked()
{
    if(ui->elevationInputTypePushButton->isChecked())
    {
        webEngineView->page()->runJavaScript("startRectangleDrawing();");
    }
    else
    {
        webEngineView->page()->runJavaScript("stopRectangleDrawing();");
    }
}

void SurfaceInput::boundingBoxReceived(double north, double south, double east, double west)
{
    ui->boundingBoxNorthLineEdit->blockSignals(true);
    ui->boundingBoxEastLineEdit->blockSignals(true);
    ui->boundingBoxSouthLineEdit->blockSignals(true);
    ui->boundingBoxWestLineEdit->blockSignals(true);

    ui->boundingBoxNorthLineEdit->setText(QString::number(north));
    ui->boundingBoxEastLineEdit->setText(QString::number(east));
    ui->boundingBoxSouthLineEdit->setText(QString::number(south));
    ui->boundingBoxWestLineEdit->setText(QString::number(west));

    ui->boundingBoxNorthLineEdit->blockSignals(false);
    ui->boundingBoxEastLineEdit->blockSignals(false);
    ui->boundingBoxSouthLineEdit->blockSignals(false);
    ui->boundingBoxWestLineEdit->blockSignals(false);

    double pointRadius[3];
    computePointRadius(north, east, south, west, pointRadius);

    ui->pointRadiusLatLineEdit->blockSignals(true);
    ui->pointRadiusLonLineEdit->blockSignals(true);
    ui->pointRadiusRadiusLineEdit->blockSignals(true);

    ui->pointRadiusLatLineEdit->setText(QString::number(pointRadius[0]));
    ui->pointRadiusLonLineEdit->setText(QString::number(pointRadius[1]));
    ui->pointRadiusRadiusLineEdit->setText(QString::number(pointRadius[2]));

    ui->pointRadiusLatLineEdit->blockSignals(false);
    ui->pointRadiusLonLineEdit->blockSignals(false);
    ui->pointRadiusRadiusLineEdit->blockSignals(false);

    ui->elevationInputTypePushButton->setChecked(false);
}


void SurfaceInput::boundingBoxLineEditsTextChanged()
{
    if(ui->elevationInputTypeComboBox->currentIndex() == 0)
    {
        bool isNorthValid, isEastValid, isSouthValid, isWestValid;
        double north = ui->boundingBoxNorthLineEdit->text().toDouble(&isNorthValid);
        double east  = ui->boundingBoxEastLineEdit->text().toDouble(&isEastValid);
        double south = ui->boundingBoxSouthLineEdit->text().toDouble(&isSouthValid);
        double west  = ui->boundingBoxWestLineEdit->text().toDouble(&isWestValid);

        if (isNorthValid && isEastValid && isSouthValid && isWestValid)
        {
            QString js = QString("drawBoundingBox(%1, %2, %3, %4);")
            .arg(north, 0, 'f', 10)
                .arg(south, 0, 'f', 10)
                .arg(east,  0, 'f', 10)
                .arg(west,  0, 'f', 10);
            webEngineView->page()->runJavaScript(js);
        }
    }
}

void SurfaceInput::pointRadiusLineEditsTextChanged()
{
    if (ui->elevationInputTypeComboBox->currentIndex() == 1)
    {
        bool isLatValid, isLonValid, isRadiusValid;
        double lat = ui->pointRadiusLatLineEdit->text().toDouble(&isLatValid);
        double lon = ui->pointRadiusLonLineEdit->text().toDouble(&isLonValid);
        double radius = ui->pointRadiusRadiusLineEdit->text().toDouble(&isRadiusValid);
        double boundingBox[4];

        if(isLatValid && isLonValid && isRadiusValid)
        {
            computeBoundingBox(lat, lon, radius, boundingBox);
            QString js = QString("drawBoundingBox(%1, %2, %3, %4);")
                             .arg(boundingBox[0], 0, 'f', 10)
                             .arg(boundingBox[2], 0, 'f', 10)
                             .arg(boundingBox[1],  0, 'f', 10)
                             .arg(boundingBox[3],  0, 'f', 10);
            webEngineView->page()->runJavaScript(js);
        }
    }
}

void SurfaceInput::surfaceInputDownloadCancelButtonClicked()
{
    ui->inputsStackedWidget->setCurrentIndex(3);

    ui->elevationInputTypeComboBox->setCurrentIndex(0);
    ui->elevationFileTypeComboBox->setCurrentIndex(0);
    ui->elevationInputTypePushButton->setChecked(false);

    ui->boundingBoxNorthLineEdit->clear();
    ui->boundingBoxEastLineEdit->clear();
    ui->boundingBoxSouthLineEdit->clear();
    ui->boundingBoxWestLineEdit->clear();

    ui->pointRadiusLatLineEdit->clear();
    ui->pointRadiusLonLineEdit->clear();
    ui->pointRadiusRadiusLineEdit->clear();

    webEngineView->page()->runJavaScript("stopRectangleDrawing();");

    if(!ui->elevationInputFileLineEdit->property("fullpath").toString().isEmpty())
    {
        QStringList cornerStrs;
        for (int i = 0; i < 8; ++i)
        {
            cornerStrs << QString::number(DEMCorners[i], 'f', 8);
        }
        QString js = QString("drawDEM([%1]);").arg(cornerStrs.join(", "));
        webEngineView->page()->runJavaScript(js);
    }
}

void SurfaceInput::surfaceInputDownloadButtonClicked()
{
    QVector<double> boundingBox = {
        ui->boundingBoxNorthLineEdit->text().toDouble(),
        ui->boundingBoxEastLineEdit->text().toDouble(),
        ui->boundingBoxSouthLineEdit->text().toDouble(),
        ui->boundingBoxWestLineEdit->text().toDouble()
    };

    double resolution = 30;

    QString defaultName = "";
    QString downloadsPath = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
    QDir dir(downloadsPath);
    QString fullPath = dir.filePath(defaultName);
    QString demFilePath = QFileDialog::getSaveFileName(ui->centralwidget, "Save DEM File", fullPath, "TIF Files (*.tif)");
    if (demFilePath.isEmpty()) {
        return;
    }
    if (!demFilePath.endsWith(".tif", Qt::CaseInsensitive)) {
        demFilePath += ".tif";
    }
    ui->elevationInputFileLineEdit->setProperty("fullpath", demFilePath);
    std::string demFile = demFilePath.toStdString();

    std::string fetchType;
    switch(ui->elevationFileTypeComboBox->currentIndex())
    {
    case 0:
        fetchType = "srtm";
        break;
    case 1:
        fetchType = "gmted";
        break;
    case 2:
        fetchType = "lcp";
        break;
    }

    startFetchDEM(boundingBox, demFile, resolution, fetchType);
}

void SurfaceInput::elevationInputFileDownloadButtonClicked()
{
    ui->inputsStackedWidget->setCurrentIndex(16);
}

void SurfaceInput::meshResolutionComboBoxCurrentIndexChanged(int index)
{
    if (index == 3)
    {
        ui->meshResolutionSpinBox->setEnabled(true);
    }
    else
    {
        ui->meshResolutionSpinBox->setEnabled(false);
    }

    ui->meshResolutionSpinBox->setValue(computeMeshResolution(ui->meshResolutionComboBox->currentIndex(), ui->momentumSolverCheckBox->isChecked()));
    updateMeshResolutionByUnits();
}

void SurfaceInput::elevationInputFileLineEditTextChanged(const QString &demFilePath)
{
    QFileInfo file(demFilePath);
    ui->outputDirectoryLineEdit->setText(file.absolutePath());

    computeDEMFile(demFilePath);
    if(demFileType == "LCP")
    {
        ui->vegetationStackedWidget->setCurrentIndex(1);
    }
    else
    {
        ui->vegetationStackedWidget->setCurrentIndex(0);
    }

    ui->meshResolutionSpinBox->setValue(computeMeshResolution(ui->meshResolutionComboBox->currentIndex(), ui->momentumSolverCheckBox->isChecked()));
    updateMeshResolutionByUnits();

    QStringList cornerStrs;
    for (int i = 0; i < 8; ++i)
        cornerStrs << QString::number(DEMCorners[i], 'f', 8);
    QString js = QString("drawDEM([%1]);").arg(cornerStrs.join(", "));
    webEngineView->page()->runJavaScript(js);

    ui->elevationInputFileLineEdit->setProperty("fullpath", demFilePath);
    QSignalBlocker blocker(ui->elevationInputFileLineEdit);
    ui->elevationInputFileLineEdit->setText(QFileInfo(demFilePath).fileName());
    ui->elevationInputFileLineEdit->setToolTip(demFilePath);

    emit updateState();
    emit updateTreeView();
}

void SurfaceInput::elevationInputFileOpenButtonClicked()
{
    QString directoryPath;
    if(!ui->elevationInputFileLineEdit->property("fullpath").toString().isEmpty())
    {
        directoryPath = ui->elevationInputFileLineEdit->property("fullpath").toString();
    }
    else
    {
        directoryPath = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
    }

    QString demFilePath = QFileDialog::getOpenFileName(ui->centralwidget, "Select a file", directoryPath, "(*.tif);;All Files (*)");

    if (demFilePath.isEmpty())
    {
        if (!ui->elevationInputFileLineEdit->property("fullpath").toString().isEmpty())
        {
            ui->elevationInputFileLineEdit->setText(ui->elevationInputFileLineEdit->property("fullpath").toString());
            ui->elevationInputFileLineEdit->setToolTip(ui->elevationInputFileLineEdit->property("fullpath").toString());
        }
        return;
    }

    ui->elevationInputFileLineEdit->setText(demFilePath);
    ui->elevationInputFileLineEdit->setToolTip(demFilePath);
}

void SurfaceInput::startFetchDEM(QVector<double> boundingBox, std::string demFile, double resolution, std::string fetchType)
{
    emit writeToConsoleSignal("Fetching DEM file...");

    progress = new QProgressDialog("Fetching DEM file...", QString(), 0, 0, ui->centralwidget);
    progress->setWindowModality(Qt::WindowModal);
    progress->setCancelButton(nullptr);
    progress->setMinimumDuration(0);
    progress->setAutoClose(true);
    progress->show();

    futureWatcher = new QFutureWatcher<int>(this);
    QFuture<int> future = QtConcurrent::run(&SurfaceInput::fetchDEMFile, this, boundingBox, demFile, resolution, fetchType);
    futureWatcher->setFuture(future);

    connect(futureWatcher, &QFutureWatcher<int>::finished, this, &SurfaceInput::fetchDEMFinished);
}

void SurfaceInput::fetchDEMFinished()
{
    // get the return value of the QtConcurrent::run() function
    int result = futureWatcher->future().result();

    if(result == NINJA_SUCCESS)
    {
        emit writeToConsoleSignal("Finished fetching DEM file.", Qt::darkGreen);

        if (progress)
        {
            progress->close();
            progress->deleteLater();
            progress = nullptr;
        }

        ui->elevationInputFileLineEdit->setText(ui->elevationInputFileLineEdit->property("fullpath").toString());
        ui->inputsStackedWidget->setCurrentIndex(3);

    } else
    {
        emit writeToConsoleSignal("Failed to fetch DEM file.");
    }

    // delete the futureWatcher every time, whether success or failure
    if (futureWatcher)
    {
        futureWatcher->deleteLater();
        futureWatcher = nullptr;
    }
}

void SurfaceInput::timeZoneComboBoxCurrentIndexChanged(int index)
{
    QString currentTimeZone = ui->timeZoneComboBox->currentText();
    QString timeZoneDetails = fetchTimeZoneDetails(currentTimeZone);
    ui->timeZoneDetailsTextEdit->setText(timeZoneDetails);
}

void SurfaceInput::timeZoneAllZonesCheckBoxClicked()
{
    AppState& state = AppState::instance();

    bool isShowAllTimeZonesSelected = ui->timeZoneAllZonesCheckBox->isChecked();
    QVector<QVector<QString>> displayData = fetchAllTimeZones(isShowAllTimeZonesSelected);

    ui->timeZoneComboBox->clear();
    for (const QVector<QString>& zone : displayData)
    {
        if (!zone.isEmpty())
        {
            ui->timeZoneComboBox->addItem(zone[0]);
        }
    }

    // Default to America/Denver
    ui->timeZoneComboBox->setCurrentText("America/Denver");
}

void SurfaceInput::timeZoneDetailsCheckBoxClicked()
{
    ui->timeZoneDetailsTextEdit->setVisible(ui->timeZoneDetailsCheckBox->isChecked());
}

QString SurfaceInput::fetchTimeZoneDetails(QString currentTimeZone)
{
    QVector<QString> matchedRow;
    QFile file(":/date_time_zonespec.csv");

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qWarning() << "Failed to open date_time_zonespec.csv";
        qDebug() << "No data found";
    }

    QTextStream in(&file);
    bool firstLine = true;

    while (!in.atEnd())
    {
        QString line = in.readLine();

        if (firstLine)
        {
            firstLine = false;
            continue;  // skip header
        }

        QStringList tokens = line.split(",", Qt::KeepEmptyParts);
        QVector<QString> row;

        for (const QString& token : tokens)
            row.append(token.trimmed().remove("\""));

        QString fullZone = row.mid(0, 1).join("/");

        if (fullZone == currentTimeZone)
        {
            matchedRow = row;
            break;
        }
    }

    file.close();

    if (matchedRow.isEmpty())
    {
        qDebug() << "No matching time zone found.";
    }

    QString standardName = matchedRow.value(2);
    QString daylightName = matchedRow.value(4);
    QString stdOffsetStr = matchedRow.value(5);
    QString dstAdjustStr = matchedRow.value(6);

    auto timeToSeconds = [](const QString& t) -> int
    {
        QString s = t.trimmed();
        bool negative = s.startsWith('-');
        s = s.remove(QChar('+')).remove(QChar('-'));

        QStringList parts = s.split(':');
        if (parts.size() != 3) return 0;

        int h = parts[0].toInt();
        int m = parts[1].toInt();
        int sec = parts[2].toInt();

        int total = h * 3600 + m * 60 + sec;
        return negative ? -total : total;
    };

    // Convert total seconds back to HH:MM:SS with sign
    auto secondsToTime = [](int totalSec) -> QString
    {
        QChar sign = totalSec < 0 ? '-' : '+';
        totalSec = std::abs(totalSec);

        int h = totalSec / 3600;
        int m = (totalSec % 3600) / 60;
        int s = totalSec % 60;

        return QString("%1%2:%3:%4")
            .arg(sign)
            .arg(h, 2, 10, QChar('0'))
            .arg(m, 2, 10, QChar('0'))
            .arg(s, 2, 10, QChar('0'));
    };

    int stdSecs = timeToSeconds(stdOffsetStr);
    int dstSecs = timeToSeconds(dstAdjustStr);
    QString combinedOffsetStr = secondsToTime(stdSecs + dstSecs);

    return QString("Standard Name:\t\t%1\n"
                   "Daylight Name:\t\t%2\n"
                   "Standard Offset from UTC:\t%3\n"
                   "Daylight Offset from UTC:\t%4")
        .arg(standardName)
        .arg(daylightName)
        .arg(stdOffsetStr)
        .arg(combinedOffsetStr);

}

QVector<QVector<QString>> SurfaceInput::fetchAllTimeZones(bool isShowAllTimeZonesSelected)
{
    QVector<QVector<QString>> fullData;
    QVector<QVector<QString>> americaData;

    QFile file(":/date_time_zonespec.csv");

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug() << "Failed to open CSV file.";
        return fullData;
    }

    QTextStream in(&file);
    bool firstLine = true;

    while (!in.atEnd())
    {
        QString line = in.readLine();

        if (firstLine)
        {
            firstLine = false;
            continue;
        }

        QStringList tokens = line.split(",", Qt::KeepEmptyParts);
        QVector<QString> row;
        for (const QString& token : tokens)
        {
            row.append(token.trimmed().remove('"'));
        }

        if (!row.isEmpty())
        {
            fullData.append(row);
        }

        if (!row.isEmpty())
        {
            QStringList parts = row[0].split("/", Qt::KeepEmptyParts);
            if (!parts.isEmpty() && parts[0] == "America" || row[0] == "Pacific/Honolulu")
            {
                americaData.append(row);
            }
        }
    }

    file.close();

    if (isShowAllTimeZonesSelected)
    {
        return fullData;
    }
    else
    {
        return americaData;
    }
}

void SurfaceInput::updateProgressMessage(const QString message)
{
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
    SurfaceInput *self = static_cast<SurfaceInput*>(pUser);

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
            emit self->updateProgressMessageSignal(QString::fromStdString("SurfaceFetch ended with unknown error"));
            emit self->writeToConsoleSignal(QString::fromStdString("unknown SurfaceFetch error"), Qt::red);
        }
        else
        {
            emit self->updateProgressMessageSignal(QString::fromStdString("SurfaceFetch ended in error:\n"+clipStr));
            emit self->writeToConsoleSignal(QString::fromStdString("SurfaceFetch error: "+clipStr), Qt::red);
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
        emit self->updateProgressMessageSignal(QString::fromStdString("SurfaceFetch ended in warning:\n"+clipStr));
        emit self->writeToConsoleSignal(QString::fromStdString("SurfaceFetch warning: "+clipStr), Qt::yellow);
    }
    else
    {
        emit self->updateProgressMessageSignal(QString::fromStdString(msg));
        emit self->writeToConsoleSignal(QString::fromStdString(msg));
    }
}

int SurfaceInput::fetchDEMFile(QVector<double> boundingBox, std::string demFile, double resolution, std::string fetchType)
{
    NinjaToolsH* ninjaTools = NULL;
    char ** papszOptions = NULL;
    NinjaErr ninjaErr = 0;

    ninjaTools = NinjaMakeTools();

    ninjaErr = NinjaSetToolsComMessageHandler(ninjaTools, &comMessageHandler, this, papszOptions);
    if(ninjaErr != NINJA_SUCCESS)
    {
        qDebug() << "NinjaSetToolsComMessageHandler(): ninjaErr =" << ninjaErr;
    }

    ninjaErr = NinjaFetchDEMBBox(ninjaTools, boundingBox.data(), demFile.c_str(), resolution, strdup(fetchType.c_str()), papszOptions);  // some errors and warnings are caught, but only as error codes, not as messages. Would need to update how we do the messaging within the various SurfaceFetch calls themselves. CPLError( CE_Warning, ...); and CPLError( CE_Failure, ...); with return of an error code seems hard to try/catch with ninjaCom.
    //ninjaErr = NinjaFetchDEMBBox(ninjaTools, boundingBox.data(), ".", resolution, strdup(fetchType.c_str()), papszOptions);  // error was caught, but message is not properly passed
    //ninjaErr = NinjaFetchDEMBBox(ninjaTools, boundingBox.data(), demFile.c_str(), resolution, "fudge", papszOptions);  // actually catches this error, with a good message
    //ninjaErr = NinjaFetchDEMBBox(ninjaTools, boundingBox.data(), demFile.c_str(), -10.0, strdup(fetchType.c_str()), papszOptions);  // error was caught, but not even a message to be passed around with this one
    if (ninjaErr != NINJA_SUCCESS)
    {
        qDebug() << "NinjaFetchDEMBBox: ninjaErr =" << ninjaErr;
        return ninjaErr;
    }

    return NINJA_SUCCESS;
}

void SurfaceInput::computeDEMFile(QString filePath)
{
    double adfGeoTransform[6];
    GDALDataset *poInputDS;
    poInputDS = (GDALDataset*)GDALOpen(filePath.toStdString().c_str(), GA_ReadOnly);

    QString GDALDriverName = poInputDS->GetDriver()->GetDescription();
    if(GDALDriverName == "AAIGrid")
    {
        demFileType = "ASC";
    }
    else if (GDALDriverName == "LCP")
    {
        demFileType = "LCP";
    }
    else if (GDALDriverName == "GTiff")
    {
        int bandCount = GDALGetRasterCount(poInputDS);
        if(bandCount >1)
        {
            demFileType = "LCP";
        }
        else
        {
            demFileType = "GTIFF";
        }
    }
    else if (GDALDriverName == "IMG")
    {
        demFileType = "IMG";
    }

    GDALXSize = poInputDS->GetRasterXSize();
    GDALYSize = poInputDS->GetRasterYSize();
    GDALGetCorners(poInputDS, DEMCorners);

    double latitude, longitude;
    GDALGetCenter(poInputDS, &longitude, &latitude);
    std::string timeZone = FetchTimeZone(longitude, latitude, NULL);
    int index = ui->timeZoneComboBox->findText(QString::fromStdString(timeZone));
    if (index >= 0)
    {
        ui->timeZoneComboBox->setCurrentIndex(index);
    }

    if (poInputDS->GetGeoTransform(adfGeoTransform) == CE_None)
    {
        double c1, c2;
        c1 = adfGeoTransform[1];
        c2 = adfGeoTransform[5];
        if (abs(c1) == abs(c2)) {
            GDALCellSize = abs(c1);
        } else {
            GDALClose((GDALDatasetH)poInputDS);
        }
    }

    GDALRasterBand* band = poInputDS->GetRasterBand(1);
    int gotMin = 0, gotMax = 0;
    double minVal = band->GetMinimum(&gotMin);
    double maxVal = band->GetMaximum(&gotMax);
    if (!gotMin || !gotMax) {
        band->ComputeStatistics(false, &minVal, &maxVal, nullptr, nullptr, nullptr, nullptr);
    }

    GDALMinValue = minVal;
    GDALMaxValue = maxVal;

    GDALClose((GDALDatasetH)poInputDS);
}

double SurfaceInput::computeMeshResolution(int index, bool isMomemtumChecked)
{
    int coarse = 4000;
    int medium = 10000;
    int fine = 20000;
    double meshResolution = 200.0;

    if( GDALCellSize == 0.0 || GDALXSize == 0 || GDALYSize == 0 )
    {
        return meshResolution;
    }

#ifdef NINJAFOAM
    if (isMomemtumChecked) {
        coarse = 25000;
        medium = 50000;
        fine = 100000;
    }
#endif //NINJAFOAM

    int targetNumHorizCells = fine;
    switch (index)
    {
    case 0:
        targetNumHorizCells = coarse;
        break;
    case 1:
        targetNumHorizCells = medium;
        break;
    case 2:
        targetNumHorizCells = fine;
        break;
    case 3:  // custom
        return ui->meshResolutionSpinBox->value();
    default:
        return ui->meshResolutionSpinBox->value();
    }

    double XLength = GDALXSize * GDALCellSize;
    double YLength = GDALYSize * GDALCellSize;
    double nXcells = 2 * std::sqrt((double)targetNumHorizCells) * (XLength / (XLength + YLength));
    double nYcells = 2 * std::sqrt((double)targetNumHorizCells) * (YLength / (XLength + YLength));

    double XCellSize = XLength / nXcells;
    double YCellSize = YLength / nYcells;

    meshResolution = (XCellSize + YCellSize) / 2;

#ifdef NINJAFOAM
    if (isMomemtumChecked)
    {
        XLength = GDALXSize * GDALCellSize;
        YLength = GDALYSize * GDALCellSize;

        double dz = GDALMaxValue - GDALMinValue;
        double ZLength = std::max((0.1 * std::max(XLength, YLength)), (dz + 0.1 * dz));
        double zmin, zmax;
        zmin = GDALMaxValue + 0.05 * ZLength; //zmin (above highest point in DEM for MDM)
        zmax = GDALMaxValue + ZLength; //zmax

        double volume;
        double cellCount;
        double cellVolume;

        volume = XLength * YLength * (zmax-zmin); //volume of blockMesh
        cellCount = targetNumHorizCells * 0.5; // cell count in volume 1
        cellVolume = volume/cellCount; // volume of 1 cell in blockMesh
        double side = std::pow(cellVolume, (1.0/3.0)); // length of side of cell in blockMesh

        //determine number of rounds of refinement
        int nCellsToAdd = 0;
        int refinedCellCount = 0;
        int nCellsInLowestLayer = int(XLength/side) * int(YLength/side);
        int nRoundsRefinement = 0;
        while(refinedCellCount < (0.5 * targetNumHorizCells))
        {
            nCellsToAdd = nCellsInLowestLayer * 8; //each cell is divided into 8 cells
            refinedCellCount += nCellsToAdd - nCellsInLowestLayer; //subtract the parent cells
            nCellsInLowestLayer = nCellsToAdd/2; //only half of the added cells are in the lowest layer
            nRoundsRefinement += 1;
        }

        meshResolution = side/(nRoundsRefinement*2.0);
    }
#endif //NINJAFOAM

    return meshResolution;
}

void SurfaceInput::updateMeshResolutionByUnits()
{
    // units are expected to always already be calculated to be in meters
    // UNLESS it is CUSTOM, then it needs to be left in whatever units it shows up in,
    //  units are then only altered when updating the units combo box
    if(ui->meshResolutionComboBox->currentIndex() != 3)
    {
        if(ui->meshResolutionUnitsComboBox->currentIndex() == 1)  // feet
        {
            ui->meshResolutionSpinBox->setValue(ui->meshResolutionSpinBox->value() * 3.28084);
        }
    }
}

void SurfaceInput::computeBoundingBox(double centerLat, double centerLon, double radius, double boundingBox[4])
{
    const double EARTH_RADIUS_MILES = 3958.7613;
    const double DEG_TO_RAD = M_PI / 180.0;

    double deltaLat = radius / (2.0 * M_PI * EARTH_RADIUS_MILES / 360.0);
    double latRadius = EARTH_RADIUS_MILES * std::cos(centerLat * DEG_TO_RAD);
    double deltaLon = radius / (2.0 * M_PI * latRadius / 360.0);

    boundingBox[0] = centerLat + deltaLat;  // North
    boundingBox[1] = centerLon + deltaLon;  // East
    boundingBox[2] = centerLat - deltaLat;  // South
    boundingBox[3] = centerLon - deltaLon;  // West
}

void SurfaceInput::computePointRadius(double north, double east, double south, double west, double pointRadius[3])
{
    const double EARTH_RADIUS_MILES = 3958.7613;
    const double DEG_TO_RAD = M_PI / 180.0;

    double centerLat = (north + south) / 2.0;
    double centerLon = (east + west) / 2.0;

    double deltaLat = std::abs(north - south) / 2.0;
    double deltaLon = std::abs(east - west) / 2.0;

    double latMiles = (2.0 * M_PI * EARTH_RADIUS_MILES / 360.0) * deltaLat;

    double latRadius = EARTH_RADIUS_MILES * std::cos(centerLat * DEG_TO_RAD);
    double lonMiles = (2.0 * M_PI * latRadius / 360.0) * deltaLon;

    double radius = (latMiles + lonMiles) / 2.0;

    pointRadius[0] = centerLat;
    pointRadius[1] = centerLon;
    pointRadius[2] = radius;
}

