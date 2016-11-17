#ifndef NINJAFOAMINPUT_H
#define NINJAFOAMINPUT_H

#include <QGroupBox>
#include <QComboBox>
#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QCheckBox>
#include <QHBoxLayout>
#include <QGridLayout>

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

#include "qdebug.h"

class ninjafoamInput : public QWidget
{
  Q_OBJECT

 public:
  
  ninjafoamInput(QWidget *parent = 0);
  QGroupBox *ninjafoamGroupBox;
  QVBoxLayout *ninjafoamLayout;
  QVBoxLayout *layout;
  QLabel *ninjafoamLabel;

};

#endif /* NINJAFOAMINPUT_H */
