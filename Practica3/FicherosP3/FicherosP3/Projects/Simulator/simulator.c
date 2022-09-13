#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

#define N_PARADAS 5 // número de paradas de la ruta
#define EN_RUTA 0 // autobús en ruta
#define EN_PARADA 1 // autobús en la parada
#define MAX_USUARIOS 40 // capacidad del autobús
#define USUARIOS 4 // numero de usuarios
// estado inicial
int estado = EN_RUTA;
int parada_actual = 0; // parada en la que se encuentra el autobus
int n_ocupantes = 0; // ocupantes que tiene el autobús
// personas que desean subir en cada parada
int esperando_parada[N_PARADAS]; //= {0,0,...0};
// personas que desean bajar en cada parada
int esperando_bajar[N_PARADAS]; //= {0,0,...0};

// Otras definiciones globales (comunicación y sincronización)

// Para cada parada un mutex de subida y otro de bajada
pthread_mutex_t mutexBajar[N_PARADAS];
pthread_mutex_t mutexSubir[N_PARADAS];
// VAriables condicion
pthread_cond_t puedenSubir;
pthread_cond_t puedenBajar;// se le espera en bajarautobus a que  autobus en parada les permita bajar
pthread_cond_t yaNoSuben;
pthread_cond_t yaNoBajan;// cuando en bajar autobus se han bajado todos permite a autobus en parada dar permiso de subida
//Hilos
pthread_t autobus_t;
pthread_t usuarios[USUARIOS];

void * thread_autobus(void * args) {
	while (1) {
		// esperar a que los viajeros suban y bajen
		Autobus_En_Parada();
		// conducir hasta siguiente parada
		Conducir_Hasta_Siguiente_Parada();
	}
}
void * thread_usuario(void * arg) {
	int id_usuario,a,b;
	// obtener el id del usario
	id_usuario= (int *) arg;
	while (1) {
		a=rand() % N_PARADAS;
		do{
			b=rand() % N_PARADAS;
		} while(a==b);
  		printf("%d VIAJA de %d hacia %d \n", id_usuario, a, b);
		Usuario(id_usuario,a,b);
	}
}
void Usuario(int id_usuario, int origen, int destino) {
	// Esperar a que el autobus esté en parada origen para subir
	Subir_Autobus(id_usuario, origen);
	// Bajarme en estación destino
	Bajar_Autobus(id_usuario, destino);
}
int main(int argc, char *argv[]) {
	int i;
	// Definición de variables locales a main
	// Opcional: obtener de los argumentos del programa la capacidad del
	// autobus, el numero de usuarios y el numero de paradas
	for(i=0; i < N_PARADAS; i++){
	  	pthread_mutex_init(&mutexSubir[i],NULL);
		pthread_mutex_init(&mutexBajar[i],NULL);
	}
	
	pthread_cond_init(&puedenSubir, NULL);
	pthread_cond_init(&puedenBajar, NULL);
	pthread_cond_init(&yaNoSuben, NULL);
	pthread_cond_init(&yaNoBajan, NULL);
	
	// CREAR HILOS

	pthread_create(&autobus_t, NULL, thread_autobus, NULL);
	for (i = 0; i < USUARIOS; i++){
		pthread_create(&usuarios[i], NULL, thread_usuario, (void *) i);
	}

	// ESPERA
	for (i = 0; i < USUARIOS; i++){
		pthread_join(usuarios[i], NULL);
	}
	pthread_join(autobus_t, NULL);

	// DESTR0Y MUTEX Y VARCOND
	for(i=0; i<N_PARADAS; i++){
		pthread_mutex_destroy(&mutexSubir[i]);
		pthread_mutex_destroy(&mutexBajar[i]);
	}
	pthread_cond_destroy(&puedenSubir);
	pthread_cond_destroy(&puedenBajar);
	pthread_cond_destroy(&yaNoSuben);
	pthread_cond_destroy(&yaNoBajan);

	return 0;
}
int min(int x, int y){
	if(x < y)
		return x;
	else
		return y;
}

void Autobus_En_Parada(){
	/* Ajustar el estado y bloquear al autobús hasta que no haya pasajeros que
	quieran bajar y/o subir la parada actual. Después se pone en marcha */

	printf("BUS parado en %d \n", parada_actual);
	estado = EN_PARADA;

	pthread_mutex_lock(&mutexSubir[parada_actual]);
	pthread_mutex_lock(&mutexBajar[parada_actual]);

	// CAlculos de ocupacion
	n_ocupantes -= esperando_bajar[parada_actual];// cuantos se bajan
	int max_subir = (esperando_parada[parada_actual]<MAX_USUARIOS-n_ocupantes )? esperando_parada[parada_actual] : MAX_USUARIOS-n_ocupantes;// Cuantos pueden subir
	n_ocupantes += max_subir;
	
	pthread_cond_broadcast(&puedenBajar); // A bajaaaar
	
	// A subiiiir
	// no se si pthread_cond_broadcast(&puedenSubir) haria que subieran mas de los que entran
	// quizas para el broadcast habria que comprobar en el while de subir que (++)n_ocupantes >= MAX_USUARIOS
	//pthread_cond_broadcast(&puedenSubir);//diria que sirve despertar a todos los que quieren subir por (++)
	// pthread_cond_signal(&puedenSubir); no me funciona bien y se bloquea
	// Creo que se debe a que la subida se realiza de uno en uno
	int j;
	for(j = 0; j < max_subir; j++){
		pthread_cond_broadcast(&puedenSubir);
	}
	
	// Esperamos que termine de bajar priorizando sobre los que quieran subir
	//como funcionaria  si hay gente que quiere subir y no entra(USUARIOS>MAX_USUARIOS)
	/*while(esperando_parada[parada_actual] > 0 || esperando_bajar[parada_actual] > 0){

		//si solo quedan por bajar
		if(esperando_bajar[parada_actual] > 0 && esperando_parada[parada_actual] == 0)
			pthread_cond_wait(&yaNoBajan, &mutexBajar[parada_actual]);

		//si solo quedan por subir
		else if(esperando_bajar[parada_actual] == 0 && esperando_parada[parada_actual] > 0)
			pthread_cond_wait(&yaNoSuben, &mutexSubir[parada_actual]);
  
		else{
			pthread_mutex_unlock(&mutexSubir[parada_actual]);
			pthread_cond_wait(&yaNoBajan, &mutexBajar[parada_actual]);
			pthread_mutex_lock(&mutexSubir[parada_actual]);
		}
	}*/
	
	// mismo bucle otra forma
	// Primero dejamos bajar
	while(esperando_bajar[parada_actual] > 0){
		pthread_mutex_unlock(&mutexSubir[parada_actual]);
		pthread_cond_wait(&yaNoBajan, &mutexBajar[parada_actual]);
		pthread_mutex_lock(&mutexSubir[parada_actual]);// cuando ya no bajan empiezan a subir bloqueamos el mutex
	}
	while(esperando_parada[parada_actual] > 0){
		pthread_cond_wait(&yaNoSuben, &mutexSubir[parada_actual]);
	}

	//el bus suelta los dos mutex
	pthread_mutex_unlock(&mutexSubir[parada_actual]);
	pthread_mutex_unlock(&mutexBajar[parada_actual]);

	estado = EN_RUTA;
	printf("...............bus en camino...................... \n");
}
void Conducir_Hasta_Siguiente_Parada(){
	/* Establecer un Retardo que simule el trayecto y actualizar numero de parada */
	sleep((random() % N_PARADAS)+1);
 	parada_actual = (parada_actual + 1) % N_PARADAS;
}

// Subir autobus y bajar autobus son como la puesta en conjunto de entralector y salelector 
// donde el escritor es el bus en la parada deseada
void Subir_Autobus(int id_usuario, int origen){
	/* El usuario indicará que quiere subir en la parada ’origen’, esperará a que
	el autobús se pare en dicha parada y subirá. El id_usuario puede utilizarse para
	proporcionar información de depuración */
	
	// El usuario id_usuario quiere subir en origen 
 	pthread_mutex_lock(&mutexSubir[origen]);
 	esperando_parada[origen]++;

	// Hasta que no se llegue a origen, y una vez ahi se permita la subida no se sube
 	while(parada_actual != origen || estado != EN_PARADA){
  		pthread_cond_wait(&puedenSubir, &mutexSubir[origen]);
 	}

	esperando_parada[origen]--;
    printf("%d SUBE en parada %d \n", id_usuario, origen);
	if(esperando_parada[origen] == 0)
		pthread_cond_signal(&yaNoSuben);//han subido todos

 	
	pthread_mutex_unlock(&mutexSubir[origen]);// deja a otros usuarios de su parada subir

}
void Bajar_Autobus(int id_usuario, int destino){
	/* El usuario indicará que quiere bajar en la parada ’destino’, esperará a que
	el autobús se pare en dicha parada y bajará. El id_usuario puede utilizarse para
	roporcionar información de depuración */

	// El usuario id_usuario quiere bajar en destino 
	pthread_mutex_lock(&mutexBajar[destino]);
 	esperando_bajar[destino]++;
 
    // Hasta que no se llegue a destino, y una vez ahi se permita la bajada no se baja
 	while(parada_actual != destino || estado != EN_PARADA){
  		pthread_cond_wait(&puedenBajar, &mutexBajar[destino]);
 	}

 	esperando_bajar[destino]--;
	printf("__________________________%d BAJA en parada %d \n", id_usuario, destino);

 	if(esperando_bajar[destino] == 0)
  		pthread_cond_signal(&yaNoBajan);// Han bajado todos
	
 	pthread_mutex_unlock(&mutexBajar[destino]);//permite a otros usuarios bajar en esta parada
}

/*
 * EJEMPLO DE EJECUCION 3 vueltas al circuito
*/
/*
US parado en 0 
...............bus en camino...................... 
0 VIAJA de 1 hacia 2 
1 VIAJA de 0 hacia 3 
2 VIAJA de 0 hacia 1 
3 VIAJA de 2 hacia 4 
BUS parado en 1 
0 SUBE en parada 1 
...............bus en camino...................... 
BUS parado en 2 
__________________________0 BAJA en parada 2 
0 VIAJA de 2 hacia 0 
0 SUBE en parada 2 
3 SUBE en parada 2 
...............bus en camino...................... 
BUS parado en 3 
...............bus en camino...................... 
BUS parado en 4 
__________________________3 BAJA en parada 4 
...............bus en camino...................... 
3 VIAJA de 0 hacia 1 
BUS parado en 0 
__________________________0 BAJA en parada 0 
0 VIAJA de 2 hacia 1 
1 SUBE en parada 0 
2 SUBE en parada 0 
3 SUBE en parada 0 
...............bus en camino...................... 
BUS parado en 1 
__________________________2 BAJA en parada 1 
2 VIAJA de 3 hacia 2 
__________________________3 BAJA en parada 1 
...............bus en camino...................... 
3 VIAJA de 2 hacia 0 
BUS parado en 2 
0 SUBE en parada 2 
3 SUBE en parada 2 
...............bus en camino...................... 
BUS parado en 3 
__________________________1 BAJA en parada 3 
1 VIAJA de 3 hacia 2 
1 SUBE en parada 3 
2 SUBE en parada 3 
...............bus en camino...................... 
BUS parado en 4 
...............bus en camino...................... 
BUS parado en 0 
__________________________3 BAJA en parada 0 
...............bus en camino...................... 
3 VIAJA de 2 hacia 3 
BUS parado en 1 
__________________________0 BAJA en parada 1 
0 VIAJA de 4 hacia 2 
...............bus en camino...................... 
BUS parado en 2 
__________________________1 BAJA en parada 2 
1 VIAJA de 1 hacia 2 
__________________________2 BAJA en parada 2 
2 VIAJA de 4 hacia 3 
3 SUBE en parada 2 
...............bus en camino...................... 
BUS parado en 3 
__________________________3 BAJA en parada 3 
...............bus en camino...................... 
3 VIAJA de 4 hacia 2 
BUS parado en 4 
0 SUBE en parada 4 
2 SUBE en parada 4 
3 SUBE en parada 4 
...............bus en camino...................... 
BUS parado en 0
*/