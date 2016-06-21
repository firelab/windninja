/******************************************************************************
 *
 * Project:  WindNinja Qt GUI
 * Purpose:  PDF output selection widgetx
 * Author:   Kyle Shannon <kyle at pobox dot com>
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

#ifndef PDFOUTPUT_H
#define PDFOUTPUT_H

#include <QGroupBox>
#include <QCheckBox>
#include <QRadioButton>
#include <QLabel>
#include <QDoubleSpinBox>
#include <QComboBox>

#include <QVBoxLayout>
#include <QHBoxLayout>

class pdfOutput : public QWidget
{
Q_OBJECT

public:
    pdfOutput(QWidget *parent = 0);

    QGroupBox *pdfGroupBox;
    QGroupBox *vectorGroupBox;
    QLabel *vectorWidthLabel;
    QDoubleSpinBox *vectorWidthDoubleSpinBox;
    QGroupBox *pdfResGroupBox;
    QDoubleSpinBox *pdfResSpinBox;
    QRadioButton *pdfMetersRadioButton, *pdfFeetRadioButton;
    QCheckBox *useMeshResCheckBox;
    QLabel *backgroundLabel;
    QComboBox *backgroundComboBox;

    QLabel *sizeLabel;
    QComboBox *sizeComboBox;
    QRadioButton *portraitRadioButton;
    QRadioButton *landscapeRadioButton;

    QHBoxLayout *vectorLayout;
    QVBoxLayout *optionLayout;
    QGridLayout *resLayout;
    QHBoxLayout *backgroundLayout;
    QVBoxLayout *orientLayout;
    QHBoxLayout *sizeLayout;
    QVBoxLayout *pageLayout;
    QVBoxLayout *mainLayout;
};

#endif /* PDFOUTPUT_H */
