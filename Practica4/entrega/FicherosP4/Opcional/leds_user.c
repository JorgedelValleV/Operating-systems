#include <stdio.h>
#include <stdlib.h>
#include <math.h> 
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <time.h>
#include <fcntl.h>

static int _file;

#define PATH "/dev/leds" //Recuerdo que ultilizo este path para ir acorde a las diapositivas
// Para que funcione hay que ejecutar script1.sh para chardev_leds seguido de script2.sh
 
 // prototipos disponibles
void binaryPattern();
void reboundPattern();
void alternPattern();
void randomPattern();

void dormir(int ms) {
    usleep(ms*1000);
    return;
}
int setLed(int filedesc, char *buf) {
	int len = strlen(buf);
    int ret;
	if((ret = write(filedesc, buf, len)) < 0) {
		puts("Error en write, SETLEDS\n");
		return -ret;
	}
	return 0;
}
int menu() {
	int opcion = -1;
	bool valido = false;

	puts("\n opcion | Descripcion");
	puts("   1    | Contador binario modulo 8");
	puts("   2    | Led rebota en los extremos");
	puts("   3    | Alternamos entre extremos y centro");
	puts("   4    | Enciende un led de forma aleatoria");	puts("   5    | Contador binario modulo 8 alternativo");
	puts("   0    | Salir\n");

	while (!valido) {
		puts("Que maravilla prefieres: ");
		scanf("%d", &opcion);
		if (opcion >= 0 && opcion <= 5)
			valido=true;
		else
			puts("Vamos campeon, te veo capaz de poner un valor valido\n");
	}

	return opcion;
}
int main() {
	bool acabado= false;
	int opcion;

	printf("\nEs un plaser presentarle mi controlador de leds");

	if((_file = open(PATH, O_RDWR)) < 0) {
		puts("Error al abrir");
		return 1;
	}

	while (!acabado) {

		opcion = menu();
		puts("Que comience el espectaculo");

		if (opcion==0) // exit
			acabado=true;
		else if (opcion == 1) // contador binario
			binaryPattern(); 
		else if (opcion == 2) // secuencia 1-2-3-2-1-2-3 como si el led encendido rebota de lado a lado
			reboundPattern();
		else if (opcion == 3) // Alternan extremos central cada vez mas rapido
			alternPattern();		
		else if (opcion == 4) // random
			randomPattern();
		else if (opcion == 5) // contador binario alternado bits
			binarioAlternativo();


		puts("Suficiente por hoy.");
		
		if((setLed(_file, "")) != 0) {
			puts("Error al apagar leds al salir");
		}
	}

	if (close(_file) < 0) {
		puts("No se pudo cerrar el archivo");
		return 1;
	} 

	return 0;
}

void binaryPattern() { // Recordamos que el orden es 2 1 3 = caps num scroll
	char* buff;
	int i;
	int k;
	for (i=0; i <= 35; i++) {
		k=i%8;				 // cns
		if (k==0) buff = "0";       // 000 
		else if (k==1) buff = "3";  // 001
		else if (k==2) buff = "1";  // 010 
		else if (k==3) buff = "13"; // 011
		else if (k==4) buff = "2";  // 100
		else if (k==5) buff = "23"; // 101
		else if (k==6) buff = "21"; // 110
		else if (k==7) buff = "213";// 111
		 
		if((setLed(_file, buff)) != 0) {
			puts("Error en contador binario SETLEDS");
		}

		dormir(500);//para 1/2 segundo
	}
}
void binarioAlternativo() { // Recordamos que el orden es 1 2 3
	char* buff;
	int i;
	int k;
	for (i=0; i <= 35; i++) {
		k=i%8;				 // ncs
		if (k==0) buff = "0";       // 000
		else if (k==1) buff = "3";  // 001
		else if (k==2) buff = "2";  // 010 
		else if (k==3) buff = "23"; // 011
		else if (k==4) buff = "1";  // 100
		else if (k==5) buff = "13"; // 101
		else if (k==6) buff = "12"; // 110
		else if (k==7) buff = "123";// 111
		 
		if((setLed(_file, buff)) != 0) {
			puts("Error en contador binario SETLEDS");
		}

		dormir(500);//para 1/2 segundo
	}
}

void reboundPattern() {
	int i=0, j=0, total = 10;
	char *ledsNumber = "123";
	char c;
	
	for (i=0; i < total; i++) {
		for (j = 0; j < 4; j++) {
			c = ledsNumber[j];
			if (j == 3) {
				c = ledsNumber[1];
			} 
			if((setLed(_file, &c)) != 0) {
				puts("Error en rebotes SETLEDS");
			}
			dormir(200);
		}
	}
}

void randomPattern() {
	int i, n, num;
	time_t t;
	char *buff;
	char buffer[10]; 

	n = 60;
	srand((unsigned) time(&t));

	for( i = 0 ; i < n ; i++ ) {
		
		num = (random() % 3) + 1;
		sprintf(buffer, "%d", num);
		buff = buffer;
		if((setLed(_file, buff)) != 0) {
			puts("Error en random SETLEDS");
		}
		
		dormir(200);
	}   
}

void alternPattern() {
	int i;
	int n = 20000;
	char *buff;
	int tiempo = 2000;
 
	for( i = 0 ; i < n ; i++ ) {
		
		if (i % 2 == 0) {
			buff = "22";
		}
		else {
			buff = "13";
		}
		
		if((setLed(_file, buff)) != 0) {
			puts("Error en alternar SETLEDS");
		}

		dormir( time );
		tiempo *= 0.9;
	}
}
