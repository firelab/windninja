/******************************************************************************
 *
 * $Id$
 * 
 * Project:  WindNinja Qt GUI
 * Purpose:  Handle lat/lon dd:mm:ss/dd:mm.mm/dd.dd conversions for diurnal
 *           inputs
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

#ifndef LATLONWIDGET_H
#define LATLONWIDGET_H

#include <QtWidgets/QLabel>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QComboBox>

#include <math.h>

class dmsWidget : public QWidget
{
    Q_OBJECT
    
public:
    dmsWidget(QWidget *parent = 0);
    QGroupBox *latGroupBox, *lonGroupBox;
    QSpinBox *latDegrees, *lonDegrees;
    QSpinBox *latMinutes, *lonMinutes;

    QDoubleSpinBox *latSeconds, *lonSeconds;

    QHBoxLayout *latLayout, *lonLayout, *layout;

};

class dmWidget : public QWidget
{
    Q_OBJECT
public:
    dmWidget(QWidget *parent = 0);
    QGroupBox *latGroupBox, *lonGroupBox;
    QSpinBox *latDegrees, *lonDegrees;
    QDoubleSpinBox *latMinutes, *lonMinutes;

    QHBoxLayout *latLayout, *lonLayout, *layout;
};

class ddWidget : public QWidget
{
    Q_OBJECT

public:
    ddWidget(QWidget *parent = 0);
    QGroupBox *latGroupBox, *lonGroupBox;
    QDoubleSpinBox *latDegrees, *lonDegrees;

    QHBoxLayout *latLayout, *lonLayout, *layout;

};

class latLonWidget : public QWidget
{
    Q_OBJECT
public:
    latLonWidget(QString title, QWidget *parent = 0);
    
    //store lat lon in dd always
    double latitude, longitude;
    
    QGroupBox *latLonGroupBox;
    QGroupBox *latLonFormatGroupBox;
    QLabel *latLonFormatLabel;
    QRadioButton *dmsRadio, *dmRadio, *ddRadio;

    //conversions
    double dms2ddLatitude(int, int, double);
    double dms2ddLongitude(int, int, double);
    double dm2ddLatitude(int, double);
    double dm2ddLongitude(int, double);
    
    QStackedWidget *stackedWidget;
    
    dmsWidget *dms;
    dmWidget *dm;
    ddWidget *dd;

    QComboBox *datumComboBox;
    
    QHBoxLayout *latLonFormatLayout;
    QVBoxLayout *latLonLayout;
    
    QVBoxLayout *layout;
    
public slots:
    void checkLatLonFormat();
    void updateLatLon();
    void updateLatLonWidget(int notMe);
    
};

#endif /* LATLONWIDGET_H */
