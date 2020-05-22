#ifndef NATIVESOLVERINPUT_H
#define NATIVESOLVERINPUT_H

#include <QtWidgets/QGroupBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QGridLayout>

#include <string>
#include <vector>

#include "gdal_priv.h"
#include "ogr_srs_api.h"

#ifndef Q_MOC_RUN
#include "boost/date_time/local_time/local_time.hpp"
#include "boost/date_time/posix_time/posix_time_types.hpp"
#endif

#include "latLonWidget.h"
#include "timeZoneWidget.h"

class nativeSolverInput : public QWidget
{
  Q_OBJECT

 public:
  
  nativeSolverInput(QWidget *parent = 0);
  QGroupBox *nativeSolverGroupBox;
  QVBoxLayout *nativeSolverLayout;
  QVBoxLayout *layout;
  QLabel *nativeSolverLabel;
  

};

#endif /* NATIVESOLVERINPUT_H */
