#!/bin/bash

# Smokejumper specific environment variables
export WRITE_TURBULENCE=TRUE
export TURBULENCE_KML_OUTPUT_COLORRAMPTYPE=specificVals

# Google Cloud Platform Keys
export GS_OAUTH2_CLIENT_EMAIL=
export GS_OAUTH2_PRIVATE_KEY_FILE=
# export GS_SECRET_ACCESS_KEY=yourkeyhere
# export GS_ACCESS_KEY_ID=yourkeyhere

# Get directory where this script is located
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
OUTPUT_DIR="$SCRIPT_DIR/outputs"

# Create output directory if it doesn't exist
mkdir -p "$OUTPUT_DIR"

# PASTCAST Inputs
DATE="9/20/2026"
TIME="1356PT"
YEAR=$(echo "$DATE" | awk -F'/' '{print $3}')
MONTH=$(echo "$DATE" | awk -F'/' '{print $1}')
DAY=$(echo "$DATE" | awk -F'/' '{print $2}')
TIME_ZONE_CODE=$(echo "$TIME" | sed -E 's/[0-9]+//g')
case "$TIME_ZONE_CODE" in
    PT|PST|PDT) TIME_ZONE="America/Los_Angeles" ;;
    MT|MST|MDT) TIME_ZONE="America/Denver" ;;
    CT|CST|CDT) TIME_ZONE="America/Chicago" ;;
    ET|EST|EDT) TIME_ZONE="America/New_York" ;;
    AK|AKST|AKDT) TIME_ZONE="America/Anchorage" ;;
esac
HOUR=$(echo "$TIME" | sed -E 's/([0-9]{2})([0-9]{2}).*/\1/')
MINUTE=$(echo "$TIME" | sed -E 's/([0-9]{2})([0-9]{2}).*/\2/')

# Elevation File Inputs
DD="N37°36.453’"
DMS="W119°35.490’"

# Convert Latitude
LAT=$(echo "$DD" | awk '{
    dir = substr($0, 1, 1);
    gsub(/[^0-9.]/, " ", $0);
    split($0, a);
    dd = a[1] + (a[2] / 60);
    if (dir == "S") dd = -dd;
    printf "%.5f", dd;
}')

# Convert Longitude
LON=$(echo "$DMS" | awk '{
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
  --num_threads 4 \
  --momentum_flag true \
  --fetch_elevation "$OUTPUT_DIR/smokejumper.tif" \
  --elevation_source lcp \
  --mesh_choice fine \
  --x_center "$LON" \
  --y_center "$LAT" \
  --x_buffer 3 \
  --y_buffer 3 \
  --buffer_units miles \
  --initialization_method wxModelInitialization \
  --wx_model_type PASTCAST-GCP-HRRR-CONUS-3-KM \
  --time_zone "$TIME_ZONE" \
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
  --diurnal_winds true \
  --write_goog_output true \
  --output_path "$OUTPUT_DIR"
