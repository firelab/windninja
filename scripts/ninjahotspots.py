#!/usr/bin/env python
#******************************************************************************
#
#  $Id$
#
#  Project:  WindNinja
#  Purpose:  Download and remove elements for a kml for fire detections
#  Author:   Jason Forthofer <jaforthofer@fs.fed.us>
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

import sys
import logging
import pprint
import urllib2
from StringIO import StringIO
import zipfile
import xml.etree.ElementTree as et

pp = pprint.PrettyPrinter(indent=4)

def Usage():
    print('ninjahotspots.py [--fix-style-url bool] [--remove-AOI bool]')
    print('                 [--remove-footprints bool] [--move-legend bool]')
    print('                 [--move-logo bool] [--scale size] [--log-level level]')
    print('                 input_url output_filename')
    sys.exit()
    
def modify_kml(args):
    args = sys.argv
    fix_style_url = 'True'
    remove_AOI = 'True'
    remove_footprints = 'True'
    move_legend = 'True'
    move_logo = 'True'
    in_scale = 1.0
    input_url = None
    output_filename = None
    log_level = logging.ERROR
    log_levels = {'debug'   :logging.DEBUG,
                  'info'    :logging.INFO,
                  'warning' :logging.WARNING,
                  'error'   :logging.ERROR,
                  'critical':logging.CRITICAL}
    i = 1
    while i < len(args):
        arg = args[i]
        if arg == '--fix-style-url':
            i += 1
            fix_style_url = args[i]
        elif arg == '--remove-AOI':
            i += 1
            remove_AOI = args[i]
        elif arg == '--remove-footprints':
            i += 1
            remove_footprints = args[i]
        elif arg == '--move-legend':
            i += 1
            move_legend = args[i]
        elif arg == '--move-logo':
            i += 1
            move_logo = args[i]
        elif arg == '--scale':
            i += 1
            in_scale = args[i]
        elif arg == '--log-level':
            i += 1
            try:
                log_level = log_levels[args[i].lower()]
            except KeyError:
                print('Invalid logging level, use one of: %s' %
                        str(log_levels.keys()))
                Usage()
        elif arg == '--help':
            Usage()
        elif input_url == None:
            input_url = args[i]
        elif output_filename == None:
            output_filename = args[i]
        else:
            Usage()
        i += 1
    
    logging.basicConfig(level=log_level)
    tries = 0
    wait = 60
    good = False
    while tries < 3 and not good:
        try:
            tries += 1
            fin = urllib2.urlopen(input_url)
            good = True
        except:
            logging.info('Could not connect to server, trying again in 1 min.')
            good = False
            time.sleep(wait)
    if not good:
        logging.critical('Could not connect to server')
        sys.exit(1)
    
    #url = 'http://activefiremaps.fs.fed.us/data/kml/conus.kmz'
    #fin = urllib2.urlopen(url)
    zin = zipfile.ZipFile(StringIO(fin.read()))
    for f in zin.namelist():
        if f.endswith('.kml'):
            filename = f
            break
      
    et._namespace_map['http://earth.google.com/kml/2.1'] = ''
      
    #print('Filename in zip is ' + filename)
    tree = et.parse(zin.open(filename))
    #tree = et.parse('conus.kml')
    root = tree.getroot()

    #Make parent/child map to be able to access parents later...
    parent_map = dict((c, p) for p in root.getiterator() for c in p)

    document = root.find('{http://earth.google.com/kml/2.1}Document')

    #Remove "Footprints", which are polygons of the pixel
    if eval_bool(remove_footprints):
        for folder in document.findall('./{http://earth.google.com/kml/2.1}Folder'):
            name = folder.find('{http://earth.google.com/kml/2.1}name').text
            if name.count('Footprints'):
                document.remove(folder)
   
    #Remove the "Area of Interest Boundary" polygon
    if eval_bool(remove_AOI):
        for folder in document.findall('./{http://earth.google.com/kml/2.1}Folder'):
            name = folder.find('{http://earth.google.com/kml/2.1}name').text
            if name.count('Area of Interest Boundary'):
                document.remove(folder)
       
    #Move legend to top middle
    for folder in document.findall('./{http://earth.google.com/kml/2.1}Folder'):
        name = folder.find('{http://earth.google.com/kml/2.1}name').text
        if name.count('Legend and Logos'):
            for ScreenOverlay in folder.findall('./{http://earth.google.com/'+
                                                 'kml/2.1}ScreenOverlay'):
                name = ScreenOverlay.find('{http://earth.google.com/kml/2.1}'+
                                          'name').text
                if eval_bool(move_legend):
                    if name == 'Legend':
                        ScreenOverlay.find('./{http://earth.google.com/kml/2.1}'+
                                           'screenXY').attrib['x'] = "0.5"
                        ScreenOverlay.find('./{http://earth.google.com/kml/2.1}'+
                                           'overlayXY').attrib['x'] = "0.5"
                if eval_bool(move_logo):
                    if name == 'Logo':
                        ScreenOverlay.find('./{http://earth.google.com/kml/2.1}'+
                                           'screenXY').attrib['x'] = "0.5"
                        ScreenOverlay.find('./{http://earth.google.com/kml/2.1}'+
                                           'overlayXY').attrib['x'] = "0.5"
                         
    #Remove reference to styleUrl (not supported in Google Maps) and replace
    #with direct url to icon image
    if eval_bool(fix_style_url):
        d = dict()
        #Grab Style url at top and then remove this tag             
        for style in document.findall('./{http://earth.google.com/kml/2.1}Style'):
            href = style.find('.//{http://earth.google.com/kml/2.1}href')
            if href is not None:
                d[style.attrib['id']] = href.text
            document.remove(style)    

        #Search for all the styleUrls, remove, and then replace with the url 
        for styleUrl in document.findall('.//{http://earth.google.com/kml/2.1}'+
                                         'styleUrl'):
            parent = parent_map[styleUrl]
            parent.remove(styleUrl)
            Style = et.SubElement(parent, 'Style')
            IconStyle = et.SubElement(Style, 'IconStyle')
            scale = et.SubElement(IconStyle, 'scale')
            scale.text = str(in_scale)
            Icon = et.SubElement(IconStyle, 'Icon')
            href = et.SubElement(Icon, 'href')
            href.text = d[styleUrl.text]
           
        #Remove all names of placemarks since they now are plotted for each icon(?) 
        for Placemark in document.findall('.//{http://earth.google.com/kml/2.1}'+
                                         'Placemark'):
            name = Placemark.find('./{http://earth.google.com/kml/2.1}name')
            if name.text == ' Fire Detection Centroid ':
                Placemark.remove(name)

    fout = zipfile.ZipFile(output_filename, mode='w', compression=zipfile.ZIP_DEFLATED)
    fout.writestr(output_filename[:-1]+'l', et.tostring(tree.getroot()))
    fout.close()
    

def eval_bool(value):
   v = value.lower()
   if(v in set(['yes', 'on', 'true', '1'])):
       return True
   elif(v in set(['no', 'off', 'false', '0'])):
       return False
   raise ValueError('Invalid boolean identifier')

if __name__ == '__main__':
    modify_kml(sys.argv)
