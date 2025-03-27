/******************************************************************************
 *
 * $Id$
 *
 * Project:  WindNinja Qt GUI
 * Purpose:  Handles solve parameters and fires the solver
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

#ifndef SOLVEPAGE_H
#define SOLVEPAGE_H

#include <QFileDialog>
#include <QFileInfo>
#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>
#include <QToolButton>

#include <QCheckBox>
#include <QVBoxLayout>

#ifdef _OPENMP
#include <omp.h>
#endif


class solvePage : public QWidget
{
  Q_OBJECT

 public:
  solvePage(QWidget *parent = 0);

  int numProcessors;
  int getNumProcessors();

  QLabel *numProcLabel;
  QString availProcString;
  QLabel *availProcLabel;
  QSpinBox *numProcSpinBox;
  QCheckBox *CaseFileBox;

  QLabel *outputDirLabel;
  QLineEdit *outputDirLineEdit;
  QToolButton *outputDirToolButton;
  QToolButton *openOutputPathButton;

  QToolButton *solveToolButton;

  QHBoxLayout *pageLayout;
  QHBoxLayout *outputPathLayout;
  QVBoxLayout *layout;

  QString outputDirectory();

public slots:
  void setOutputDir(QString path);

private slots:
  void chooseOutputDir();
};

#endif /* SOLVEPAGE_H */
