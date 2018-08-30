#!/bin/bash
int=1
up=10
balance=1000
symbol="SYM"
while(($int<=$up))
do
    echo -e "357" >>create$int.xml
    echo -e "<?xml version="1.0" encoding="UTF-8"?>" >> create$int.xml
    echo -e "<create>" >> create$int.xml
    echo -e "  <account id=\"$int\" balance=\"1000\"/>" >> create$int.xml
    echo -e "  <symbol sym=\"$symbol$int\">" >> create$int.xml
    echo -e "    <account id=\"$int\">1000</account>" >> create$int.xml
    echo -e "  </symbol>" >> create$int.xml
    echo -e "</create>" >> create$int.xml
    echo -e  "<transactions accountid=\"$int\">" >>create$int.xml
    echo -e "<order sym=\"$symbol$int\" amount=\"-500\" limit=\"11\"></order>" >>create$int.xml
    echo -e "</transactions>" >>create$int.xml
    echo -e  "<transactions accountid=\"$int\">" >>create$int.xml
    echo -e "<order sym=\"$symbol$int\" amount=\"50\" limit=\"20\"></order>" >>create$int.xml
    echo -e "</transactions>" >>create$int.xml
    let "int++"
    done

int=1
up=10
while(($int<=$up))
do
    nc vcm-3813.vm.duke.edu 12345 < create$int.xml
    let int++
        done
