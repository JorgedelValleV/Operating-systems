#!/bin/bash

rm -r temp
mkdir ./temp

#aaaaaaaaaaaaaaaaaaaa

#  Copie dos ficheros de texto que ocupen más de un bloque (por ejemplo fuseLib.c y myFS.h) 
#  a nuestro SF y a un directorio temporal, por ejemplo ./temp
cp ./src/fuseLib.c ./mount-point/fuseLib.c
cp ./src/myFS.h ./mount-point/myFS.h

cp ./src/fuseLib.c ./temp/fuseLib.c
cp ./src/myFS.h ./temp/myFS.h

#bbbbbbbbbbbbbbbbbbbbb

# Audite el disco
./my-fsck virtual-disk

#  haga un diff entre los ficheros originales y los copiados en el SF
DIFF_1=$(diff ./src/fuseLib.c ./mount-point/fuseLib.c)
DIFF_2=$(diff ./src/myFS.h ./mount-point/myFS.h)
if [ "$DIFF_1" == "" ];
then echo "Iguales la copia de fuseLib.c en mount y original"
else echo "Diferrentes la copia de fuseLib.c en mount y original"
fi
if [ "$DIFF_2" == "" ]; 
then echo "Iguales la copia de myFS.h en mount y original"
else echo "Diferrentes la copia de myFS.h en mount y original"
fi

# Comprobacion auxiliar de las copias en temp
DIFF_3=$(diff ./src/fuseLib.c ./temp/fuseLib.c)
DIFF_4=$(diff ./src/myFS.h ./temp/myFS.h)
if [ "$DIFF_3" == "" ];
then echo "Iguales la copia de fuseLib.c en temp y original"
else echo "Diferrentes la copia de fuseLib.c en temp y original"
fi
if [ "$DIFF_4" == "" ]; 
then echo "Iguales la copia de myFS.h en temp y original"
else echo "Diferrentes la copia de myFS.h en temp y original"
fi

#Trunque el primer fichero (man truncate) en copiasTemporales y en nuestro SF de
#manera que ocupe ocupe un bloque de datos menos
truncate -s -4K ./mount-point/fuseLib.c
truncate -s -4K ./temp/fuseLib.c
echo "Truncado fuseLib.c en mount y temp"

#ccccccccccccccccccccc

# Audite el disco
./my-fsck virtual-disk

# haga un diff entre el fichero original y el truncado.
DIFF_T1=$(diff ./src/fuseLib.c ./mount-point/fuseLib.c)
DIFF_T2=$(diff ./src/fuseLib.c ./temp/fuseLib.c)

if [ "$DIFF_T1" == "" ]; 
then echo "Iguales fuseLib.c original y el truncado en el SF en mount"
else echo "Diferentes fuseLib.c original y el truncado en el SF en mount"
fi

if [ "$DIFF_T2" == "" ]; 
then echo "Iguales fuseLib.c original y el truncado en temp"
else echo "Diferentes fuseLib.c original y el truncado en temp"
fi

#ddddddddddddddddddddd

# Copie un tercer fichero de texto a nuestro SF. Por ejemplo MyfileSystem.c
cp ./src/MyFileSystem.c ./mount-point/MyFileSystem.c

#eeeeeeeeeeeeeeeeeeeee

# Audite el disco
./my-fsck virtual-disk

#  haga un diff entre el fichero original y el copiado en el SF
DIFF_5=$(diff ./src/MyFileSystem.c ./mount-point/MyFileSystem.c)
if [ "$DIFF_5" == "" ];
then echo "Iguales la copia de MyFileSystem.c en mount y original"
else echo "Diferrentes la copia de MyFileSystem.c en mount y original"
fi

#fffffffffffffffffffff

# Trunque el segundo fichero en copiasTemporales y en nuestro 
# SF haciendo que ocupe algún bloque de datos más
# 4K = 4096 B blocksize
truncate -s +4K ./mount-point/myFS.h
truncate -s +4K ./temp/myFS.h

echo "Truncado myFS.h en mount y temp"

#ggggggggggggggggggggg

# Audite el disco
./my-fsck virtual-disk

# haga un diff entre el fichero original y el truncado. 
# Intuyo que tanto en SF como temp
DIFF_T3=$(diff ./src/myFS.h ./mount-point/myFS.h)
DIFF_T4=$(diff ./src/myFS.h ./temp/myFS.h)

if [ "$DIFF_T3" == "" ]; 
then echo "Iguales myFS.h original y el truncado en el SF en mount"
else echo "Diferentes myFS.h original y el truncado en el SF en mount"
fi

if [ "$DIFF_T4" == "" ]; 
then echo "Iguales myFS.h original y el truncado en temp"
else echo "Diferentes myFS.h original y el truncado en temp"
fi