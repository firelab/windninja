#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTreeWidgetItem>
#include <QtWebEngineWidgets/qwebengineview.h>
#include "ui_mainwindow.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  MainWindow(QWidget *parent = nullptr);
  ~MainWindow();

private:
  void onTreeItemClicked(QTreeWidgetItem *item, int column);


Ui::MainWindow *ui;
QWebEngineView *webView;
};
#endif // MAINWINDOW_H
