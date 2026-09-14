# THIS SOFTWARE WAS DEVELOPED AT THE ROCKY MOUNTAIN RESEARCH STATION (RMRS)
# MISSOULA FIRE SCIENCES LABORATORY BY EMPLOYEES OF THE FEDERAL GOVERNMENT
# IN THE COURSE OF THEIR OFFICIAL DUTIES. PURSUANT TO TITLE 17 SECTION 105
# OF THE UNITED STATES CODE, THIS SOFTWARE IS NOT SUBJECT TO COPYRIGHT
# PROTECTION AND IS IN THE PUBLIC DOMAIN. RMRS MISSOULA FIRE SCIENCES
# LABORATORY ASSUMES NO RESPONSIBILITY WHATSOEVER FOR ITS USE BY OTHER
# PARTIES,  AND MAKES NO GUARANTEES, EXPRESSED OR IMPLIED, ABOUT ITS QUALITY,
# RELIABILITY, OR ANY OTHER CHARACTERISTIC.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
# OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
# DEALINGS IN THE SOFTWARE.

# Instructions/Documentation
# https://github.com/firelab/windninja/wiki/Building-the-WindNinja-Docker-Image
# https://github.com/firelab/windninja/wiki/Running-the-WindNinja-Docker-Image-in-an-HPC-Environment
    
# Update environment variables for OpenMPI
# If running docker / singulairty container on mulitple cores make sure to use these environmental varibale before trying to run Windninja 
# OPENMPI_VERSION=4.0.4
# export MPI_DIR=/opt/openmpi-${OPENMPI_VERSION}
# export MPI_BIN=$MPI_DIR/bin
# export MPI_LIB=$MPI_DIR/lib
# export MPI_INC=$MPI_DIR/include
# export PATH=$MPI_BIN:$PATH
# export LD_LIBRARY_PATH=$MPI_LIB:$LD_LIBRARY_PATH

FROM ubuntu:24.04
USER root
ADD . /src/wind/windninja/
SHELL [ "/usr/bin/bash", "-c" ]
ENV DEBIAN_FRONTEND noninteractive
ENV WM_PROJECT_INST_DIR /opt
ENV WINDNINJA_DATA=/src/wind/windninja/data

RUN dpkg-reconfigure debconf --frontend=noninteractive && \
    apt-get update && \
    apt-get install -y wget gnupg2 cmake git apt-transport-https ca-certificates \
                       software-properties-common sudo build-essential \
                       pkg-config g++ libboost-program-options-dev \
                       libboost-date-time-dev libboost-test-dev python3-pip && \
    cd /src && \
    DEBIAN_FRONTEND=noninteractive /src/wind/windninja/scripts/build_deps_ubuntu_2404.sh && \
    rm -rf /var/lib/apt/lists

RUN mkdir -p /src/wind/build && \
    cd /src/wind/build && \
    # Building the windninja with different funationalites
    cmake \
    # Supresses common build wanrings (required).
    -D SUPRESS_WARNINGS=ON \
    # Turns on the Momemtum solver. Dependent on OpenFOAM (required).
    -D NINJAFOAM=ON \
    # Turns off the GUI. Not needed for Docker images using only the CLI. 
    -D NINJA_GUI=OFF \
    # User can add their specific flag from the cmake here similarly from the above example
    .. && \
    make -j12 && \
    make install && \
    ldconfig && \
    cd /src/wind/windninja


# This segment is responsible for openfoam11
RUN source /opt/openfoam11/etc/bashrc && \
    mkdir -p $FOAM_RUN/../applications && \
    cp -r /src/wind/windninja/src/ninjafoam/11/* $FOAM_RUN/../applications && \
    cd $FOAM_RUN/../applications/ && \
    sed -i "s|export WM_PROJECT_INST_DIR=|export WM_PROJECT_INST_DIR=/opt|g" /opt/openfoam11/etc/bashrc && \
    sed -i "s|export WM_PROJECT_DIR=\$WM_PROJECT_INST_DIR/openfoam11|export WM_PROJECT_DIR=/opt/openfoam11|g" /opt/openfoam11/etc/bashrc && \
    . /opt/openfoam11/etc/bashrc && \
    wmake libso && \
    cd utility/applyInit && \
    wmake && \
    # copy custom libraries and binaries from $FOAM_USER_LIBBIN and $FOAM_USER_APPBIN to $FOAM_LIBBIN and $FOAM_APPBIN.
    # required for OpenFOAM to work with Singularity, because Singularity drops the home directory where these files are normally located.
    cp $FOAM_RUN/../platforms/linux64GccDPInt32Opt/lib/libWindNinja.so /opt/openfoam11/platforms/linux64GccDPInt32Opt/lib/ && \
    cp $FOAM_RUN/../platforms/linux64GccDPInt32Opt/bin/applyInit /opt/openfoam11/platforms/linux64GccDPInt32Opt/bin/ && \
    chmod 644 /opt/openfoam11/platforms/linux64GccDPInt32Opt/lib/libWindNinja.so && \
    chmod 755 /opt/openfoam11/platforms/linux64GccDPInt32Opt/bin/applyInit