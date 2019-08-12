/*
Sistemas Operativos
Departamento de Ingenierıa en Informatica
LAB3: Programacion con hebras Pthreads

Autor: Dany Efrain Rubiano Jimenez 22250855-k
*/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

pthread_mutex_t  * listaMutex;
pthread_t * listaHilos;
int **jugadas;  // matriz que guarda las jugadas posibles de cada ronda indicadas por entrada
int **parejas; // matriz que guarda las parejas que se reciben por entrada
int **apuestas; // matriz que guarda las apuestas que realiza cada preja en cada ronda
int *puntajes; // arreglo que guarda los puntajes por equipo
int apuesta_compartida[2]; // arreglo que permite compartir las jugadas entre los jugadores de una pareja

typedef struct datos{
	int id; //guarda el id de cada hebra para su sincronizacion
	int cant_parejas; 
	int cant_rondas;
	int index; // guarga el index donde debe comenzar a recorrer la matriz de jugadas
	int pareja; // guarda el id de la pareja a la cual corresponde 
	int ronda; // guarda la ronda
	int turno; // guarda el orden de juego por equipo
	int apuesta; // guarda la apuesta del jugador
} Datos;

/* Funcion que permite la sincronizacion y el manejo del juego */
void jugar(struct datos *data){
	int turn, ronda, pareja, i, index, id, ganancia;
	turn = data->turno;
	ronda = data->ronda;
	pareja = data->pareja-1;
	id = data->id;
    
    pthread_mutex_lock(&listaMutex[id]);
    //printf("hebra %d\n", id);

	if(turn == 1){
		calcularJugada(data);
	}

	if(turn == 2){
		calcularJugada(data);
		ganancia = apuesta_compartida[0] + apuesta_compartida[1];
		apuestas[ronda][pareja] = ganancia;
		//printf("%d, %d\n", apuesta_compartida[0], apuesta_compartida[1]);
		//printf("%d\n", ganancia);
	}

	if(turn == 3){ //turno =3 correspondería a la hebra  que calcula el ganador en cada ronda
		calcularGanador(data);
	}

	if(turn == 4){ // turno = 4 corresponde a la ultima hebra, la cual debe imprimir los puntajes de los ganadores
		for(i=0; i < data->cant_parejas; i++){
			printf("%d\n", puntajes[i]);
		}
	}
	pthread_mutex_unlock(&listaMutex[id+1]);

}

/* Funcion que calcula cual es la mejor jugada para apostar, guardando su resultado en la memoria compartida */
int calcularJugada(struct datos *data){
	int apuesta, si, no, ronda, index, turno;
	index = data->index;
	ronda = data->ronda;
	turno = data->turno;

	si = jugadas[ronda][index] - jugadas[ronda][index+1]; // Calcula la ganancia en caso de apuesta
	no = jugadas[ronda][index+2] - jugadas[ronda][index+3]; // calcula la ganancia en caso de no apostar
    
    // calcula la ganancia de la mejor jugada
    if(si >= no)
    	apuesta = si;
    else if(si < no)
    	apuesta = no;

    apuesta_compartida[turno-1] = apuesta;

    //printf("%d\n",apuesta);
    return apuesta;
}

/* Funcion que calcula el ganador de cada ronda y guarda el puntaje */
int calcularGanador(struct datos *data){
	int ronda, i, cant_parejas, ganador, puntaje; 
	ronda = data->ronda;
	cant_parejas = data->cant_parejas;
	ganador = 0; // variable para guardar el index de la pareja ganadora
	puntaje = apuestas[ronda][0];

	// Busca cual es la mayor apuesta 
	for(i=0; i<cant_parejas; i++){
		if(puntaje <= apuestas[ronda][i]){
			puntaje = apuestas[ronda][i];
			ganador = i;
		}
	}
	puntajes[ganador] += 100;
	printf("%d %d\n", ganador+1, puntaje); // imprime la pareja vencedora de la ronda y su ganancia en  ella 
	return 0;
}
	
/* Función que entrega una planificación a cada hebra, dandole la posicion que debe recorrer 
   para calcular su mejor jugada, la ronda en que actua la hebra, y el id de la pareja */
void planificar(Datos juego[], int cant_parejas, int cant_rondas){

	// se toma como supuesto que cada pareja esta constituida por dos numeros consecutivos, así para cada pareja
    
    // asiganción de parejas
	int index, i, j;
    index = 0;

	for(i=0; i<cant_rondas; i++){
		for(j=0; j<cant_parejas; j++){
			juego[index].pareja=j+1;
			juego[index+1].pareja=j+1;
			index+=2;
		}
		index+=1;
	}

	juego[cant_parejas*cant_rondas*2+cant_rondas].pareja=0;
    

    // asignación de turnos
    // turno = 1 corresponde a jugador 1
    // turno = 2 corresponde a jugador 2
    // turno = 3 corresponde a la hebra que calcula el ganador de ronda
    // turno = 4 corresponde a la hebra que entrega los resultados

    index = 0;

	for(i=0; i<cant_rondas; i++){
		for(j=0; j<cant_parejas; j++){
			juego[index].turno=1;
			juego[index+1].turno=2;
			index+=2;
		}
		juego[index].turno=3;
		index+=1;
	}

	juego[cant_parejas*cant_rondas*2+cant_rondas].turno=4;


	//asignacion de index para recorrer las jugadas posibles del jugador

	index = 0;

	for(i=0; i<cant_rondas; i++){
		for(j=0; j<cant_parejas; j++){
			juego[index].index=j*8;
			juego[index+1].index=j*8+4;
			index+=2;
		}
		juego[index].index=-1;
		index+=1;
	}

	juego[cant_parejas*cant_rondas*2+cant_rondas].index=-1;


	//asignacion de la ronda de cada hebra

	index = 0;

	for(i=0; i<cant_rondas; i++){
		for(j=0; j<cant_parejas; j++){
			juego[index].ronda=i;
			juego[index+1].ronda=i;
			index+=2;
		}
		juego[index].ronda=i;
		index+=1;
	}

	juego[cant_parejas*cant_rondas*2+cant_rondas].ronda=-1; // indica que corresponde al termino del juego

	/*for(i=0; i < cant_parejas*cant_rondas*2+cant_rondas+1; i++){
		printf("id: %d, pareja: %d, ronda: %d, turno; %d, index: %d \n", juego[i].id, juego[i].pareja, juego[i].ronda, juego[i].turno, juego[i].index);
	}*/

}

 
 /* Funcion Principal */
int main(int argc, char ** argv) {
	int aux1; // para guardar la cantidad de parejas
	int aux2; // para guardar la cantidad de rondas
	int total;

	scanf("%d %d", &aux1, &aux2); 
	//printf("cant_parejas: %d\n", aux1);
	//printf("cant_rondas: %d\n", aux2);

	/* El total de hebras va a estar dador por la suma de :
	   hebras_jugadoras = cantidad_parejas x cantidad_rondas X 2
	   hebra_final_ronda = cantidad_rondas
	   hebra_final = 1

	   total = hebras_jugadoras + hebra_final_ronda + hebra_final

	   hebras_jugadoras  representa todas las hebras que se necesitan para las desiciones de los jugadores en el juego
	   hebra_final_ronda representa las hebras que calculan el ganador de cada ronda
	   hebra_final imprime los puntajes finales

	*/

	total = aux1*aux2*2+aux2+1;

	Datos juego[total];
	//printf("total: %d\n", total);

	int i, j, n;
    
	parejas = (int **)malloc (aux1*sizeof(int *));
	// i representa el id de cada pareja
	// j guarda los jugadores por pareja

	for(i=0;i<aux1;i++){
        parejas[i] = (int *) malloc (2*sizeof(int));
    }
    
	for(i=0; i<aux1; i++){
		for(j=0; j<2; j++){
			if(scanf("%d", &n) == 1){  
                parejas[i][j] = n; // Guarda en la matriz cada pareja
		    }
		}
	}

    jugadas = (int **)malloc (aux2*sizeof(int *));
    // i representa las rondas 
    // j guarda las jugadas posibles de todos los jugadores tal como lo representa la entrada

    for(i=0;i<aux2;i++){
        jugadas[i] = (int *) malloc (aux1*2*4*sizeof(int));
    }

	for(i=0; i<aux2; i++){
		for(j=0; j<aux1*4*2; j++){
			if(scanf("%d", &n) == 1){
                jugadas[i][j] = n; // guarda todas las jugadas posibles en la matriz
		    }
		}
	}
	/*
	for(i=0; i<aux2; i++){
		for(j=0; j<aux1*4*2; j++){
			printf("%d ", jugadas[i][j]);
		}
		printf("\n");
	}
	*/

	apuestas = (int **)malloc (aux2*sizeof(int *));
	// i representa las rondas
	// j representa las parejas 

	for(i=0;i<aux2;i++){
        apuestas[i] = (int *) malloc (aux1*sizeof(int));
    }

    for(i=0; i<aux2; i++){
    	for(j=0; j<aux1; j++){
    		apuestas[i][j] = 0;
    	}
    }

    puntajes = (int *)malloc (aux1*sizeof(int *));
    // la posición del arreglo indica el id del equipo (id +1)

    for(i=0; i<aux1; i++){
    	puntajes[i] = 0;
    }

    // Inicializacion de arreglo de memoria compartida
    apuesta_compartida[0] = 0;
    apuesta_compartida[1] = 0;

    // Inicializacion de variables
	for(i=0; i<total; i++){
		juego[i].id = i;
		juego[i].cant_parejas = aux1;
		juego[i].cant_rondas = aux2;
		juego[i].index = 0;
		juego[i].pareja = 0;
		juego[i].ronda = 0;
		juego[i].turno = 0;
		juego[i].apuesta = 0;
	}

	/* for(i=0; i < total; i++){
		printf("id: %d, pareja: %d, ronda: %d, turno; %d, index: %d \n", juego[i].id, juego[i].pareja, juego[i].ronda, juego[i].turno, juego[i].index);
	} */

	planificar(juego, aux1, aux2);

	printf("\n");
    
    /* Para la sincronización con una lista de mutex, se utiliza el metodo de bloquear todas las hebras menos la primera, 
       para luego ir desbloqueando de una en una. 
       De esta manera se sigue uno de los ejemplos que se realizaron durante las ayudantias*/ 

    listaHilos = (pthread_t *) malloc(sizeof(pthread_t)*total);
    listaMutex = (pthread_mutex_t *)malloc(total*sizeof(pthread_mutex_t));

    for(i=0;i<total;i++){
		pthread_mutex_init(&listaMutex[i],NULL);
	}
	for(i=1;i<total;i++){
		pthread_mutex_lock(&listaMutex[i]);
	}
	for(i=0;i<total;i++){
		pthread_create(&listaHilos[i], NULL, (void *) jugar, &(juego[i]));
	}
	//Se espera que todas la hebras terminen
	for (i=0; i < total; i++) {
		pthread_join(listaHilos[i], NULL); //espera a que la hebra termine
	}
}
