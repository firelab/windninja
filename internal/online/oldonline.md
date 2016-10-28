---
layout: internal
permalink: /oldline/
---

<h1 class="post-title" itemprop="name headline" style="color:white;">Sample WindNinja Run</h1>
<!-- <h1 style="color:white;">Sample WindNinja Run</h1> -->
<br>

<html>
<head>
<style>
body
{
color:#00C6FF
}
</style>
<script
src="http://maps.googleapis.com/maps/api/js?&key=AIzaSyBL2JbHu9cN3mSvKDvi_VPDQJHPwT2_w8M">
</script>
<script>
var myCenter=new google.maps.LatLng(46.9163056,-114.0905556);

function initialize()
{
var mapProp = {
  center:myCenter,
  zoom:11,
  mapTypeId:google.maps.MapTypeId.TERRAIN,
  styles: [{"featureType":"water","elementType":"geometry","stylers":[{"visibility":"on"},{"color":"#aee2e0"}]},{"featureType":"landscape","elementType":"geometry.fill","stylers":[{"color":"#abce83"}]},{"featureType":"poi","elementType":"geometry.fill","stylers":[{"color":"#769E72"}]},{"featureType":"poi","elementType":"labels.text.fill","stylers":[{"color":"#7B8758"}]},{"featureType":"poi","elementType":"labels.text.stroke","stylers":[{"color":"#EBF4A4"}]},{"featureType":"poi.park","elementType":"geometry","stylers":[{"visibility":"simplified"},{"color":"#8dab68"}]},{"featureType":"road","elementType":"geometry.fill","stylers":[{"visibility":"simplified"}]},{"featureType":"road","elementType":"labels.text.fill","stylers":[{"color":"#5B5B3F"}]},{"featureType":"road","elementType":"labels.text.stroke","stylers":[{"color":"#ABCE83"}]},{"featureType":"road","elementType":"labels.icon","stylers":[{"visibility":"off"}]},{"featureType":"road.local","elementType":"geometry","stylers":[{"color":"#A4C67D"}]},{"featureType":"road.arterial","elementType":"geometry","stylers":[{"color":"#9BBF72"}]},{"featureType":"road.highway","elementType":"geometry","stylers":[{"color":"#EBF4A4"}]},{"featureType":"transit","stylers":[{"visibility":"off"}]},{"featureType":"administrative","elementType":"geometry.stroke","stylers":[{"visibility":"on"},{"color":"#87ae79"}]},{"featureType":"administrative","elementType":"geometry.fill","stylers":[{"color":"#7f2200"},{"visibility":"off"}]},{"featureType":"administrative","elementType":"labels.text.stroke","stylers":[{"color":"#ffffff"},{"visibility":"on"},{"weight":4.1}]},{"featureType":"administrative","elementType":"labels.text.fill","stylers":[{"color":"#495421"}]},{"featureType":"administrative.neighborhood","elementType":"labels","stylers":[{"visibility":"off"}]}]
  };

var map=new google.maps.Map(document.getElementById("googleMap"),mapProp);

  var ctaLayer = new google.maps.KmlLayer({
    url: 'http://windninja.wfmrda.org/ninjadata/kmso.kml',
    map: map
  });
  var ctaLayer = new google.maps.KmlLayer({
    url: 'http://windninja.wfmrda.org/ninjadata/UV.kml',
    map: map
  });

}

google.maps.event.addDomListener(window, 'load', initialize);
</script>
</head>

<body>
<div id="googleMap" style="width:900px;height:480px;"></div>
</body>
</html>


<div class="col col-8">
<h1>About</h1>
<p> This simulation is automatic and will update at regular intervals. This simulation runs using the latest weather data from Missoula International Airport (KMSO). Two RAWS stations, TR266 and PNTM8 are used as validation points to check the
accuracy of the simulation</p>

<iframe src="http://windninja.wfmrda.org/ninjadata/log.txt" style="background: #FFFFFF;" height="75"  width="600"></iframe>
</div>


# Key

![UV](http://windninja.wfmrda.org/ninjadata/UV.bmp)






# Validation

These continuously updating plots compare wind speed data from RAWS stations to the nearest simulation point.

<html>
    <head>
        <title>Sample</title>

        <script type="text/javascript">
            function showImage(smSrc, lgSrc) {
                document.getElementById('largeImg').src = smSrc;
                showLargeImagePanel();
                unselectAll();
                setTimeout(function() {
                    document.getElementById('largeImg').src = lgSrc;
                }, 1)
            }
            function showLargeImagePanel() {
                document.getElementById('largeImgPanel').style.display = 'block';
            }
            function unselectAll() {
                if(document.selection)
                    document.selection.empty();
                if(window.getSelection)
                    window.getSelection().removeAllRanges();
            }
        </script>

        <style type="text/css">
            #largeImgPanel {
                text-align: center;
                display: none;
                position: fixed;
                z-index: 100;
                top: 0; left: 0; width: 90%; height: 90%;
                background-color: background-color;
            }
        </style>
    </head>

    <body>
        <p>Click on any image thumbnail to enlarge. Click again to hide:</p>

        <img src="http://windninja.wfmrda.org/ninjadata/valTR.png" style="width:200px;height=200px;cursor:pointer"
             onclick="showImage(this.src, 'http://windninja.wfmrda.org/ninjadata/valTR.png');" />
        <img src="http://windninja.wfmrda.org/ninjadata/valPN.png" style="width:200px;height=200px;cursor:pointer"
             onclick="showImage(this.src, 'http://windninja.wfmrda.org/ninjadata/valPN.png');" />

        <div id="largeImgPanel" onclick="this.style.display='none'">
            <img id="largeImg"
                 style="height:100%; margin:0; padding:0;" />
        </div>
    </body>
</html>

<br>



<a href="http://synopticlabs.org/api/mesonet/"> <img src="http://firelab.github.io/windninja/assets/meso-api-logo-light.png" style="width:146px;height:25px;"> </a>



[Source Code](https://github.com/tfinney9/WNkml/)
