#!/bin/bash
int=6
up=10
symbol="SYM"
while(($int<=$up))
do
    echo -e "230" >>create$int.xml
    echo -e "<?xml version="1.0" encoding="UTF-8"?>" >> create$int.xml
    echo -e  "<transactions accountid=\"$(($int-5))\">" >>create$int.xml
    echo -e "<order sym=\"$symbol$(($int-5))\" amount=\"-500\" limit=\"11\"></order>" >>create$int.xml
    echo -e "</transactions>" >>create$int.xml
    echo -e  "<transactions accountid=\"$(($int-5))\">" >>create$int.xml
    echo -e "<order sym=\"$symbol$(($int-6))\" amount=\"50\" limit=\"20\"></order>" >>create$int.xml
    echo -e "</transactions>" >>create$int.xml

    let "int++"
done
int=6
up=10
while(($int<=$up))
do
    nc vcm-3813.vm.duke.edu 12345 < create$int.xml
    let int++
    done
