// Copyright 2017 Kyle Shannon.  All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

package main

import (
	"html/template"
)

var templates *template.Template

func init() {
	templates = template.Must(template.New("templates").Parse(templateData))
}

const templateData = `
{{define "geogreport"}}
<!DOCTYPE html>
<html>
 <body>
 <table>
 <tr><th>Country</th><th>Region</th><th>Visits</th></tr>
  {{range $idx, $elem := . -}}
 <tr>
  <td>{{$elem.Country}}</td>
  <td>{{$elem.Region}}</td>
  <td>{{$elem.Visits}}</td>
 </tr>
 {{end -}}
 </table>
 </body>
</html>
{{end}}

{{define "map"}}
<!DOCTYPE html>
<html>
 <head>
  <script type="text/javascript" src="https://www.gstatic.com/charts/loader.js"></script>
  <script type="text/javascript">
   google.charts.load('upcoming', {'packages':['geochart']});
   google.charts.setOnLoadCallback(drawRegionsMap);

   function drawRegionsMap() {
    var data = google.visualization.arrayToDataTable([
     ['Country', 'Count'],
     {{range $idx, $elem := . -}}
     ['{{$elem.Country}}',{{$elem.Visits}}],
     {{end -}}
     ]);
     var options = {
      colorAxis: {colors: ['#888af3', '#000028']},
      backgroundColor: '#e6e6e6',
      datalessRegionColor: 'white',
      defaultColor: 'white',
      legend : {textStyle: {color: 'black', fontSize: 16}}
     };
     var chart = new google.visualization.GeoChart(document.getElementById('regions_div'));
      chart.draw(data, options);
    }
 </script>
 </head>
 <body>
  <div id="regions_div" style="width: 900px; height: 500px;"></div>
 </body>
</html>
{{end}}
`

// google.visualization.events.addListener(chart, 'ready', function () {
//   regions_div.innerHTML = '<img src="' + chart.getImageURI() + '">';
// });
