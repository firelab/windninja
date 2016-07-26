---
layout: internal
---
## Environment variables

Set the environment variable `WRITE_FOAM_FILES` to end the run at various locations in a NinjaFOAM simulation.

`WRITE_FOAM_FILES = 0` generates the STL and writes the basic OpenFOAM case directory

`WRITE_FOAM_FILES = 1` writes the mesh dict files (but does not generate the mesh)

`WRITE_FOAM_FILES = 2` generates the mesh

## OpenFOAM work flow
    surfaceTransformPoints -translate "(0 0 10)" constant/triSurface/butte.stl constant/triSurface/butte_out.stl
    blockMesh
    decomposePar -force
    mpiexec -np 4 moveDynamicMesh -parallel
    reconstructPar -latestTime
    topoSet -dict system/topoSetDict
    refineMesh -dict system/refineMeshDict
    checkMesh -latestTime
    renumberMesh -latestTime -overwrite
    applyInit
    decomposePar -force
    mpiexec -np 4 simpleFoam -parallel
    reconstructPar
    sample -latestTime
***

## Convert output
To convert OpenFOAM output to WindNinja output formats (.kmz and .asc):

    ../../output_converter/convert_output postProcessing/surfaces/951/U_triSurfaceSampling.raw ninja_output ../butte.tif

Set `BUILD_CONVERT_OUTPUT = ON` in CMake to build the output converter.

## Sample the 3D output

### Sample a surface
Generate a surface to sample with `surfaceTransformPoints` (see above). Then edit `system/sampleDict` if necessary and run `sample -latestTime`.

### Sample a transect
Copy `system/sampleDict_transect` to `system/sampleDict` and edit `$axis$`, `$xstart$`, `$ystart$`, `$zstart$`, `$xend$`, `$yend$`, `$zend$`, and `$nPoints$`. Then run `sample -latestTime`.
