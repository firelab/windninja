#******************************************************************************
#*
#* $Id$
#*
#* Project:  WindNinja
#* Purpose:  Testing script
#* Author:   Kyle Shannon <kyle@pobox.com>
#*
#******************************************************************************
#*
#* THIS SOFTWARE WAS DEVELOPED AT THE ROCKY MOUNTAIN RESEARCH STATION (RMRS)
#* MISSOULA FIRE SCIENCES LABORATORY BY EMPLOYEES OF THE FEDERAL GOVERNMENT 
#* IN THE COURSE OF THEIR OFFICIAL DUTIES. PURSUANT TO TITLE 17 SECTION 105 
#* OF THE UNITED STATES CODE, THIS SOFTWARE IS NOT SUBJECT TO COPYRIGHT 
#* PROTECTION AND IS IN THE PUBLIC DOMAIN. RMRS MISSOULA FIRE SCIENCES 
#* LABORATORY ASSUMES NO RESPONSIBILITY WHATSOEVER FOR ITS USE BY OTHER 
#* PARTIES,  AND MAKES NO GUARANTEES, EXPRESSED OR IMPLIED, ABOUT ITS QUALITY, 
#* RELIABILITY, OR ANY OTHER CHARACTERISTIC.
#*
#* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
#* OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
#* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
#* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
#* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
#* FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
#* DEALINGS IN THE SOFTWARE.
#*
#*****************************************************************************/

echo $PWD ;
CFG_NAME="cli_pointInitialization_diurnal" ;
../src/cli/WindNinja_cli cfg/${CFG_NAME}.cfg ;
mkdir -p output/new/${CFG_NAME} ;
mv surface_data/*.asc output/new/${CFG_NAME} ;
mv surface_data/*.prj output/new/${CFG_NAME} ;
mv surface_data/*.dbf output/new/${CFG_NAME} ;
mv surface_data/*.shp output/new/${CFG_NAME} ;
mv surface_data/*.shx output/new/${CFG_NAME} ;
mv surface_data/*.atm output/new/${CFG_NAME} ;
mv surface_data/*.kmz output/new/${CFG_NAME} ;
diff -rq output/original/${CFG_NAME} output/new/${CFG_NAME} ;
exit $?

