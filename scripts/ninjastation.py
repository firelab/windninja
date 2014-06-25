#!/usr/bin/env python
#******************************************************************************
#
#  $Id$
#
#  Project:  WindNinja
#  Purpose:  Download and remove elements for a kml for station locations
#  Author:   Kyle Shannon <kyle@pobox.com>
#
#******************************************************************************
#
#  THIS SOFTWARE WAS DEVELOPED AT THE ROCKY MOUNTAIN RESEARCH STATION (RMRS)
#  MISSOULA FIRE SCIENCES LABORATORY BY EMPLOYEES OF THE FEDERAL GOVERNMENT
#  IN THE COURSE OF THEIR OFFICIAL DUTIES. PURSUANT TO TITLE 17 SECTION 105
#  OF THE UNITED STATES CODE, THIS SOFTWARE IS NOT SUBJECT TO COPYRIGHT
#  PROTECTION AND IS IN THE PUBLIC DOMAIN. RMRS MISSOULA FIRE SCIENCES
#  LABORATORY ASSUMES NO RESPONSIBILITY WHATSOEVER FOR ITS USE BY OTHER
#  PARTIES,  AND MAKES NO GUARANTEES, EXPRESSED OR IMPLIED, ABOUT ITS QUALITY,
#  RELIABILITY, OR ANY OTHER CHARACTERISTIC.
#
#  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
#  OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
#  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
#  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
#  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
#  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
#  DEALINGS IN THE SOFTWARE.
#
#******************************************************************************

import cgi
import logging
import os
import sys
import time
import urllib2
import zipfile
import re

def Usage():
    print('ninjastation.py [--accepted station_type] [--log-level level]')
    print('                [--include-inactive] [--out-file filename]'   )
    print('                [--input-file filename]'                      )
    sys.exit()

def fetch_station_list(args):
    args = sys.argv
    accepted = set(['NWS/FAA', 'RAWS'])
    log_level = logging.ERROR
    include_inactive = False
    outfile = 'ninja_stations.kmz'
    infile = 'mesowest_csv.tbl'
    icons = {'NWS/FAA' : 'http://maps.google.com/mapfiles/kml/pal4/icon49.png',
             'RAWS': 'http://maps.google.com/mapfiles/kml/pal4/icon57.png',
             'TMPRAWS': 'http://maps.google.com/mapfiles/kml/pal4/icon57.png',
             'NSRAWS': 'http://maps.google.com/mapfiles/kml/pal4/icon57.png',
             'NWS COOP': 'http://maps.google.com/mapfiles/kml/pal4/icon56.png',
             'APRSWXNET/CWOP': 'http://maps.google.com/mapfiles/kml/pal4/icon56.png',
             'ITD': 'http://maps.google.com/mapfiles/kml/pal4/icon56.png',
             'MARITIME': 'http://maps.google.com/mapfiles/kml/pal4/icon49.png',
             'CARB': 'http://maps.google.com/mapfiles/kml/pal4/icon56.png',
             'DRI': 'http://maps.google.com/mapfiles/kml/pal4/icon56.png'}
    reset_accepted = True
    log_levels = {'debug'   :logging.DEBUG,
                  'info'    :logging.INFO,
                  'warning' :logging.WARNING,
                  'error'   :logging.ERROR,
                  'critical':logging.CRITICAL}
    i = 1
    while i < len(args):
        arg = args[i]
        if arg == '--accepted':
            if reset_accepted:
                accepted = set()
                reset_accepted = False
            i += 1
            accepted.add(args[i])
        elif arg == '--log-level':
            i += 1
            try:
                log_level = log_levels[args[i].lower()]
            except KeyError:
                print('Invalid logging level, use one of: %s' %
                        str(log_levels.keys()))
                Usage()
        elif arg == '--include-inactive':
            include_inactive = True
        elif arg == '--out-file':
            i += 1
            outfile = args[i]
        elif arg == '--input-file':
            i += 1
            infile = args[i]
        elif arg == '--help':
            Usage()
        i += 1

    logging.basicConfig(level=log_level)
    skipped = dict()
    fin = open(infile)
    fout = zipfile.ZipFile(outfile, mode='w', compression=zipfile.ZIP_DEFLATED)
    kml = '<?xml version="1.0" encoding="UTF-8"?>\n' \
          '<kml xmlns="http://www.opengis.net/kml/2.2">\n' \
          '  <Document>\n'
    header = 1
    dot = re.compile('[A-Z]{2} *DOT')   # DOT (examples: DOT, MT DOT, IDDOT)
    wfo = re.compile('WFO') # Any WFO (this is NWS Weather Forecast Office)
    for line in fin:
        if header:
            header = 0
            continue
        line = line.strip().split(',')
        if not line:
            continue
        name = line[0]
        num = line[1]
        full_name = cgi.escape(line[2])
        state = line[3]
        country = line[4]
        try:
            lat = float(line[5])
        except:
            lat = -1
        try:
            lon = float(line[6])
        except:
            lon = -1
        try:
            elev = float(line[7])
        except:
            elev = -1
        st_type = line[9].strip()
        
        if dot.match(st_type) and 'STATE COOP' in accepted:
            icon = icons['NWS COOP']
            pass
        elif wfo.match(st_type) and 'NWS/FAA' in accepted:
            icon = icons['NWS/FAA']
            pass
        elif st_type not in accepted:
            logging.info('Skipping station: %s with type: %s' % (name, st_type))
            if st_type not in skipped.keys():
                skipped[st_type] = 1
            else:
                skipped[st_type] += 1
            continue
        else:
            icon = icons[st_type.upper()]
        
        active = line[10].upper() == 'ACTIVE'
        provider = line[12]
        wims = line[-1]
        if wims.endswith(';') and len(wims) > 1:
            wims = wims[:-1]
        else:
            wims = ''
        if not active and not include_inactive:
            logging.info('Skipping station: %s, INACTIVE' % name)
            if st_type not in set(skipped.keys()):
                skipped[st_type] = 1
            else:
                skipped[st_type] += 1
            continue
        roman = '<a href="http://raws.wrh.noaa.gov/cgi-bin/roman/meso_base.cgi?stn=%s&time=GMT">ROMAN Link</a>' % name
        nws = '<a href="http://www.wrh.noaa.gov/mesowest/getobext.php?sid=%s">NWS Link</a>' % name
        fam = '<a href="http://fam.nwcg.gov/fam-web/weatherfirecd/data/mt/wlstinv1!%s.txt">Station Catalog</a>' % wims
        fw9 = '<a href="http://fam.nwcg.gov/fam-web/weatherfirecd/data/mt/wx%s.fw9">Fire Weather File (FW9)</a>' % wims
        if icon.startswith('http'):
            icon = '<href>' + icon + '</href>'
        kml += '    <Placemark>\n' \
               '      <Style>\n' \
               '        <IconStyle>\n' \
               '          <Icon>\n' \
               '            %s\n' \
               '          </Icon>\n' \
               '        </IconStyle>\n' \
               '      </Style>\n' \
               '      <Point>\n' \
               '        <coordinates>%.9f,%.9f,0</coordinates>\n' \
               '      </Point>\n' \
               '      <description>\n' \
               '        <![CDATA[\n' \
               '          <b>%s</b><br/>\n' \
               '          <table border="1">\n' \
               '            <tr>\n' \
               '              <td>Type</td>\n' \
               '              <td>%s</td>\n' \
               '            </tr>\n' \
               '            <tr>\n' \
               '              <td>Identifier</td>\n' \
               '              <td>%s</td>\n' \
               '            </tr>\n' \
               '            <tr>\n' \
               '              <td>State</td>\n' \
               '              <td>%s</td>\n' \
               '            </tr>\n' \
               '            <tr>\n' \
               '              <td>Country</td>\n' \
               '              <td>%s</td>\n' \
               '            </tr>\n' \
               '            <tr>\n' \
               '              <td>Longitude</td>\n' \
               '              <td>%f</td>\n' \
               '            </tr>\n' \
               '            <tr>\n' \
               '              <td>Latitude</td>\n' \
               '              <td>%f</td>\n' \
               '            </tr>\n' \
               '            <tr>\n' \
               '              <td>Elevation</td>\n' \
               '              <td>%.1f</td>\n' \
               '            </tr>\n' \
               '            <tr>\n' \
               '              <td>Provider</td>\n' \
               '              <td>%s</td>\n' \
               '            </tr>\n' \
               '            <tr>\n' \
               '              <td>WIMS ID</td>\n' \
               '              <td>%s</td>\n' \
               '            </tr>\n' \
               '          </table>\n' \
               '          <table>\n' \
               '            <tr>\n' \
               '              <td>%s</td>\n' \
               '            </tr>\n' \
               '            <tr>\n' \
               '              <td>%s</td>\n' \
               '            </tr>\n' \
               '            <tr>\n' \
               '              <td>%s</td>\n' \
               '            </tr>\n' \
               '            <tr>\n' \
               '              <td>%s</td>\n' \
               '            </tr>\n' \
               '          </table>\n' \
               '        ]]>\n' \
               '      </description>\n' \
               '    </Placemark>\n' % (icon, lon, lat, full_name, st_type, 
                                       name, state, country, lon, lat, elev,
                                       provider, wims, roman, nws, fam, fw9)
    kml += '  </Document>\n'
    kml += '</kml>'
    fout.writestr('doc.kml', kml)
    fin.close()
    fout.close()

if __name__ == '__main__':
    fetch_station_list(sys.argv)
