#!/bin/bash                                                                    


    echo -e "<?xml version="1.0" encoding="UTF-8"?>" >> create16.xml
    echo -e  "<transactions accountid=\"1\">" >>create16.xml
    echo -e "<cancel uid=\"2\" ></cancel>" >>create16.xml
    echo -e "<cancel uid=\"9\" ></cancel>" >>create16.xml
    echo -e "</transactions>" >>create16.xml

    nc vcm-3813.vm.duke.edu 12345 < create16.xml

