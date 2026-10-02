/******************************************************************************
*
* $Id: mapInitialization.js
*
* Project:  WindNinja
* Purpose:  Handles map definition and layer fetching
* Author:   Mason Willman <mason.willman@usda.gov>
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

const apiKey = "pk.eyJ1IjoiYm5vcmRncmVuIiwiYSI6ImNsZmxuMHowZzAzaTczeG80ZXR3a2ZnNHEifQ.kc7P57DJg8tyDMjjP7czuQ";

const streetBaseLayer = L.tileLayer(`https://api.mapbox.com/styles/v1/{id}/tiles/{z}/{x}/{y}?access_token=${apiKey}`, {
    tileSize: 512,
    maxZoom: 18,
    zoomOffset: -1,
    id: 'mapbox/streets-v12',
    zIndex: 200
});

const satelliteBaseLayer = L.tileLayer(`https://api.mapbox.com/styles/v1/{id}/tiles/{z}/{x}/{y}?access_token=${apiKey}`, {
    tileSize: 512,
    maxZoom: 18,
    zoomOffset: -1,
    id: 'mapbox/satellite-v9',
    zIndex: 200
});

const topographicBaseLayer = L.tileLayer(
    `https://api.mapbox.com/styles/v1/{id}/tiles/{z}/{x}/{y}?access_token=${apiKey}`,
    {
        tileSize: 512,
        maxZoom: 18,
        zoomOffset: -1,
        id: 'mapbox/outdoors-v12',
        zIndex: 200
    }
);

const usgsTopoBaseLayer = L.tileLayer(
    'https://basemap.nationalmap.gov/arcgis/rest/services/USGSTopo/MapServer/tile/{z}/{y}/{x}',
    {
        maxZoom: 16,
        attribution: 'USGS The National Map',
        zIndex: 200
    }
);

const map = new L.Map('map', {
    preferCanvas: true,
    worldCopyJump: true,
    layers: [streetBaseLayer],
    center: [37.5, -96.5],
    zoom: 4
});

map.createPane('fsTopoPane');
map.getPane('fsTopoPane').style.zIndex = 200;

map.createPane('fsHillshadePane');
map.getPane('fsHillshadePane').style.zIndex = 200;

map.createPane('firePerimeterPane');
map.getPane('firePerimeterPane').style.zIndex = 300;

map.createPane('demPane');
map.getPane('demPane').style.zIndex = 350; 

const fsTopoBaseLayer = L.esri.Vector.vectorTileLayer(
    "https://tiles.arcgis.com/tiles/gGHDlz6USftL5Pau/arcgis/rest/services/FSBasemap_20240617/VectorTileServer",
    {
        maxZoom: 16,
        pane: 'fsTopoPane'
    }
);

const fsHillshadeLayer = L.tileLayer(
    "https://services.arcgisonline.com/arcgis/rest/services/Elevation/World_Hillshade/MapServer/tile/{z}/{y}/{x}",
    {
        maxZoom: 16,
        pane: 'fsHillshadePane',
        opacity: 0.35,
        attribution: "Esri"
    }
);

const fsTopoHillshadeLayer = L.layerGroup([
    fsTopoBaseLayer,
    fsHillshadeLayer
]);


const firePerimeterLayer = L.esri.featureLayer({
    url: "https://services3.arcgis.com/T4QMspbfLg3qTGWY/arcgis/rest/services/WFIGS_Interagency_Perimeters_Current/FeatureServer/0",

    pane: 'firePerimeterPane',

    simplifyFactor: 0.5,

    style: {
        color: "#E60000",
        weight: 2,
        fillColor: "#FFBEBE",
        fillOpacity: 0.7
    },
});

const firePointsLayer = L.esri.featureLayer({
    url: "https://services3.arcgis.com/T4QMspbfLg3qTGWY/ArcGIS/rest/services/WFIGS_Incident_Locations_Current/FeatureServer/0",

    pointToLayer: function (geojson, latlng) {
        return L.circleMarker(latlng, {
            radius: 5,
            color: "#000000",
            weight: 1,
            fillColor: "#000000",
            fillOpacity: 1
        });
    },

    onEachFeature: function (feature, layer) {
        const properties = feature.properties;

        const incidentType = {
            WF: "Wildfire",
            RX: "Prescribed Fire",
            CX: "Incident Complex"
        };

        const type = incidentType[properties.IncidentTypeCategory] || "Incident";

        const discovered = properties.FireDiscoveryDateTime
            ? new Date(properties.FireDiscoveryDateTime).toLocaleDateString()
            : "N/A";

        const updated = properties.ModifiedOnDateTime_dt
            ? new Date(properties.ModifiedOnDateTime_dt).toLocaleDateString()
            : "N/A";

        layer.bindPopup(`
            <strong>${properties.IncidentName || "Unknown Fire"}</strong><br>
            ${type} · ${properties.POOCounty || "N/A"} County, ${properties.POOState || "N/A"}<br>
            <br>
            <strong>Size:</strong> ${properties.IncidentSize != null ? properties.IncidentSize.toLocaleString() + " acres" : "N/A"}<br>
            <strong>Contained:</strong> ${properties.PercentContained != null ? properties.PercentContained + "%" : "N/A"}<br>
            <strong>Fire Behavior:</strong> ${properties.FireBehaviorGeneral || "N/A"}<br>
            <strong>Discovered:</strong> ${discovered}<br>
            <strong>Last Updated:</strong> ${updated}<br>
        `);
    },
});

const viirsHotspotsLayer = L.esri.featureLayer({
    url: "https://services9.arcgis.com/RHVPKKiFTONKtxq3/arcgis/rest/services/Satellite_VIIRS_Thermal_Hotspots_and_Fire_Activity/FeatureServer/0",

    pointToLayer: function (geojson, latlng) {
        const properties = geojson.properties;
        const color = getHotspotColor(properties.hours_old);

        return L.circleMarker(latlng, {
            radius: 5,
            color: color,
            weight: 1,
            fillColor: color,
            fillOpacity: 0.7
        });
    },

    onEachFeature: function (feature, layer) {
        const properties = feature.properties;

        const acquisitionDate = properties.acq_date
            ? new Date(properties.acq_date).toLocaleString()
            : "N/A";

        const detection = properties.daynight === "D"
            ? "Day"
            : properties.daynight === "N"
                ? "Night"
                : "N/A";

        layer.bindPopup(`
            <strong>VIIRS Thermal Hotspot</strong><br>
            <br>
            <strong>Satellite:</strong> ${properties.satellite || "N/A"}<br>
            <strong>Acquired:</strong> ${acquisitionDate}<br>
            <strong>Age:</strong> ${properties.hours_old != null ? properties.hours_old + " hours" : "N/A"}<br>
            <strong>Confidence:</strong> ${properties.confidence || "N/A"}<br>
            <strong>Fire Radiative Power:</strong> ${properties.frp != null ? properties.frp.toFixed(1) + " MW" : "N/A"}<br>
            <strong>Brightness Temperature:</strong> ${properties.bright_ti4 != null ? properties.bright_ti4.toFixed(1) + " K" : "N/A"}<br>
            <strong>Detection:</strong> ${detection}
        `);
    },
});

const modisHotspotsLayer = L.esri.featureLayer({
    url: "https://services9.arcgis.com/RHVPKKiFTONKtxq3/arcgis/rest/services/MODIS_Thermal_v1/FeatureServer/1",

    pointToLayer: function (geojson, latlng) {
        const properties = geojson.properties;
        const hoursOld = properties.HOURS_OLD;

        const color = getHotspotColor(hoursOld);

        return L.circleMarker(latlng, {
            radius: 5,
            color: color,
            weight: 1,
            fillColor: color,
            fillOpacity: 0.9
        });
    },

    onEachFeature: function (feature, layer) {
        const properties = feature.properties;

        const acquisitionDate = properties.ACQ_DATE
            ? new Date(properties.ACQ_DATE).toLocaleString()
            : "N/A";

        const detection = properties.DAYNIGHT === "D"
            ? "Day"
            : properties.DAYNIGHT === "N"
                ? "Night"
                : "N/A";

        const satellite = properties.SATELLITE === "A"
            ? "Aqua"
            : properties.SATELLITE === "T"
                ? "Terra"
                : "N/A";

        layer.bindPopup(`
            <strong>MODIS Thermal Hotspot</strong><br>
            <br>
            <strong>Satellite:</strong> ${satellite}<br>
            <strong>Acquired:</strong> ${acquisitionDate}<br>
            <strong>Age:</strong> ${properties.HOURS_OLD != null ? properties.HOURS_OLD + " hours" : "N/A"}<br>
            <strong>Confidence:</strong> ${properties.CONFIDENCE != null ? properties.CONFIDENCE + "%" : "N/A"}<br>
            <strong>Fire Radiative Power:</strong> ${properties.FRP != null ? properties.FRP.toFixed(1) + " MW" : "N/A"}<br>
            <strong>Brightness:</strong> ${properties.BRIGHTNESS != null ? properties.BRIGHTNESS.toFixed(1) + " K" : "N/A"}<br>
            <strong>Detection:</strong> ${detection}
        `);
    },
});

const goesWestHotspotsLayer = L.geoJSON(null, {

    pointToLayer: function (feature, latlng) {

        return L.circleMarker(latlng, {
            radius: 5,
            color: "#8B0000",
            weight: 1,
            fillColor: "#FF4500",
            fillOpacity: 0.8
        });

    },

    onEachFeature: function (feature, layer) {

        const properties = feature.properties;

        const datetime = properties.acq_date_time
                    ? new Date(properties.acq_date_time).toLocaleString()
                    : "N/A"
        
        const frp = properties.total_frp != null
                    ? properties.total_frp.toFixed(1) + " MW"
                    : "N/A"

        layer.bindPopup(`
            <strong>${properties.known_incident_name || "Possible Wildland Fire"}</strong><br>
            <br>
            <strong>Type:</strong> ${properties.type_description || "N/A"}<br>
            <strong>Satellite:</strong> ${properties.satellite || "N/A"}<br>
            <strong>Acquired:</strong> ${datetime}<br>
            <strong>FRP:</strong> ${frp}<br>
            <strong>Confidence:</strong> ${properties.confidence || "N/A"}<br>
            <strong>County:</strong> ${properties.county || "N/A"}<br>
            <strong>State:</strong> ${properties.state || "N/A"}<br>
            <strong>Feature Tracking ID:</strong> ${properties.feature_tracking_id || "N/A"}
        `);

    },
});

function fetchGoesWest(map) {

    if (!map.hasLayer(goesWestHotspotsLayer)) return;

    const bounds = map.getBounds();

    const bbox =
        `${bounds.getWest()},${bounds.getSouth()},` +
        `${bounds.getEast()},${bounds.getNorth()}`;

    const url =
        "https://fire.data.nesdis.noaa.gov/api/ogc/detections" +
        "/collections/ngfs_schema.ngfs_features_scene_west_conus/items" +
        `?bbox=${bbox}&limit=10000`;


    fetch(url)

        .then(res => {
            return res.json();
        })

        .then(data => {
            goesWestHotspotsLayer.clearLayers();

            if (data && data.features) {
                goesWestHotspotsLayer.addData(data);
            } else {
                console.error("No features in NGFS response");
            }

        })

        .catch(err => {
            console.error("Error fetching NGFS features:", err);
        });

}

const goesEastHotspotsLayer = L.geoJSON(null, {

    pointToLayer: function (feature, latlng) {

        return L.circleMarker(latlng, {
            radius: 5,
            color: "#8B0000",
            weight: 1,
            fillColor: "#FF4500",
            fillOpacity: 0.8
        });

    },

    onEachFeature: function (feature, layer) {

        const properties = feature.properties;

        const datetime = properties.acq_date_time
                    ? new Date(properties.acq_date_time).toLocaleString()
                    : "N/A"
        
        const frp = properties.total_frp != null
                    ? properties.total_frp.toFixed(1) + " MW"
                    : "N/A"

        layer.bindPopup(`
            <strong>${properties.known_incident_name || "Possible Wildland Fire"}</strong><br>
            <br>
            <strong>Type:</strong> ${properties.type_description || "N/A"}<br>
            <strong>Satellite:</strong> ${properties.satellite || "N/A"}<br>
            <strong>Acquired:</strong> ${datetime}<br>
            <strong>FRP:</strong> ${frp}<br>
            <strong>Confidence:</strong> ${properties.confidence || "N/A"}<br>
            <strong>County:</strong> ${properties.county || "N/A"}<br>
            <strong>State:</strong> ${properties.state || "N/A"}<br>
            <strong>Feature Tracking ID:</strong> ${properties.feature_tracking_id || "N/A"}
        `);

    },
});


function fetchGoesEast(map) {

    if (!map.hasLayer(goesEastHotspotsLayer)) return;

    const bounds = map.getBounds();
    const bbox =
        `${bounds.getWest()},${bounds.getSouth()},` +
        `${bounds.getEast()},${bounds.getNorth()}`;

    const url =
        "https://fire.data.nesdis.noaa.gov/api/ogc/detections" +
        "/collections/ngfs_schema.ngfs_features_scene_east_conus/items" +
        `?bbox=${bbox}&limit=10000`;

    fetch(url)
        .then(res => {
            return res.json();
        })

        .then(data => {
            goesEastHotspotsLayer.clearLayers();

            if (data && data.features) {
                goesEastHotspotsLayer.addData(data);
            } else {
                console.error("No features in NGFS response");
            }

        })

        .catch(err => {
            console.error( "Error fetching NGFS features:", err);
        });
}

function getHotspotColor(hoursOld) {
    if (hoursOld < 1) {
        return "#a80000";
    }

    if (hoursOld < 6) {
        return "#e64000";
    }

    if (hoursOld < 12) {
        return "#ff5500";
    }

    if (hoursOld < 24) {
        return "#ffaa00";
    }

    return "#ffff00";
}

const synopticToken = "33e3c8ee12dc499c86de1f2076a9e9d4";

const selectedStationIds = new Set();

const synopticStationsLayer = L.geoJSON(null, {

    pointToLayer: function (feature, latlng) {

        const stationId = feature.properties?.stid;
        const isSelected = selectedStationIds.has(stationId);

        return L.circleMarker(latlng, {
            radius: 6,
            color: isSelected ? "#ff0000" : "#003366",
            weight: 1.5,
            fillColor: isSelected ? "#ff0000" : "#3388ff",
            fillOpacity: 0.8
        });

    },

    onEachFeature: function (feature, layer) {

        const properties = feature.properties;

        layer.bindPopup(`
            <strong>${properties.name || "Unknown Station"}</strong><br>
            <strong>Station ID:</strong> ${properties.stid ?? "N/A"}<br>
            <strong>Latitude:</strong> ${properties.latitude ?? "N/A"}<br>
            <strong>Longitude:</strong> ${properties.longitude ?? "N/A"}<br>
            <strong>Elevation:</strong> ${properties.elevation ?? "N/A"} ft<br>
            <strong>State:</strong> ${properties.state ?? "N/A"}<br>
            <strong>Country:</strong> ${properties.country ?? "N/A"}<br>
            <strong>Status:</strong> ${properties.status ?? "N/A"}<br>
        `);

    }

});

function updateSynopticStations(stationIds) {

    selectedStationIds.clear();

    stationIds.forEach(function (stationId) {
        selectedStationIds.add(stationId);
    });

    synopticStationsLayer.eachLayer(function (layer) {

        const stationId = layer.feature?.properties?.stid;
        const isSelected = selectedStationIds.has(stationId);

        layer.setStyle({
            color: isSelected ? "#ff0000" : "#003366",
            fillColor: isSelected ? "#ff0000" : "#3388ff"
        });

    });
}

function fetchSynopticStations(mapInstance) {
    if (!mapInstance.hasLayer(synopticStationsLayer)) return;

    const bounds = mapInstance.getBounds();

    // BBOX format: west,south,east,north
    const bbox =
        `${bounds.getWest()},${bounds.getSouth()},` +
        `${bounds.getEast()},${bounds.getNorth()}`;

    const url =
        `https://api.synopticdata.com/v2/stations/latest` +
        `?token=${synopticToken}` +
        `&bbox=${bbox}` +
        '&network=1,2' +
        `&output=geojson`;

    console.log("Fetching Synoptic stations:", url);

    fetch(url)
        .then(res => {
            console.log("Synoptic HTTP status:", res.status, res.statusText);
            return res.json();
        })
        .then(data => {
            console.log("Synoptic response:", data);

            synopticStationsLayer.clearLayers();

            if (data && data.features) {
                console.log("Number of features:", data.features.length);
                synopticStationsLayer.addData(data);
            } else {
                console.error("No features in Synoptic response");
            }
        })
        .catch(err => {
            console.error("Error fetching Synoptic Data stations:", err);
        });
}