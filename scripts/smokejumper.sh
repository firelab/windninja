#!/bin/bash

# WindNinja Smokejumper Simulation Script
#
# This script runs a WindNinja simulation using:
#   - Archived HRRR Forecast data (Google Cloud Platform keys needed)
#   - A dem fetched via a Point and Radius
# And is written for a workflow specific to requests from Smokejumpers. 
#
# Input formats:
#   DATE           - Simulation date in M/D/YYYY format (e.g. 9/20/2026)
#   TIME           - Simulation time in HH:MM format and UTC time zone (e.g. 13:00)
#   LAT            - Latitude in Degrees Decimal Minutes format with a hemisphere (e.g. N37°36.453’)
#   LON            - Longitude in Degrees Decimal Minutes format with a hemisphere (e.g. W119°35.490’)
#   RADIUS         - Simulation radius in miles
#   NUM_THREADS    - Number of CPU threads used by WindNinja
#   OUTPUT_DIR     - Directory where WindNinja output files will be written
#   ELEVATION_FILE - File name used to save the fetched DEM

# Environment Variables
export WRITE_TURBULENCE=TRUE
export TURBULENCE_KML_OUTPUT_COLORRAMPTYPE=specificVals

export GS_OAUTH2_CLIENT_EMAIL=yourkeyhere
export GS_OAUTH2_PRIVATE_KEY_FILE=yourkeyhere

# export GS_SECRET_ACCESS_KEY=yourkeyhere
# export GS_ACCESS_KEY_ID=yourkeyhere

# Inputs
DATE="9/20/2026"
TIME="1300"
LAT="N37°36.453’"
LON="W119°35.490’"
RADIUS=3
NUM_THREADS=4
OUTPUT_DIR="path/to/output/dir"
ELEVATION_FILE="filename.tif"

# Extract datetime
YEAR=$(echo "$DATE" | awk -F'/' '{print $3}')
MONTH=$(echo "$DATE" | awk -F'/' '{print $1}')
DAY=$(echo "$DATE" | awk -F'/' '{print $2}')
HOUR=$(echo "$TIME" | sed -E 's/([0-9]{2})([0-9]{2}).*/\1/')
MINUTE=$(echo "$TIME" | sed -E 's/([0-9]{2})([0-9]{2}).*/\2/')

# Calculate lat lon 
LAT=$(echo "$LAT" | awk '{
    dir = substr($0, 1, 1);
    gsub(/[^0-9.]/, " ", $0);
    split($0, a);
    dd = a[1] + (a[2] / 60);
    if (dir == "S") dd = -dd;
    printf "%.5f", dd;
}')

LON=$(echo "$LON" | awk '{
    dir = substr($0, 1, 1);
    gsub(/[^0-9.]/, " ", $0);
    split($0, a);
    dd = a[1] + (a[2] / 60);
    if (dir == "W") dd = -dd;
    printf "%.5f", dd;
}')

echo "=== WindNinja Simulation Inputs ==="
echo "Date: $MONTH/$DAY/$YEAR | Time: $HOUR:$MINUTE $TIME_ZONE"
echo "Center Point: LAT=$LAT, LON=$LON"

# Execute WindNinja CLI
WindNinja_cli \
  --num_threads "$NUM_THREADS" \
  --momentum_flag true \
  --fetch_elevation "$OUTPUT_DIR/$ELEVATION_FILE" \
  --elevation_source lcp \
  --mesh_choice fine \
  --x_center "$LON" \
  --y_center "$LAT" \
  --x_buffer "$RADIUS" \
  --y_buffer "$RADIUS" \
  --buffer_units miles \
  --initialization_method wxModelInitialization \
  --wx_model_type PASTCAST-GCP-HRRR-CONUS-3-KM \
  --start_year "$YEAR" \
  --start_month "$MONTH" \
  --start_day "$DAY" \
  --start_hour "$HOUR" \
  --start_minute "$MINUTE" \
  --stop_year "$YEAR" \
  --stop_month "$MONTH" \
  --stop_day "$DAY" \
  --stop_hour "$HOUR" \
  --stop_minute "$MINUTE" \
  --output_wind_height 20.0 \
  --units_output_wind_height ft \
  --write_goog_output true \
  --output_path "$OUTPUT_DIR"
