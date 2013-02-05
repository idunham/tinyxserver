#!/bin/sh

VF=version.txt

if [ -r $VF ] && [ -f $VF ] && [ -s $VF ]; then
    set $(cat version.txt) NONE NULL
    DIR=$1
    VER=$2
    
    cd ..
   	tar czvf $DIR-$VER.tar.gz $DIR-$VER
fi


