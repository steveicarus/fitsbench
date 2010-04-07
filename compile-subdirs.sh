#!/bin/sh

TOP=`pwd`

( cd fftw-3.2.2 ; make ; make install )

( cd cfitsio ; make ; make install )

