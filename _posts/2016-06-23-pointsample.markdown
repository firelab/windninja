---
layout: post
title:  "Point Sampling"
color:  blue
width:   3
height:  1
date:   2016-06-23 11:31:49 +0200
categories: main
---
It is possible to extract point predictions from WindNinja using the command line interface. A csv file needs to be supplied with the requested point locations. The input csv is specified with `input_points_file`. The output filename can be optionally specified with `output_points_file`. The input points file must have the following format:

    WGS84
    point_1,46.920830,-114.092500,3.28
    point_2,47.0654891,-114.29849,10
    ...

The first line is the datum that the points are in. The point lat/lon have to be in decimal degrees. The lines for each point follow the convention name,latitude,longitude,height where name is selected by the user, and height is meters above the ground.

The output files look like:

    ID,lat,lon,height,datetime,u,v,w,wx_u,wx_v
    point_1,46.920830,-114.092500,3.280000,2014-Jun-05 12:00:00 MDT,6.444286,-0.290651,0.026691,6.489773,-0.187149
    point_2,47.0654891,-114.29849,10.000000,2014-Jun-05 12:00:00 MDT,7.4443586,-0.234551,0.0345691,6.4435773,-0.145149
    ...

If the run is not a weather model, datetime, wx_u, and wx_v are omitted.
