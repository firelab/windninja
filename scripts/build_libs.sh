#!/bin/bash

sed -i 's/$USER-$WM/$WM/g' /opt/openfoam8/etc/bashrc 
source /opt/openfoam8/etc/bashrc 
mkdir -p $FOAM_RUN/../applications 
cp -r /opt/src/windninja/src/ninjafoam/* $FOAM_RUN/../applications 
cd $FOAM_RUN/../applications 
wmake libso 
cd utility/applyInit 
wmake
ldconfig

