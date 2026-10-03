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

# Setup environment
FROM ubuntu:24.04
COPY . /src/wind/windninja/
SHELL [ "/usr/bin/bash", "-c" ]
ENV DEBIAN_FRONTEND=noninteractive
ENV WM_PROJECT_INST_DIR=/opt
ENV WINDNINJA_DATA=/src/wind/windninja/data

# Install dependencies
RUN apt-get update && \
    apt-get install -y \
        wget \
        gnupg2 \
        cmake \
        git \
        ca-certificates \
        software-properties-common \
        build-essential && \
    cd /src && \
    /src/wind/windninja/scripts/build_deps_docker.sh && \
    rm -rf /var/lib/apt/lists/*

# Configure and build WindNinja
RUN mkdir -p /src/wind/build && \
    cd /src/wind/build && \
    # Building the windninja with different functionalities
    cmake \
    # Suppresses common build warnings (required).
    -D SUPRESS_WARNINGS=ON \
    # Turns on the Momentum solver. Dependent on OpenFOAM (required).
    -D NINJAFOAM=ON \
    # Turns off the GUI. Not needed for Docker images using only the CLI (optional).
    -D NINJA_GUI=OFF \
    # User can add their specific flag from the cmake here similarly from the above example
    /src/wind/windninja && \
    make -j12 && \
    make install && \
    ldconfig

# Build OpenFOAM 11 libraries and executables
RUN source /opt/openfoam11/etc/bashrc && \
    mkdir -p $FOAM_RUN/../applications && \
    cp -r /src/wind/windninja/src/ninjafoam/11/* $FOAM_RUN/../applications && \
    cd $FOAM_RUN/../applications/ && \
    wmake libso && \
    cd utility/applyInit && \
    wmake && \
    # Copy custom libraries and binaries from $FOAM_USER_LIBBIN and
    # $FOAM_USER_APPBIN to $FOAM_LIBBIN and $FOAM_APPBIN.
    # Required for OpenFOAM to work with Singularity, because Singularity
    # drops the home directory where these files are normally located.
    cp $FOAM_RUN/../platforms/linux64GccDPInt32Opt/lib/libWindNinja.so \
       /opt/openfoam11/platforms/linux64GccDPInt32Opt/lib/ && \
    cp $FOAM_RUN/../platforms/linux64GccDPInt32Opt/bin/applyInit \
       /opt/openfoam11/platforms/linux64GccDPInt32Opt/bin/ && \
    chmod 644 /opt/openfoam11/platforms/linux64GccDPInt32Opt/lib/libWindNinja.so && \
    chmod 755 /opt/openfoam11/platforms/linux64GccDPInt32Opt/bin/applyInit