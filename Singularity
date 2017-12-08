BootStrap: debootstrap
OSVersion: xenial
MirrorURL: http://us.archive.ubuntu.com/ubuntu

%post
apt-get update -qq && \
apt-get install -y -qq wget apt-transport-https software-properties-common && \
apt-add-repository universe
add-apt-repository -y ppa:ubuntugis/ubuntugis-unstable && \
add-apt-repository http://dl.openfoam.org/ubuntu
sh -c "wget -O - http://dl.openfoam.org/gpg.key | apt-key add -"
apt-get update && \
apt-get install -y \
    cmake \
    git \
    libgdal-dev \
    qt4-dev-tools \
    libqtwebkit-dev \
    libboost-program-options-dev \
    libboost-date-time-dev \
    libboost-test-dev \
    openfoam4 && \
echo "source /opt/openfoam4/etc/bashrc" >> $SINGULARITY_ENVIRONMENT

%appinstall windninja
rm -rf windninja && \
git clone http://github.com/firelab/windninja && \
cd windninja && \
mkdir build && \
cd build && \
cmake -D SUPRESS_WARNINGS=ON ../ && \
make -j 4 && \
make install
mv /usr/local/lib/libninja.so /.singularity.d/libs/

