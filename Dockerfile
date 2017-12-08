# Copyright (c) Kyle Shannon
# Distributed under the terms of the Modified BSD License.
FROM ubuntu:16.04

MAINTAINER Kyle Shannon <kyle@pobox.com>

USER root
ENV DEBIAN_FRONTEND noninteractive
RUN apt-get update && \
    apt-get install -y -qq software-properties-common && \
    add-apt-repository -y ppa:ubuntugis/ubuntugis-unstable && \
    add-apt-repository http://dl.openfoam.org/ubuntu && \
    apt-get update && \
    apt-get install -y -qq \
        cmake \
        git \
        libgdal-dev \
        qt4-dev-tools \
        libqtwebkit-dev \
        libboost-program-options-dev \
        libboost-date-time-dev \
        libboost-test-dev \
        openfoam4 && \


RUN mkdir /opt/src
WORKDIR /opt/src
RUN git clone http://github.com/firelab/windninja
WORKDIR /opt/src/windninja
RUN mkdir build
WORKDIR /opt/src/windninja/build
RUN cmake -D SUPRESS_WARNINGS=ON ..
RUN make -j4
RUN make install

RUN useradd -m -s /bin/bash -N -u 1000 ziggy
USER ziggy
WORKDIR /home/ziggy
RUN source /opt/openfoam4/etc/bashrc
