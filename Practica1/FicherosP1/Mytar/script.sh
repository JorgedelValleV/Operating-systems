#!/bin/bash

if ! test -x mytar ; then
	echo "No existe o no es ejecutable"
	exit 1
fi
# 2.Si existe tmp lo borra
if test -e tmp ; then
	rm -r tmp
fi
# 3. Crea tmp y cambia a el
mkdir ./tmp
cd tmp
# 4.Creará tres ficheros (el head toma por defecto 10 por lo que no hace falta el head -10)
echo "Hello world!" > file1.txt
head /etc/passwd > file2.txt
head -c 1024 /dev/urandom > file3.dat
# 5. Invoca mytar crea filetar.mtar con los tres ficheros (../ porque estamos en tmp y no en Mytar)
../mytar -c -f filetar.mtar file1.txt file2.txt file3.dat 
# 6. Crea directorio out y copia ahi el filetar.mtar
mkdir out
cp filetar.mtar out/

# 7. Cambia a out y ejecuta mytar para extrar(out->tmp->Mytar)
cd out
../../mytar -x -f filetar.mtar

# 8. usara diff para comparar

if diff file1.txt ../file1.txt && diff file2.txt ../file2.txt && diff file3.dat ../file3.dat ; then
	cd ../..
	echo "Correct"
	exit 0
else
	cd ../..
	echo "Un mensaje descriptivo, algun fichero no coincide"
	exit 1
fi