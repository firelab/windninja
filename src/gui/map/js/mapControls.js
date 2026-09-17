/******************************************************************************
*
* $Id: mapControls.js
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

let centerControlAdded = false;
let clearControlAdded = false;

const centerControl = L.control({ position: 'topleft' });
const clearControl = L.control({ position: 'topright' });
const loadControl = L.control({ position: 'topright' });
const snapshotControl = L.control({ position: 'topright' });

function hasCenterableItems() {
    return (
        boundingBoxLayer.getLayers().length > 0 ||
        demLayer.getLayers().length > 0 ||
        (lastActiveLayer && map.hasLayer(lastActiveLayer))
    );
}

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

function hasClearableItems() {
    return (
        boundingBoxLayer.getLayers().length > 0 ||
        windninjaOutputTree.children.length > 1 ||
        initializationOutputTree.children.length > 1 ||
        stationOutputTree.children.length > 1 ||
        unknownOutputTree.children.length > 1
    );
}

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

snapshotControl.onAdd = function (map) {
    const container = L.DomUtil.create('div', 'leaflet-bar leaflet-control');
    const button = L.DomUtil.create('a', 'leaflet-control-snapshot', container);
    button.innerHTML = '📷';
    button.title = 'Capture Map Snapshot';
    button.href = '#';

    L.DomEvent.on(button, 'click', async (e) => {
        L.DomEvent.stopPropagation(e);
        L.DomEvent.preventDefault(e);

        if (window.bridge?.captureMapSnapshot) {
            const controls = map._container.querySelectorAll('.leaflet-control-container');
            controls.forEach(ctrl => ctrl.style.display = 'none');

            try {
                await new Promise(resolve => requestAnimationFrame(() => setTimeout(resolve, 50)));
                await window.bridge.captureMapSnapshot();
            } finally {
                controls.forEach(ctrl => ctrl.style.display = '');
            }
        }
    });

    return container;
};

function initMapControls(map) {
    loadControl.addTo(map);
    snapshotControl.addTo(map);
}