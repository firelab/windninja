#!/bin/bash

mkdir msmpi && cd msmpi
wget -O msmpi64.msi "https://download.microsoft.com/download/A/1/3/A1397A8C-4751-433C-8330-F738C3BE2187/mpi_x64.Msi"
7z e msmpi64.msi

