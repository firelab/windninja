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
#                windninja:3.7.5 
#
FROM ubuntu:20.04

MAINTAINER Kyle Shannon <kyle@pobox.com>

USER root
ENV DEBIAN_FRONTEND noninteractive
RUN apt-get update && \
    apt-get install -y wget gnupg2 && \
    sh -c "wget -O - https://dl.openfoam.org/gpg.key | apt-key add -" && \
    apt-get install -y apt-transport-https ca-certificates && \
    apt-get install -y -qq software-properties-common && \
    add-apt-repository -y ppa:ubuntugis/ubuntugis-unstable && \
    add-apt-repository http://dl.openfoam.org/ubuntu && \
    add-apt-repository -y ppa:rock-core/qt4 && \
    apt-get update && \
    apt-get install -y  \
        cmake \
        git \
        libgdal-dev \
        libqt4-dev \
        libqtwebkit-dev \
        libboost-program-options-dev \
        libboost-date-time-dev \
        libboost-test-dev \
        openfoam8 && \
    rm -rf /var/lib/apt/lists

RUN mkdir /opt/src && \
    cd /opt/src && \
    git clone http://github.com/firelab/windninja && \
    cd  /opt/src/windninja && \
    mkdir build && \
    cd  /opt/src/windninja/build && \
    cmake -D SUPRESS_WARNINGS=ON .. && \
    make -j4 && \
    make install && \
    ldconfig && \
    echo "source /opt/openfoam8/etc/bashrc" >> /root/.bashrc
CMD /usr/bin/bash -c /usr/local/bin/WindNinja
VOLUME /data
WORKDIR /data
