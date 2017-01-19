#!/bin/sh

root=`pwd`
echo $1

LD_LIBRARY_PATH=$root/..:$root/../../lib/uriparser/lib:$root/../../lib/xerces/lib:/lib/:/usr/lib:$LD_LIBRARY_PATH
chmod 777 ./main
./main $1