/******************************************************************************
*
* $Id: layerSources.js
*
* Project:  WindNinja
* Purpose:  Handles map controls (center, clear, load, snapshot)
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

// Controls State Tracking
let centerControlAdded = false;
let clearControlAdded = false;

// Leaflet Control Declarations
const centerControl = L.control({ position: 'topleft' });
const clearControl = L.control({ position: 'topright' });
const loadControl = L.control({ position: 'topright' });
const snapshotControl = L.control({ position: 'topright' });

/**
 * Checks if there are items worth centering on.
 */
function hasCenterableItems() {
    return (
        boundingBoxLayer.getLayers().length > 0 ||
        demLayer.getLayers().length > 0 ||
        (lastActiveLayer && map.hasLayer(lastActiveLayer))
    );
}

/**
 * Dynamically toggles visibility of the Center Control.
 */
function updateCenterControl() {
    const shouldShow = hasCenterableItems();

    if (shouldShow && !centerControlAdded) {
        centerControl.addTo(map);
        centerControlAdded = true;
    } else if (!shouldShow && centerControlAdded) {
        map.removeControl(centerControl);
        centerControlAdded = false;
    }
}

/**
 * Checks if there are items worth clearing.
 */
function hasClearableItems() {
    return (
        boundingBoxLayer.getLayers().length > 0 ||
        windninjaOutputTree.children.length > 1 ||
        initializationOutputTree.children.length > 1 ||
        stationOutputTree.children.length > 1 ||
        unknownOutputTree.children.length > 1
    );
}

/**
 * Dynamically toggles visibility of the Clear Control.
 */
function updateClearControl() {
    const shouldShow = hasClearableItems();

    if (shouldShow && !clearControlAdded) {
        clearControl.addTo(map);
        clearControlAdded = true;
    } else if (!shouldShow && clearControlAdded) {
        map.removeControl(clearControl);
        clearControlAdded = false;
    }
}

// --- Control Handlers ---

// Center Button Control
centerControl.onAdd = function (mapInstance) {
    const container = L.DomUtil.create('div', 'leaflet-bar leaflet-control leaflet-control-center');
    const button = L.DomUtil.create('a', 'leaflet-center-button', container);
    button.innerHTML = '⌖';
    button.title = 'Center on current layer';
    button.href = '#';

    L.DomEvent.on(button, 'click', (e) => {
        L.DomEvent.stopPropagation(e);
        L.DomEvent.preventDefault(e);
        centerMapOnActiveLayer();
    });

    return container;
};

// Clear Button Control
clearControl.onAdd = function (mapInstance) {
    const container = L.DomUtil.create('div', 'leaflet-bar leaflet-control');
    const button = L.DomUtil.create('a', 'leaflet-control-clear', container);
    button.innerHTML = 'Clear';
    button.title = 'Clear all layers';
    button.href = '#';

    L.DomEvent.on(button, 'click', (e) => {
        L.DomEvent.stopPropagation(e);
        L.DomEvent.preventDefault(e);

        clearBoundingBoxLayer();
        clearWindNinjaOutputTree();
        clearInitializationOutputTree();
        clearStationOutputTree();
        clearUnknownOutputTree();
        clearTimeSeriesContainer();

        activeWindNinjaLayer = null;
        activeInitializationLayer = null;
        activeWindNinjaTimeSeriesImg = null;
        activeInitializationTimeSeriesImg = null;
    });

    return container;
};

// Load Output File Control
loadControl.onAdd = function (mapInstance) {
    const container = L.DomUtil.create('div', 'leaflet-bar leaflet-control');
    const button = L.DomUtil.create('a', 'leaflet-control-load', container);
    button.innerHTML = '+';
    button.title = 'Load output file';
    button.href = '#';

    L.DomEvent.on(button, 'click', (e) => {
        L.DomEvent.stopPropagation(e);
        L.DomEvent.preventDefault(e);

        if (window.bridge?.loadMapLayers) {
            window.bridge.loadMapLayers();
        }
    });

    return container;
};

// Save Map Snapshot Control
snapshotControl.onAdd = function (mapInstance) {
    const container = L.DomUtil.create('div', 'leaflet-bar leaflet-control');
    const button = L.DomUtil.create('a', 'leaflet-control-snapshot', container);
    button.innerHTML = '📷';
    button.title = 'Save Map Snapshot';
    button.href = '#';

    L.DomEvent.on(button, 'click', (e) => {
        L.DomEvent.stopPropagation(e);
        L.DomEvent.preventDefault(e);

        if (window.bridge?.captureMapSnapshot) {
            window.bridge.captureMapSnapshot();
        }
    });

    return container;
};

/**
 * Initializes static UI controls on the map instance.
 * Call this after the map object is created.
 */
function initMapControls(map) {
    loadControl.addTo(map);
    snapshotControl.addTo(map);
}