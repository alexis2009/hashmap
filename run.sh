#!/bin/bash

gcc hashmap.c
python gentest.py
str=`./a.out < in > tmp && diff out tmp`
rm tmp

[[ $str == '' ]] && echo "OK" || echo "ERROR"
