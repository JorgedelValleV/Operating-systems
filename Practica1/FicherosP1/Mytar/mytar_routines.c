#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "mytar.h"

extern char *use;

/** Copy nBytes bytes from the origin file to the destination file.
 *
 * origin: pointer to the FILE descriptor associated with the origin file
 * destination:  pointer to the FILE descriptor associated with the destination file
 * nBytes: number of bytes to copy
 *
 * Returns the number of bytes actually copied or -1 if an error occured.
 */
int
copynFile(FILE * origin, FILE * destination, int nBytes)
{
	// Complete the function
	int n_Bytes = 0;
    int destinationByte = 0;
    int originByte = 0;

    if (origin == NULL) {// Error no existe archivo origen
		 return -1; 
	}

    // Copia de datos
    while((n_Bytes < nBytes) && (originByte = getc(origin)) != EOF) {
        if ((ferror(origin) != 0)) {//ERROR
			 return -1; 
		}
        destinationByte = putc(originByte, destination);
        if (destinationByte == EOF) { //Error no se lee bien un valor
			return -1; 
		}
        n_Bytes++;
    }
    return n_Bytes;
}

/** Loads a string from a file.
 *
 * file: pointer to the FILE descriptor 
 * 
 * The loadstr() function must allocate memory from the heap to store 
 * the contents of the string read from the FILE. 
 * Once the string has been properly built in memory, the function returns
 * the starting address of the string (pointer returned by malloc()) 
 * 
 * Returns: !=NULL if success, NULL if error
 */
char*
loadstr(FILE * file)
{
	// Complete the function
	int fileLength = 0;
	int i = 0;
    char *name;
    char c;

    while((c = getc(file) != '\0')) {
        fileLength++;
    }

    name =  malloc(sizeof(char) * (fileLength + 1)); // +1 para el final
    fseek(file, -(fileLength + 1), SEEK_CUR);

    for (i = 0; i < fileLength+1; i++) {
        name[i] = getc(file);
    }    
    return name;
}

/** Read tarball header and store it in memory.
 *
 * tarFile: pointer to the tarball's FILE descriptor 
 * nFiles: output parameter. Used to return the number 
 * of files stored in the tarball archive (first 4 bytes of the header).
 *
 * On success it returns the starting memory address of an array that stores 
 * the (name,size) pairs read from the tar file. Upon failure, the function returns NULL.
 */
stHeaderEntry*
readHeader(FILE * tarFile, int *nFiles)
{
	// Complete the function
	int nr_files = 0;//valor a devolver por referencia
	int i = 0;
	int size = 0;
    stHeaderEntry *stHeader=NULL;//valor a devolver

    if (fread( &nr_files, sizeof(int), 1, tarFile) == 0) {//lee nº de filas
        return NULL; //error al leer el mtar
    }

    stHeader=malloc(sizeof(stHeaderEntry)*nr_files);//reserva de memoria
   
    for (i = 0; i < nr_files; i++) {
        if ((stHeader[i].name=loadstr(tarFile)) ==NULL) {
            return NULL;
        }
        fread(&size, sizeof(unsigned int), 1, tarFile);
        stHeader[i].size = size;
    }

    (*nFiles)=nr_files;
    return stHeader;
}

/** Creates a tarball archive 
 *
 * nfiles: number of files to be stored in the tarball
 * filenames: array with the path names of the files to be included in the tarball
 * tarname: name of the tarball archive
 * 
 * On success, it returns EXIT_SUCCESS; upon error it returns EXIT_FAILURE. 
 * (macros defined in stdlib.h).
 *
 * HINTS: First reserve room in the file to store the tarball header.
 * Move the file's position indicator to the data section (skip the header)
 * and dump the contents of the source files (one by one) in the tarball archive. 
 * At the same time, build the representation of the tarball header in memory.
 * Finally, rewind the file's position indicator, write the number of files as well as 
 * the (file name,file size) pairs in the tar archive.
 *
 * Important reminder: to calculate the room needed for the header, a simple sizeof 
 * of stHeaderEntry will not work. Bear in mind that, on disk, file names found in (name,size) 
 * pairs occupy strlen(name)+1 bytes.
 *
 */
int
createTar(int nFiles, char *fileNames[], char tarName[])
{
	// Complete the function
	FILE * inputFile;
    FILE * mtarFile;// fichero destino

    int copiados = 0;//bytes copiados
    int stHeaderBytes = 0;
    int i = 0;
    stHeaderEntry *stHeader;

    // reserva memoria para las cabeceras
    stHeader      =  malloc(sizeof(stHeaderEntry) * nFiles);
    stHeaderBytes += sizeof(int); // nº de archivos
    stHeaderBytes += nFiles*sizeof(unsigned int); // para cada estructura

    for (i=0; i < nFiles; i++) {
        stHeaderBytes+=strlen(fileNames[i])+1; // suma de bytes
    }

    mtarFile =  fopen(tarName, "w"); // Abrir el mtar para escritura
    fseek(mtarFile, stHeaderBytes, SEEK_SET); // mover el output para dejar espacio a la cabecera

    for (i=0; i < nFiles; i++) {
        if ((inputFile = fopen(fileNames[i], "r")) == NULL) {//Abrimos inputdile- control de error
            return (EXIT_FAILURE);
        }
        copiados = copynFile(inputFile, mtarFile, INT_MAX);//copiamos
    
        if (copiados == -1) {     
            return EXIT_FAILURE;    
        } 
        else {//rellenamos el stheaderEntry correspondiente con la ruta y tama˜no de inputFil
            stHeader[i].size = copiados;
            stHeader[i].name = malloc(sizeof(fileNames[i]) + 1);
            strcpy(stHeader[i].name, fileNames[i]);
        }        
    
        if (fclose(inputFile) == EOF) return EXIT_FAILURE; // Cerramos - Error
    }
     
    // Nos posicionamos para escribir en el byte 0 del fichero mtar 
    if (fseek(mtarFile, 0, SEEK_SET) != 0)
        return (EXIT_FAILURE);
    else
        fwrite(&nFiles, sizeof(int), 1, mtarFile); // Escribir el n´umero de ficheros(int de 4 bytes)
    
    //Copiar el array de stHeaderEntry
    for (i = 0; i < nFiles; i++) {
        fwrite(stHeader[i].name, strlen(stHeader[i].name)+1, 1, mtarFile);// +1 por el char final
        fwrite(&stHeader[i].size, sizeof(unsigned int), 1, mtarFile);// 1 byte
    }

    //Liberamos memoria y cerramos el fichero mtar
    for (i=0; i < nFiles; i++) {
        free(stHeader[i].name);
    }
    free(stHeader);
    if (fclose(mtarFile)   == EOF) {//cerrar-error
		return (EXIT_FAILURE);
	} 

    printf("eres un genio\n");

    return (EXIT_SUCCESS);
	//return EXIT_FAILURE
}

/** Extract files stored in a tarball archive
 *
 * tarName: tarball's pathname
 *
 * On success, it returns EXIT_SUCCESS; upon error it returns EXIT_FAILURE. 
 * (macros defined in stdlib.h).
 *
 * HINTS: First load the tarball's header into memory.
 * After reading the header, the file position indicator will be located at the 
 * tarball's data section. By using information from the 
 * header --number of files and (file name, file size) pairs--, extract files 
 * stored in the data section of the tarball.
 *
 */
int
extractTar(char tarName[])
{
	// Complete the function
	FILE *file = NULL;
    FILE *destinationFile = NULL;
    stHeaderEntry *stHeader;
    int nr_files = 0;
	int i = 0;
	int copiedBytes = 0;

    if((file = fopen(tarName, "r") ) == NULL) {//error no existe o problema
        return (EXIT_FAILURE); 
    }
    if ((stHeader=readHeader(file, &nr_files)) == NULL) {
        return (EXIT_FAILURE); 
    } 
    
	//Escritura
    for (i = 0; i < nr_files; i++) {
        if ((destinationFile = fopen(stHeader[i].name, "w")) == NULL) {//abre con control de posible error
			return EXIT_FAILURE;
		}
        else {
            copiedBytes = copynFile(file, destinationFile, stHeader[i].size);//escritura de la catidad de bytes definida en el header       
            if (copiedBytes == -1) {
			    return EXIT_FAILURE;
		    }     
        }  
        if(fclose(destinationFile) != 0) {
			return EXIT_FAILURE;
		}
    }
    
    for (i = 0; i <nr_files; i++) {
        free(stHeader[i].name);
    }
    free(stHeader);
    if (fclose(file) == EOF) {return (EXIT_FAILURE);}
    return (EXIT_SUCCESS);
}