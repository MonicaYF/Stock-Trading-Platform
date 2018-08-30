#!/bin/bash                                                                 
int=11
up=14                 
symbol="SYM"
while(($int<=$up))
do
    echo -e "135" >>create$int.xml
    echo -e "<?xml version="1.0" encoding="UTF-8"?>" >> create$int.xml
    echo -e  "<transactions accountid=\"$(($int-10))\">" >>create$int.xml
    echo -e "<query uid=\"$((($int-10)*2-1))\" ></query>" >>create$int.xml
    echo -e "<query uid=\"$((($int-10)*2))\" ></query>" >>create$int.xml    
    echo -e "</transactions>" >>create$int.xml
    let "int++"
done
int=11
up=14
while(($int<=$up))
do
    nc vcm-3813.vm.duke.edu 12345 < create$int.xml
let int++
done
