#******************************************************************************
#*
#* $Id$
#*
#* Project:  WindNinja
#* Purpose:  Perform WindNinja CLI testing
#* Author:   Levi Malott <lmnn3@mst.edu>
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

# ==============================================================================
# Imports
# ==============================================================================
import os
import os.path
import filecmp
import shutil
import subprocess
import re
import datetime
import sys
from optparse import OptionParser

# ==============================================================================
# Constants
# ==============================================================================
REGEX_VALID_EXTS  = '.*\.asc|.*\.prj|.*\.dbf|.*\.shp|.*\.shx|.*\.atm|.*\.kmz|.*\.kml'
CLI_PATH          = 'WindNinja_cli'
WINDNINJA_ENV_VAR = 'WINDNINJA_DATA'
OUTPUT_DIR        = 'output'

DATA_DIR     = os.environ.get( WINDNINJA_ENV_VAR )
NEW_DIR      = os.path.join( OUTPUT_DIR, 'new' )
ORIG_DIR     = os.path.join( OUTPUT_DIR, 'original' )

# ==============================================================================
# Functions
# ==============================================================================

#***************
def modification_date( filename ):
    '''
    ==============================================================================
    Function:
    Returns the modification (or creation) date of a file as a datetime object

        Arguments
        ---------
        filename - name of the file to check

        Returns
        -------
        dt - date and time the file was last modified
    ==============================================================================
    '''
    dt = os.path.getmtime( filename )
    return datetime.datetime.fromtimestamp( dt )
#***************

#***************
def test_cli_configuration( cfg_file ):
    '''
    ==============================================================================
    Function: test_cli_configuration
    Given a cfg_file, WindNinja_CLI is run with it as an argument. The output
    files are moved to a relative output directory where each file is compared
    with the original (valid) output files. A Boolean is returned indicating
    whether the new files match the original or not.
        Arguments
        ---------
        cfg_file - Location of the cfg_file to run from trunk/test_data

        Returns
        -------
        passed   - True if the output of WindNinja_CLI with cfg_file matches the original
    ==============================================================================
    '''

    passed        = False
    current_dir   = os.getcwd()                            #Save the current directory to place output files
    cfg_dir       = os.path.dirname(  cfg_file )           #Directory where configuration file resides
    cfg_name      = os.path.basename( cfg_file )           #Name + ext of configuration file
    cfg_basename  = os.path.splitext( cfg_name )[0]        #Name of configuration file
    #output locations
    output_dir    = os.path.join( current_dir, NEW_DIR, cfg_basename )
    original_dir  = os.path.join( current_dir, ORIG_DIR, cfg_basename )

    if( not os.path.exists( original_dir ) ):
        os.mkdir( original_dir )

    #Create the output directory if it does not exist
    if( not os.path.exists( output_dir ) ):
        os.mkdir( output_dir )
    #If it does exist, remove it and its contents, then re-create
    else:
        shutil.rmtree( output_dir )
        os.mkdir( output_dir )


    #Elevation file paths in configurations are relative to where the CLI
    #process is executed. For consistency, switching to the trunk folder
    #ensures those files will be found
    os.chdir( os.path.join( cfg_dir, '..' ) )
    cli_call_time = datetime.datetime.now()
    retval = subprocess.check_call( [ CLI_PATH, cfg_file ], shell=False )

    #Change to the configuration directory, as all output files will be placed there
    os.chdir( cfg_dir)

    subdirs = filter(os.path.isdir, os.listdir( os.path.curdir ) )
    subdirs = [ d for d in subdirs if modification_date( d ) >= cli_call_time ]

    if( not subdirs ):
        #Find all valid output files in the current directory, which is the
        #configuration file directory
        output_files = [ f for f in os.listdir( '.' ) \
                         if re.match( REGEX_VALID_EXTS, f ) ]



        #Only keep files that were created after the CLI command was executed
        valid_files = [ f for f in output_files \
                        if modification_date( f ) >= cli_call_time ]

    else:
        valid_files = subdirs

    #Move all of the CLI output files to the corresponding directory
    map( lambda x: shutil.move( x, output_dir ), valid_files )

    #Determine if any files differ from original output
    compare_results = filecmp.dircmp( output_dir, original_dir )
    #Return to the original calling directory
    os.chdir( current_dir )

    #Pass only if there are no different files and there are some files
    #with the same name
    if( len( compare_results.diff_files ) > 0 or
        len( compare_results.common ) == 0 ):
        passed = False
    else:
        passed = True

    return passed
#***************


if __name__ == '__main__':

    parser = OptionParser()
    parser.add_option( '-c', '--cfg-file', dest='cfg_file',
                       help='Name of the configuration file in WINDNINJA_DATA directory' )

    ( options, args ) = parser.parse_args()

    if( DATA_DIR is None ):
        print( '%s environment variable does not exist. Please create set it to'
               '/path/to/src/trunk/data' % WINDNINJA_ENV_VAR )
        sys.exit( 1 )

    cfg_file = os.path.join( DATA_DIR, options.cfg_file )

    if( os.path.isfile( cfg_file ) ):
        passed = test_cli_configuration( cfg_file )
        sys.exit( 0 ) if passed else exit( 1 )
    else:
        print '%s does not exist' % cfg_file
        sys.exit( 1 )
