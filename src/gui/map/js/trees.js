/******************************************************************************
*
* $Id: mapTrees.js
*
* Project:  WindNinja
* Purpose:  Handles map trees and related functions
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

const baseTree = {
    label: 'Base Maps',
    children: [
        { label: 'Streets', layer: streetBaseLayer },
        { label: 'Satellite', layer: satelliteBaseLayer },
        { label: 'Topographic', layer: topographicBaseLayer },
        { label: 'USGS Topographic', layer: usgsTopoBaseLayer },
        { label: 'Forest Service Topographic', layer: fsTopoHillshadeLayer }
    ]
};

const windninjaOutputTree = {
    label: 'Simulation Output',
    children: [
        { label: 'No Output', layer: L.layerGroup([]), radioGroup: 'windninjaOutput'}
    ]
};

const initializationOutputTree = {
    label: 'Weather Model Output',
    children: [
        { label: 'No Output', layer: L.layerGroup([]),  radioGroup: 'initializationOutput' }
    ]
};

const stationOutputTree = {
    label: 'Weather Stations',
    children: [
        { label: 'No Output', layer: L.layerGroup([]),  radioGroup: 'stationOutput' }
    ]
};

const unknownOutputTree = {
    label: 'Unknown Output',
    children: [
        { label: 'No Output', layer: L.layerGroup([]), radioGroup: 'unknownOutput' }
    ]
};

const overlayTree = {
    label: 'WindNinja Overlays',
    children: []
};

const fireOverlayTree = {
    label: 'Fire Overlays',
    children: [
        { label: 'Fire Perimeters', layer: firePerimeterLayer },
        { label: 'Fire Points', layer: firePointsLayer },
        { label: 'VIIRS Hotspots', layer: viirsHotspotsLayer },
        { label: 'MODIS Hotspots', layer: modisHotspotsLayer },
        { label: 'GOES West Hotspots', layer: goesWestHotspotsLayer },
        { label: 'GOES East Hotspots', layer: goesEastHotspotsLayer }
    ]
};

const dataOverlayTree = {
    label: 'Data Overlays',
    children: [
        { label: "Stations", layer: synopticStationsLayer}
    ]
};

const layerControl = L.control.layers.tree(
    baseTree,
    [fireOverlayTree, dataOverlayTree],
    { collapsed: true }
).addTo(map);

map.on('overlayadd', e => {

    if (e.layer === windninjaLegendToggle) {
        windninjaLegendEnabled = true;
        if (activeWindNinjaLayer) activeWindNinjaLayer.fire('add');
    }
    if (e.layer === initializationLegendToggle) {
        initializationLegendEnabled = true;
        if (activeInitializationLayer) activeInitializationLayer.fire('add');
    }

    if (e.layer === windninjaTimeSeriesToggle) {
        windninjaTimeSeriesEnabled = true;
        if (activeWindNinjaLayer) activeWindNinjaLayer.fire('add');
    }
    if (e.layer === initializationTimeSeriesToggle) {
        initializationTimeSeriesEnabled = true;
        if (activeInitializationLayer) activeInitializationLayer.fire('add');
    }

    if (e.layer === windninjaOutputTree.children[windninjaOutputTree.children.length-1]?.layer) {
        if (activeWindNinjaTimeSeriesImg) {
            activeWindNinjaTimeSeriesImg.remove();
            activeWindNinjaTimeSeriesImg = null;
        }
        activeWindNinjaLayer = null;
        updateOverlayTree();
    }
    if (e.layer === initializationOutputTree.children[initializationOutputTree.children.length-1]?.layer) {
        if (activeInitializationTimeSeriesImg) {
            activeInitializationTimeSeriesImg.remove();
            activeInitializationTimeSeriesImg = null;
        }
        activeInitializationLayer = null;
        updateOverlayTree();
    }
    if (e.layer === goesEastHotspotsLayer) {
        fetchGoesEast(map);
    }
    if (e.layer === goesWestHotspotsLayer) {
        fetchGoesWest(map);
    }
    if (e.layer === synopticStationsLayer) {
        fetchSynopticStations(map);
    }
});

map.on('overlayremove', e => {

    if (e.layer === windninjaLegendToggle) {
        windninjaLegendEnabled = false;
        if (activeWindNinjaLegendCard) {
            activeWindNinjaLegendCard.remove();
            activeWindNinjaLegendCard = null;
        }            
    }
    if (e.layer === initializationLegendToggle) {
        initializationLegendEnabled = false;
        if (activeInitializationLegendCard) {
            activeInitializationLegendCard.remove();
            activeInitializationLegendCard = null;
        }
    }

    if (e.layer === windninjaTimeSeriesToggle) {
        windninjaTimeSeriesEnabled = false;
        if (activeWindNinjaTimeSeriesImg) {
            activeWindNinjaTimeSeriesImg.remove();
            activeWindNinjaTimeSeriesImg = null;
        }
    }
    if (e.layer === initializationTimeSeriesToggle) {
        initializationTimeSeriesEnabled = false;
        if (activeInitializationTimeSeriesImg) {
            activeInitializationTimeSeriesImg.remove();
            activeInitializationTimeSeriesImg = null;
        }
    }
});

map.on("moveend", function () {
    if(map.hasLayer(goesEastHotspotsLayer)) {
        fetchGoesEast(map);
    }
    if(map.hasLayer(goesWestHotspotsLayer)) {
        fetchGoesWest(map);                
    }
    if (map.hasLayer(synopticStationsLayer)) {
        fetchSynopticStations(map);
    }
});


function updateOverlayTree() 
{
    if (!layerControl || !layerControl.setOverlayTree) return;

    const displayTree = [];
    const overlayChildren = [];

    if (boundingBoxLayer.getLayers().length > 0) {
        overlayChildren.push({
            label: 'Bounding Box',
            layer: boundingBoxLayer
        });
    }

    if (demLayer.getLayers().length > 0) {
        overlayChildren.push({
            label: 'Elevation File Outline',
            layer: demLayer
        });
    }

    if (activeWindNinjaLayer) {

        if (activeWindNinjaLayer._hasLegend) {
            overlayChildren.push({
                label: 'Simulation Legend',
                layer: windninjaLegendToggle
            });
        }

        if (activeWindNinjaLayer._hasTimeSeries) {
            overlayChildren.push({
                label: 'Simulation Datetime',
                layer: windninjaTimeSeriesToggle
            });
        }
    }

    if (activeInitializationLayer) {
        overlayChildren.push({
            label: 'Weather Model Legend',
            layer: initializationLegendToggle
        });

        overlayChildren.push({
            label: 'Weather Model Datetime',
            layer: initializationTimeSeriesToggle
        });
    }

    displayTree.push(fireOverlayTree);
    displayTree.push(dataOverlayTree);

    if (windninjaOutputTree.children.length > 1)
        displayTree.push(windninjaOutputTree);

    if (initializationOutputTree.children.length > 1)
        displayTree.push(initializationOutputTree);

    if (stationOutputTree.children.length > 1)
        displayTree.push(stationOutputTree);

    if (unknownOutputTree.children.length > 1)
        displayTree.push(unknownOutputTree);

    if (overlayChildren.length > 0) {
        displayTree.push({
            label: 'WindNinja Overlays',
            children: overlayChildren
        });
    }

    layerControl.setOverlayTree(displayTree);
}

function clearWindNinjaOutputTree() 
{
    clearTimeSeriesContainer();
    windninjaOutputTree.children.forEach(item => {
        if (item.layer && item.label !== 'No Output') {
            map.removeLayer(item.layer);
        }
    });

    windninjaOutputTree.children = [{
        label: 'No Output',
        layer: L.layerGroup([]),
        radioGroup: 'windninjaOutput'
    }];

    updateOverlayTree();
    updateCenterControl();
    updateClearControl();
}

function clearInitializationOutputTree() 
{
    clearTimeSeriesContainer();
    initializationOutputTree.children.forEach(item => {
        if (item.layer && item.label !== 'No Output') {
            map.removeLayer(item.layer);
        }
    });

    initializationOutputTree.children = [{
        label: 'No Output',
        layer: L.layerGroup([]),
        radioGroup: 'initializationOutput'
    }];

    updateOverlayTree();
    updateCenterControl();
    updateClearControl();
}

function clearStationOutputTree() 
{
    stationOutputTree.children.forEach(item => {
        if (item.layer && item.label !== 'No Output') {
            map.removeLayer(item.layer);
        }
    });

    stationOutputTree.children = [{
        label: 'No Stations',
        layer: L.layerGroup([]),
        radioGroup: 'stationOutput'
    }];

    updateOverlayTree();
    updateCenterControl();
    updateClearControl();
}

function clearUnknownOutputTree() 
{
    unknownOutputTree.children.forEach(item => {
        if (item.layer && item.label !== 'No Output') {
            map.removeLayer(item.layer);
        }
    });

    unknownOutputTree.children = [{
        label: 'No Output',
        layer: L.layerGroup([]),
        radioGroup: 'unknownOutput'
    }];

    updateOverlayTree();
    updateCenterControl();
    updateClearControl();
}
