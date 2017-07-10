#!/bin/sh
./MeshInfo $1
./AmrLevels $1
./AmrTree $1
./AmrConnect $1
./AmrSidePoints $1
