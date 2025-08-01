# Copyright (c) Kyle Shannon
# Distributed under the terms of the Modified BSD License.
#
# Note, to use this X11 application from within a container, you need to
# allow X11 connections from other hosts as well as extra 
# command line options on the docker (or podman) run line. To access data
# from outside the narrow confines of the WindNinja container, you'll also
# need to specify a volume mount. By default, this container uses "/data" as 
# a working directory. For instance, as an unpriviledged user 
# using podman: 
# 
#     mkdir $HOME/MyWindNinjaRuns
#     xhost +
#     podman run -ti --rm \
#                -e DISPLAY -v /tmp/.X11-unix:/tmp/.X11-unix \
#                -v $HOME/MyWindNinjaRuns:/data:z \
#                --env="QT_X11_NO_MITSHM=1" \
#                --security-opt label=type:container_runtime_t \
#                windninja:3.11.1 
#



### Update environment variables for OpenMPI
## If running docker / singulairty container on mulitple cores make sure to use these environmental varibale before trying to run Windninja 
# OPENMPI_VERSION=4.0.4
# export MPI_DIR=/opt/openmpi-${OPENMPI_VERSION}
# export MPI_BIN=$MPI_DIR/bin
# export MPI_LIB=$MPI_DIR/lib
# export MPI_INC=$MPI_DIR/include
# export PATH=$MPI_BIN:$PATH
# export LD_LIBRARY_PATH=$MPI_LIB:$LD_LIBRARY_PATH


# export CPL_DEBUG=NINJAFOAM
# source /opt/openfoam8/etc/bashrc
# export OMPI_ALLOW_RUN_AS_ROOT=1
# export OMPI_ALLOW_RUN_AS_ROOT_CONFIRM=1
# export FOAM_USER_LIBBIN=/usr/local/lib/


FROM ubuntu:20.04
USER root
ADD . /opt/src/windninja/
SHELL [ "/usr/bin/bash", "-c" ]
ENV DEBIAN_FRONTEND noninteractive
ENV WM_PROJECT_INST_DIR /opt
ENV WINDNINJA_DATA=/opt/src/windninja/data
RUN dpkg-reconfigure debconf --frontend=noninteractive && \
    apt-get update &&  \
    apt-get install -y wget gnupg2 cmake git apt-transport-https ca-certificates \
                       software-properties-common sudo build-essential \
                       pkg-config g++ libboost-program-options-dev \
                       libboost-date-time-dev libboost-test-dev python3-pip && \
    cd /opt/src && \
    DEBIAN_FRONTEND=noninteractive /opt/src/windninja/scripts/build_deps_docker.sh && \
    rm -rf /var/lib/apt/lists

RUN mkdir -p /opt/src/windninja/build && \
    cd  /opt/src/windninja/build && \

    # Building the windninja with different funationalites
    cmake \
    # Just a flag to ignore some of the common warnings (required)
    -D SUPRESS_WARNINGS=ON \
    # This flag  is responsible for Momentum solver (required)
    -D NINJAFOAM=ON \
    # This Flag is required to allow the WindNinja to download DEM Files (optional)
    -D BUILD_FETCH_DEM=ON \
    #This Flag is required to build the slope aspect grid (optional)
    -D BUILD_SLOPE_ASPECT_GRID=ON \
    # This Flag is required to build the flow seperation grid utility (optional)
    -D BUILD_FLOW_SEPARATION_GRID=ON \
    # User can add their specific flag from the cmake here similarly from the above example
    .. && \
    make -j12 && \
    make install && \
    ldconfig && \
    cd /opt/src/windninja 


# This segment is responsible for openfoam8
RUN source /opt/openfoam8/etc/bashrc &&\
mkdir -p $FOAM_RUN/../applications && \
cp -r /opt/src/windninja/src/ninjafoam/8/* $FOAM_RUN/../applications && \
cd $FOAM_RUN/../applications/ && \
sed -i "s|export WM_PROJECT_INST_DIR=|export WM_PROJECT_INST_DIR=/opt|g" /opt/openfoam8/etc/bashrc && \
sed -i "s|export WM_PROJECT_DIR=\$WM_PROJECT_INST_DIR/openfoam8|export WM_PROJECT_DIR=/opt/openfoam8|g" /opt/openfoam8/etc/bashrc && \
. /opt/openfoam8/etc/bashrc && \
wmake libso && \
cd utility/applyInit && \
wmake  &&\


cp $FOAM_RUN/../platforms/linux64GccDPInt32Opt/lib/libWindNinja.so /opt/openfoam8/platforms/linux64GccDPInt32Opt/lib/ &&\
cp $FOAM_RUN/../platforms/linux64GccDPInt32Opt/bin/applyInit /opt/openfoam8/platforms/linux64GccDPInt32Opt/bin/ &&\
chmod 644 /opt/openfoam8/platforms/linux64GccDPInt32Opt/lib/libWindNinja.so &&\
chmod 755 /opt/openfoam8/platforms/linux64GccDPInt32Opt/bin/applyInit 

# To create a Singularity image from this Dockerfile, run the following commands:
# 1. Build the Docker image
#    docker build -t windninja:latest .
# 2. Convert the Docker image to a Singularity image
#    singularity build windninja_latest.sif docker-daemon://windninja:latest

