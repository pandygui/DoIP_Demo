#!/bin/sh
exe="Vehicle"
des="/home/zhang/桌面/build-Vehicle-Desktop_Qt_5_3_GCC_64bit-Release/mytest"

deplist=$(ldd $exe | awk '{if (match($3,"/")){ printf("%s "),$3 } }')

cp $deplist $des
