#!/bin/sh

TOP=`pwd`

( cd fftw-3.2.2 ; ./configure --enable-threads --prefix=$TOP/sub )

( cd cfitsio ; ./configure --enable-reentrant --prefix=$TOP/sub )

