#!/bin/sh
mkdir -p build
xc8 -Q --opt=3 --std=c99 --chip=18F27J53 --outdir=build -Oalarmclock *.c
