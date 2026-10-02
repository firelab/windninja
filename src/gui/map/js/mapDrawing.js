
let drawDEMCheck = false;

// Script functions
function sendBoundingBox(layer) {
    const bounds = layer.getBounds();
    const bboxData = {
        north: bounds.getNorth(),
        south: bounds.getSouth(),
        east: bounds.getEast(),
        west:bounds.getWest()
    };

    if (window.bridge?.receiveBoundingBox) {
        window.bridge.receiveBoundingBox(JSON.stringify(bboxData));
    }
}

function drawBoundingBox(north, south, east, west) {
    clearBoundingBoxLayer();
    const bounds = [[south, west], [north, east]];
    const rectangle = L.rectangle(bounds, {
            bubblingMouseEvents: false,
        }).addTo(boundingBoxLayer);
    rectangle.editing.enable();
    map.fitBounds(rectangle.getBounds());
    rectangle.on('edit', () => {
            sendBoundingBox(rectangle);
            map.fitBounds(rectangle.getBounds()); 
    });
    updateCenterControl();
    updateClearControl();
}

function drawDEM(DEMCorners) {
    clearDEMLayer();

    const rectangleCorners = [
        [DEMCorners[1], DEMCorners[0]], // NE
        [DEMCorners[3], DEMCorners[2]], // SE
        [DEMCorners[5], DEMCorners[4]], // SW
        [DEMCorners[7], DEMCorners[6]], // NW
        [DEMCorners[1], DEMCorners[0]]  // NE
    ];

    const rectangle = L.polygon(rectangleCorners, {
        color: 'black',
        weight: 2,
        fill: false,
        pane: 'demPane'
    }).addTo(demLayer);

    setTimeout(() => {
        map.fitBounds(rectangle.getBounds());
    }, 10);

    updateCenterControl();
    updateOverlayTree();
}

function drawBoundingBoxAroundDEM(bufferMiles) {
    clearBoundingBoxLayer();

    const demBounds = demLayer.getBounds();

    const center = demBounds.getCenter();

    const milesPerDegreeLatitude = 69.0;
    const milesPerDegreeLongitude =
        69.0 * Math.cos(center.lat * Math.PI / 180.0);

    const latBuffer = bufferMiles / milesPerDegreeLatitude;
    const lngBuffer = bufferMiles / milesPerDegreeLongitude;

    const bufferedBounds = L.latLngBounds(
        [
            demBounds.getSouth() - latBuffer,
            demBounds.getWest() - lngBuffer
        ],
        [
            demBounds.getNorth() + latBuffer,
            demBounds.getEast() + lngBuffer
        ]
    );

    const rectangle = L.rectangle(bufferedBounds).addTo(boundingBoxLayer);

    map.fitBounds(rectangle.getBounds());

    updateCenterControl();
    updateClearControl();
}

function startRectangleDrawing() {
    clearBoundingBoxLayer();
    rectangleDrawer.enable();
}

function stopRectangleDrawing() {
    clearBoundingBoxLayer();            
    rectangleDrawer.disable();
}

map.on('draw:created', ({ layer }) => {
    clearBoundingBoxLayer();
    boundingBoxLayer.addLayer(layer);
    layer.editing.enable();
    sendBoundingBox(layer);
    layer.on('edit', () => 
    {
        sendBoundingBox(layer);
        map.fitBounds(layer.getBounds());
    });
    map.fitBounds(layer.getBounds());
    updateCenterControl();
});

function clearBoundingBoxLayer() 
{
    boundingBoxLayer.eachLayer(layer => {
        if (layer.editing) {
            layer.editing.disable();
        }
    });

    boundingBoxLayer.clearLayers();

    const pane = map.getPane('boundingBoxPane');

    if (pane) {
        pane.innerHTML = '';
    }

    updateCenterControl();
    updateClearControl();
}

function clearDEMLayer() {
    demLayer.clearLayers();
}