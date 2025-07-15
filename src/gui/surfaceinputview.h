#ifndef SURFACEINPUTVIEW_H
#define SURFACEINPUTVIEW_H

#include "surfaceinput.h"
#include <QtWebEngineWidgets/qwebengineview.h>
#include <QWebEngineProfile>
#include <QWebEngineSettings>
#include <QObject>
#include <QStandardPaths>
#include <QFileDialog>
#include <QDebug>
#include <QProgressDialog>
#include <QFuture>
#include <QFutureWatcher>
#include <QProgressDialog>
#include <QtConcurrent/QtConcurrent>

namespace Ui {
class MainWindow;
}

class SurfaceInputView : public QObject {
  Q_OBJECT
public:
  explicit SurfaceInputView(Ui::MainWindow* ui,
                            QWebEngineView* webView,
                            SurfaceInput* surfaceInput,
                            QObject* parent = nullptr);

signals:
  void requestRefresh();

public slots:
  void boundingBoxReceived(double north, double south, double east, double west);

private slots:
  void surfaceInputDownloadCancelButtonClicked();
  void surfaceInputDownloadButtonClicked();
  void meshResolutionUnitsComboBoxCurrentIndexChanged(int index);
  void elevationInputTypePushButtonClicked();
  void boundingBoxLineEditsTextChanged();
  void pointRadiusLineEditsTextChanged();
  void elevationInputFileDownloadButtonClicked();
  void elevationInputFileOpenButtonClicked();
  void elevationInputFileLineEditTextChanged(const QString &arg1);
  void meshResolutionComboBoxCurrentIndexChanged(int index);

private:
  Ui::MainWindow *ui;
  QWebEngineView *webView;
  SurfaceInput *surfaceInput;

  QFutureWatcher<int> futureWatcher;

  QString currentDemFilePath;
};

#endif // SURFACEINPUTVIEW_H
