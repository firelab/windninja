#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <QObject>
#include <vector>
#include "appstate.h"
#include "mainwindow.h"
#include "modeldata.h"
#include "provider.h"

class Controller : public QObject {
    Q_OBJECT
public:
    Controller(MainWindow* view, QObject* parent = nullptr);

private:
  MainWindow* view;
  Provider provider;
  BaseInput setBaseInput();
  DomainAverageWind setDomainAverageWind();

  void onSolveRequest();
  void onTimeZoneDataRequest();
  void onTimeZoneDetailsRequest();
};

#endif // CONTROLLER_H
