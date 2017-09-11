#!/bin/bash

if (( "$#" == 0 )); then
    exec 5< numbers

    while   read line1 <&5 ; do
            read line2 <&5
            
            size1=${#line1}
            size2=${#line2}

            if (($size1 >= $size2)); then
                lenth=$size1
            else
                lenth=$size2
            fi              
    done
    exec 5<&-
else
   lenth=$1 
fi

#Preklad C++ zdrojaku
mpic++ --prefix /usr/local/share/OpenMPI -o clapba clapba.cpp

((pocetProcesoru = lenth+lenth-1))

#Spusteni
mpirun --prefix /usr/local/share/OpenMPI -np $pocetProcesoru clapba

#Uklid
rm -f clapba