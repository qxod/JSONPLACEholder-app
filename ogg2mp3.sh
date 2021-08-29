#! /bin/sh
./gstfs -f -osrc=$1,src_ext=ogg,dst_ext=mp3,pipeline="filesrc name=\"_source\" ! oggdemux !