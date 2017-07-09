#!/bin/sh
./TestMeshInfo $1
./TestAmrLevels $1
./TestAmrTree $1
./TestAmrConnect $1
./TestAmrSidePoints $1
