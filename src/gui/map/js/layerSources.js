/******************************************************************************
*
* $Id: layerSources.js
*
* Project:  WindNinja
* Purpose:  Handles basemaps and fire data layers 
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
    id: 'mapbox/streets-v12'
});

const satelliteBaseLayer = L.tileLayer(`https://api.mapbox.com/styles/v1/{id}/tiles/{z}/{x}/{y}?access_token=${apiKey}`, {
    tileSize: 512,
    maxZoom: 18,
    zoomOffset: -1,
    id: 'mapbox/satellite-v9'
});

const topographicBaseLayer = L.tileLayer(
    `https://api.mapbox.com/styles/v1/{id}/tiles/{z}/{x}/{y}?access_token=${apiKey}`,
    {
        tileSize: 512,
        maxZoom: 18,
        zoomOffset: -1,
        id: 'mapbox/outdoors-v12'
    }
);

const usgsTopoBaseLayer = L.tileLayer(
    'https://basemap.nationalmap.gov/arcgis/rest/services/USGSTopo/MapServer/tile/{z}/{y}/{x}',
    {
        maxZoom: 16,
        attribution: 'USGS The National Map'
    }
);

const firePerimeterLayer = L.esri.featureLayer({
    url: "https://services3.arcgis.com/T4QMspbfLg3qTGWY/arcgis/rest/services/WFIGS_Interagency_Perimeters_Current/FeatureServer/0",

    simplifyFactor: 0.5,

    style: {
        color: "#E60000",
        weight: 2,
        fillColor: "#FFBEBE",
        fillOpacity: 0.7
    }
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
    }
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
    }
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

        layer.bindPopup(`
            <strong>MODIS Thermal Hotspot</strong><br>
            <br>
            <strong>Satellite:</strong> ${properties.SATELLITE || "N/A"}<br>
            <strong>Acquired:</strong> ${acquisitionDate}<br>
            <strong>Age:</strong> ${properties.HOURS_OLD != null ? properties.HOURS_OLD + " hours" : "N/A"}<br>
            <strong>Confidence:</strong> ${properties.CONFIDENCE != null ? properties.CONFIDENCE + "%" : "N/A"}<br>
            <strong>Fire Radiative Power:</strong> ${properties.FRP != null ? properties.FRP.toFixed(1) + " MW" : "N/A"}<br>
            <strong>Brightness:</strong> ${properties.BRIGHTNESS != null ? properties.BRIGHTNESS.toFixed(1) + " K" : "N/A"}<br>
            <strong>Detection:</strong> ${detection}
        `);
    }
});

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

console.log("MAP.JS LOADED");