#!/bin/bash

#Kontrola vstupnich argumentu
if [ $# != 2 ]; then
	echo "Chyba: Spatny pocet argumentu."
	exit 1
fi

#1. argument
pocetHodnot=$1
#2. argument
pocetProcesoru=$2

#Preklad C zdrojaku
mpicc --prefix /usr/local/share/OpenMPI -o mes mes.c

#Vytvoreni souboru s random cisly (vstupni soubor programu)
dd if=/dev/random bs=1 count=$pocetHodnot of=numbers status=none

#Spusteni
mpirun --prefix /usr/local/share/OpenMPI -np $pocetProcesoru mes numbers $pocetProcesoru

#Uklid
rm -f mes numbers