#!/bin/sh
bin=$1         #发布的程序名称  

desDir="./lib" #你的路径  

if [ ! -d $desDir ];then
      mkdir $desDir
fi 
libList=$(ldd $bin | awk  '{if (match($3,"/")){ printf("%s "),$3 } }')
cp $libList $desDir