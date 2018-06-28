/******************************************************************************
 *
 * $Id$
 *
 * Project:  WindNinja Qt GUI
 * Purpose:  Google earth output selection widgetx
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

#ifndef GOOGLEOUTPUT_H
#define GOOGLEOUTPUT_H

#include <QGroupBox>
#include <QComboBox>
#include <QCheckBox>
#include <QRadioButton>
#include <QLabel>
#include <QDoubleSpinBox>

#include <QVBoxLayout>
#include <QHBoxLayout>

class googleOutput : public QWidget
{
  Q_OBJECT

 public:
  googleOutput(QWidget *parent = 0);
  
  QGroupBox *googleGroupBox;
  QGroupBox *vectorGroupBox;
  QLabel *vectorWidthLabel;
  QDoubleSpinBox *vectorWidthDoubleSpinBox;
  QCheckBox *contourCheckBox;
  QGroupBox *legendGroupBox;
  QRadioButton *uniformRangeRadioButton,* equalCountRadioButton;
  QGroupBox *googleResGroupBox;
  QDoubleSpinBox *googleResSpinBox;
  QRadioButton *googleMetersRadioButton, *googleFeetRadioButton;
  QCheckBox *useMeshResCheckBox;
  
  //alternative color Options
  QGroupBox *colorblindBox;
  QComboBox *inputColorblindComboBox;
  QCheckBox *applyVectorScaling;

  QGridLayout *colorLayout;

  QHBoxLayout *vectorLayout;
  QVBoxLayout *optionLayout;
  QVBoxLayout *legendOptionLayout;
  QGridLayout *resLayout;
  QVBoxLayout *pageLayout;
  QVBoxLayout *mainLayout;

  public slots:
  void setColorScheme(int choice);

};

#endif /* GOOGLEOUTPUT_H */
