/******************************************************************************
 *
 * $Id$
 *
 * Project:  WindNinja
 * Purpose:  Meta output information
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

#ifndef OUTPUT_META_DATA_H
#define OUTPUT_META_DATA_H

#include <QtWidgets/QLabel>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QVBoxLayout>

#include "outputHeightWidget.h"

class outputMetaData : public QWidget
{
 Q_OBJECT

 public:
    outputMetaData( QWidget *parent = 0 );
    ~outputMetaData();

    outputHeightWidget *outputHeight; /**< Output wind Heigth Widget */
    
    QLabel *bufferLabel;	/**< Describe the buffer */
    QSpinBox *bufferSpinBox;	/**< Set buffer size in % */

    QLabel *outputSpeedUnitsLabel;
    QComboBox *outputSpeedUnitsCombo;

    QCheckBox *wxModelOutputCheckBox; /**< Write out the raw wx model data */
    
    QHBoxLayout *outputSpeedUnitsLayout;
    QHBoxLayout *bufferLayout;	/**< layout for buffer info */
    QVBoxLayout *layout;	/**< main layout */
};

#endif /* OUTPUT_META_DATA_H */
