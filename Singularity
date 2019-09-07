Bootstrap: library
From: alpine

%post

  apk add \
    --repository http://dl-cdn.alpinelinux.org/alpine/v3.8/main \
    qt-dev

  apk add \
    --repository http://dl-cdn.alpinelinux.org/alpine/edge/testing \
    gdal-dev \
    netcdf-dev

  apk add \
    alpine-sdk \
    boost-dev \
    cmake \
    curl

  cd /opt
  git clone --depth=1 https://github.com/firelab/windninja
  cd windninja
  mkdir build
  cd build
  cmake -D..
  make -j 6

%test
  export WINDNINJA_DATA=/opt/windninja/data
  cd /opt/windninja/build
  ctest
