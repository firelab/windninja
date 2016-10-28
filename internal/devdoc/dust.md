---
layout: internal
permalink: /internal/dust/
---
<h1 class="post-title" itemprop="name headline" style="color:white;">Dust Emissions Model</h1>

---

<br>

# PM Emissions Model

PM10 flux is modeled with an empirical formula parameterized for burned soils based on post-fire PM flux measurements and wind tunnel experiments:

`flux = K x rho / g x u*[u*^2 - u*t^2]`

`u*` = friction velocity

`u*t` = threshold friction velocity = 0.22 m/s

`K` = PM10 release rate = 0.0007 1/m

The threshold friction velocity and PM10 release rate were determined from measurements on burned soils in high desert sagebrush landscapes. Friction velocity is calculated from the final flow field based on the shear stress normal to the ground surface. PM2.5 is assumed to be 60% of the PM10 fraction based on post-fire field measurements. See [here](http://forest.moscowfsl.wsu.edu/cgi-bin/engr/library/searchpub.pl?pub=2013f) for details regarding post-fire field experiments.

## Using the emissions model

`EMISSIONS` and `FRICTION_VELOCITY` must be turned on.

### Input

The only required input is a shapefile containing a single fire perimeter. If an elevation file is not specified, one will be automatically downloaded.

### Output

An optional multiband GeoTIFF output option (`write_multiband_geotiff_ouptput`) is available for the emissions output. If this is selected a multiband GeoTIFF will be written for the speed, direction, and dust grids. The name of the GeoTIFF output files can be specified with `geotiff_file`. If the specified .tif files do not exist, they are created during the first emissions run. If the files already exist, a new band is appended to each .tif file. The date and time of the first time step in the emissions simulation are stored in the `TIFFTAG_DATETIME` metadata of the .tif file and each band has a `DT` metadata item which indicates hours since the first time step.

### Options

Friction velocity can be calculated with one of two methods: `logProfile` or `shearStress`. The default method is `logProfile`. The method can be changed by specifying `friction_velocity_calculation_method`.
