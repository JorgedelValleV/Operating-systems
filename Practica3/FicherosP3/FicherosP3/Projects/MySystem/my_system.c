#include <stdlib.h>
#include <stdio.h>

int main(int argc, char* argv[])
{
	if (argc!=2){
		fprintf(stderr, "Usage: %s <command>\n", argv[0]);
		exit(1);
	}

	return my_system(argv[1]);
}

int my_system(const char * command){
 
// utiliza la llamada al sistema fork() y la función execl()
// para crear un nuevo proceso shell hijo que invoca el comando 
// que se pasa como parámetro a la función
	int stat;
 	int pid = fork();

 	if(pid == -1)
  		printf("ERROR. Process %u could not fork.\n",pid);

	else if(pid == 0)//Child
  		execlp("/bin/bash", "bash", "-c", command, (char*) NULL);

 	else{//Parent
  		wait(&stat);// Padre espera a finalización de hijos
  		stat = WEXITSTATUS(stat);
 	}
	return stat;
}

/*
 * EJEMPLO DE EJECUCION ./my_system "ls -l"
*/
/*
total 28
-rw-rw-r-- 1 usuarioso usuarioso   122 nov  8  2019 Makefile
-rwxrwxr-x 1 usuarioso usuarioso 20032 ene 20 20:39 my_system
-rw-rw-r-- 1 usuarioso usuarioso   715 ene 20 20:39 my_system.c
*/

/*
 * EJEMPLO DE EJECUCION ./my_system ls
*/
/*
Makefile  my_system  my_system.c
*/

/*
 * EJEMPLO DE EJECUCION ./my_system "echo hola > t"
*/
/*
crea el archivo correctamente
*/