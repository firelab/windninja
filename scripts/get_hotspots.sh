#Get MODIS
python ninjahotspots.py http://activefiremaps.fs.fed.us/data/kml/conus.kmz --scale 0.25 modis_conus.kmz
python ninjahotspots.py http://activefiremaps.fs.fed.us/data/kml/alaska.kmz --scale 0.25 modis_alaska.kmz
python ninjahotspots.py http://activefiremaps.fs.fed.us/data/kml/hawaii.kmz --scale 0.25 modis_hawaii.kmz
python ninjahotspots.py http://activefiremaps.fs.fed.us/data/kml/canada.kmz --scale 0.25 modis_canada.kmz

#Get VIIRS
python ninjahotspots.py http://activefiremaps.fs.fed.us/data_viirs/kml/conus.kmz --scale 0.25 viirs_conus.kmz
python ninjahotspots.py http://activefiremaps.fs.fed.us/data_viirs/kml/alaska.kmz --scale 0.25 viirs_alaska.kmz
python ninjahotspots.py http://activefiremaps.fs.fed.us/data_viirs/kml/hawaii.kmz --scale 0.25 viirs_hawaii.kmz
python ninjahotspots.py http://activefiremaps.fs.fed.us/data_viirs/kml/canada.kmz --scale 0.25 viirs_canada.kmz
