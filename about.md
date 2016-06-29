---
layout: page
title: About
permalink: /about/
---

Wind is one of the most influential environmental factors affecting wildland fire behavior. The complex terrain of fire-prone landscapes causes local changes in wind speed and direction that are not predicted well by standard weather models or expert judgment. WindNinja was developed to help fire managers predict these winds.

![WindNinja](http://firelab.org/sites/default/files/images/projects/WindNinja_logo_180dpi-md.jpg)

WindNinja is a computer program that computes spatially varying wind fields for wildland fire and other applications requiring high resolution wind prediction in complex terrain. It was developed to be used by emergency responders within their typical operational constraints of fast simulation times (seconds), low CPU requirements (single processor laptops), and low technical expertise. WindNinja can be run in three different modes depending on the application and available inputs. The first mode is a forecast, where WindNinja uses coarser resolution mesoscale weather model data from the US National Weather Service to forecast wind at future times. The second mode uses one or more surface wind measurements to build a wind field for the area. The third mode uses a user-specified average surface wind speed and direction. Other required inputs for a WindNinja simulation include elevation data for the modeling area (which WindNinja can obtain from Internet sources), date and time, and dominant vegetation type. A diurnal slope flow model and non-neutral atmospheric stability model can be turned on or off. Outputs of the model are ASCII Raster grids of wind speed and direction (for use in spatial fire behavior models such as FARSITE and FlamMap), a GIS shapefile (for plotting wind vectors in GIS programs), and a .kmz file (for viewing in Google Earth). WindNinja is typically run on domain sizes up to 50 kilometers by 50 kilometers and at resolutions of around 100 meters. WindNinja runs on 32- and 64-bit versions of Windows XP and later operating systems ([installers can be accessed on the WindNinja Software page](http://www.firelab.org/document/windninja-software)). WindNinja can also be run on versions of Linux, however building from source code is required. [See Building WindNinja on Linux](http://firelab.github.io/windninja/main/2016/06/23/buildlinux.html)
[Instructions Also Available on Github](https://github.com/firelab/windninja/wiki/Building-WindNinja-on-Linux)


#### Other Links


[Link to main Github Website](https://github.com/firelab/windninja)


[Link to FireLab Website](http://www.firelab.org/project/windninja)



### For Incident Support


see [FAQ](http://firelab.github.io/windninja/about/),


See the [Change Log](http://firelab.github.io/windninja/log/) for release notes,


[Change Log on Github](https://github.com/firelab/windninja/blob/master/NEWS.md),


Contact the WindNinja Support Team at [wind.ninja.support@gmail.com](mailto:wind.ninja.support@gmail.com)



