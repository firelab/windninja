---
layout: page
title: NinjaFOAM Overview
permalink: /hyde/foamveriew/
---

This page documents the individual steps in a NinjaFOAM simulation. These steps are executed in `ninjafoam::simulate_wind()`. Output from each OpenFOAM call is written to a log file (e.g., `log.checkMesh`) in the temporary directory.

1. A temporary case directory is created and the OpenFOAM case files are written to the temporary directory. The OpenFOAM files are copied from template files located in `WINDNINJA_DATA/ninjafoam.zip`. 

2. The DEM is converted to binary STL format and written to `constant/triSurface`.

3. Call `surfaceTransformPoints`. This creates an STL of the output surface. This is a surface shifted to the user-specified output wind height. It is written to `constant/triSurface/<demBaseName>_out.stl`.

4. Call `surfaceCheck`. This performs a check on the generated STL and writes a file called `log.json` which is later used in the `blockMeshDict` file writing if a DEM is not available (e.g., runs initiated with an STL).

5. Write [`blockMeshDict`](https://github.com/firelab/windninja/wiki/blockMeshDict). During this step, `xmin`, `xmax`, `ymin`, `ymax`, `zmin`, `zmax` are read/calculated from the DEM unless the run was initiated with an STL file, in which case the necessary terrain information is read from `log.json`. This is the bounding box for `blockMesh`.

6. Run `blockMesh`.

7. Run `moveDynamicMesh`, in parallel if more than once processor is available.

8. Refine the near-surface layer with [`refineMesh`](https://github.com/firelab/windninja/wiki/refineMesh). This refines the near-surface cells in all directions.

9. Call `checkMesh` to check the current cell count. Repeat 8-9 until cell count meets our threshold.

10. Call `renumberMesh`.

11. Call `applyInit`. This applies the initial conditions.  

12. Run `simpleFoam`, in parallel if more than one processor is available. 

13. Call `sample` to sample the velocity field at the user-specified output height. The sampling is controlled by `sampleDict`. The sampling is performed on the output surface `<demBaseName>_out.stl` created at the beginning of the simulation. There are several types of interpolation schemes available: `cell`, `cellPoint`, `cellPointFace`, etc.

14. Convert the raw xyz output to the requested WindNinja formats.

15. Remove the temporary directory and all OpenFOAM case files unless VTK output was requested.