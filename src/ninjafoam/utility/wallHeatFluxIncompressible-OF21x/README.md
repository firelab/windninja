wallHeatFluxIncompressible
==========================

Utility originally by the forum member Eelco van Vliet, for OpenFOAM 2.1.0: http://www.cfd-online.com/Forums/openfoam-post-processing/101972-wallheatflux-utility-incompressible-case.html#post368330

It's the one provided in the branch `OF21x`.

This git repository further adapts the utility for OpenFOAM 2.2.x and 2.3.x, done by Bruno Santos (wyldckat@github working at [blueCAPE Lda](http://www.bluecape.com.pt)).

WARNING: If you're planning on using OpenFOAM 2.2.0, then use the branch `OF21x`.

NOTE: In the Foundation's OpenFOAM-dev there is now a more general implementation that was introduced in the commits related to this bug report: http://www.openfoam.org/mantisbt/view.php?id=1856 - this means that the Boussinesq implementation is now part of the compressible heat transfer solvers, therefore no longer depending on explicit solvers dedicated to the Boussinesq implementation.


License
=======

The same as OpenFOAM(R), namely GNU GPL v3. For more information, see the file LICENSE.


Building on OpenFOAM 2.3.x, 2.2.x, 2.1.x
========================================

Using Git
---------

  1. Go to your user folder:

     ```
     mkdir -p $FOAM_RUN
     cd $FOAM_RUN/..
     ```

  2. Clone the repository and go into the cloned repository:

     ```
     git clone https://github.com/wyldckat/wallHeatFluxIncompressible.git
     cd wallHeatFluxIncompressible
     ```

  3. Checkout the repository respective to the version of OpenFOAM you are using:

   * OpenFOAM 2.3.x:

     ```
     git checkout OF23x
     ```

   * OpenFOAM 2.2.x:

     ```
     git checkout OF22x
     ```

   * OpenFOAM 2.1.x:

     ```
     git checkout OF21x
     ```

  4. Build `wallHeatFluxIncompressible` by running:

     ```
     wmake
     ```


Using Zip
---------

  1. Go to your user folder:

     ```
     mkdir -p $FOAM_RUN
     cd $FOAM_RUN/..
     ```

  2. Get the Zip file for the repository respective to the version of OpenFOAM you are using:

   * OpenFOAM 2.3.x:

     ```
     wget https://github.com/wyldckat/wallHeatFluxIncompressible/archive/OF23x.zip
     ```

   * OpenFOAM 2.2.x:

     ```
     wget https://github.com/wyldckat/wallHeatFluxIncompressible/archive/OF22x.zip
     ```

   * OpenFOAM 2.1.x:

     ```
     wget https://github.com/wyldckat/wallHeatFluxIncompressible/archive/OF21x.zip
     ```

  3. Unzip the respective file and go into the respective folder, for example:

     ```
     unzip OF23x.zip
     cd wallHeatFluxIncompressible-OF23x
     ```
     
  4. Build `wallHeatFluxIncompressible` by running:

     ```
     wmake
     ```
