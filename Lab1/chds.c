/*
 ============================================================================
 Name        : LAB1.c
 Author      : Diego Miranda G., Benjamin Pastene P.
 Version     : v1.0
 Description : Dilation multi-hebras.
 ============================================================================
 */

#include "../encabezados/encabezado.h"

Imagen dilatar(Imagen img, char* outputFile, int hebrasNo, int opcion,
		int debug);
void* dilatarAux(void* args);

int main(int argc, char* argv[]) {
	int N, hebrasNo, debug, opcion, verificador, i, j;
	char* inputFile;
	char* outputFile;
	Imagen img, img_ret;
	verificador = 0;
	while ((opcion = getopt(argc, argv, "i:I:o:O:n:N:h:H:d:D:")) != -1) {
		switch (opcion) {
		case 'i':
		case 'I':
			inputFile = (char*) optarg;
			verificador++;
			break;
		case 'o':
		case 'O':
			outputFile = (char*) optarg;
			verificador++;
			break;
		case 'n':
		case 'N':
			N = atoi(optarg);
			verificador++;
			break;
		case 'h':
		case 'H':
			hebrasNo = atoi(optarg);
			verificador++;
			break;
		case 'd':
		case 'D':
			debug = atoi(optarg);
			verificador++;
			break;
		case '?':
			printf(
					ANSI_COLOR_AMARILLO "ADVERTENCIA!" ANSI_COLOR_NORMAL "Ingreso un parametro desconocido -%s, no sera utilizado\n",
					optarg);
		default:
			abort();
			break;
		}
	}

	if (verificador == 5) {

		cargar_archivo(inputFile, &img, N);
		opcion = 1;

		/*			DEBUG		*/
		if (debug == 1) {
			printf(
					ANSI_COLOR_CELESTE "DEBUG >> Imagen inicial:\n"ANSI_COLOR_NORMAL);
			for (i = 0; i < N; i++) {
				printf(ANSI_COLOR_CELESTE "DEBUG >> " ANSI_COLOR_NORMAL);
				for (j = 0; j < N; j++)
					printf(ANSI_COLOR_VERDE"%d"ANSI_COLOR_NORMAL,
							img.matriz_imagen[i][j]);

				printf("\n");
			}
		}
		/*		FIN DEBUG		*/

		//if opcion es 0, el trabajo es por dato si es 1 será por fila.
		img_ret = dilatar(img, outputFile, hebrasNo, opcion, debug);

		/*			DEBUG		*/
		if (debug == 1) {
			printf(
					ANSI_COLOR_CELESTE "DEBUG >> Imagen final:\n"ANSI_COLOR_NORMAL);
			for (i = 0; i < N; i++) {
				printf(ANSI_COLOR_CELESTE "DEBUG >> " ANSI_COLOR_NORMAL);
				for (j = 0; j < N; j++) {
					if (img.matriz_imagen[i][j] != img_ret.matriz_imagen[i][j])
						printf(ANSI_COLOR_AZUL"%d"ANSI_COLOR_NORMAL,
								img_ret.matriz_imagen[i][j]);
					else
						printf(ANSI_COLOR_VERDE"%d"ANSI_COLOR_NORMAL,
								img_ret.matriz_imagen[i][j]);
				}
				printf("\n");
			}
		}
		/*		FIN DEBUG		*/

	} else {
		printf(
				ANSI_COLOR_ROJO "ERROR >> "ANSI_COLOR_MORADO"Falta ingresar un dato.\n" ANSI_COLOR_NORMAL "-i -I -o -O -n -N -h -H -d -D son validos, pero debe haber al menos uno del par.\n");
		abort();
	}

	return 0;
}

// opcion indica si una hebra realiza como minimo una fila (1) o como minimo un dato o pixel (0)
// se asume que el main no utilizara ni modificara la imagen
Imagen dilatar(Imagen img, char* outputFile, int hebrasNo, int opcion,
		int debug) {
	FILE* archivo;
	int trabajo_total_hebras, trabajo_minimo_por_hebra, contador, resto, i = 0,
			j = 0;
	Imagen img_ret;
	Hebra arreglo_hebras[hebrasNo];
	void** args = malloc(sizeof(void*) * 5);
	int* numero = malloc(sizeof(int));

	// Dimension minima = 3

	// ====== PLANIFICANDO ==========//

	crear_imagen(&img_ret, img.dimension);

	for (i = 0; i < img.dimension; i++)
		for (j = 0; j < img.dimension; j++)
			img_ret.matriz_imagen[i][j] = img.matriz_imagen[i][j];

	// planificar como minimo una fila, se restan 2 filas (inicio y final), porque comienza en (1,1) termina en (n-1,n-1)
	if (opcion == 1)
		trabajo_total_hebras = img.dimension - 2; // 1,n-2
	// planificar como minimo un dato
	else
		trabajo_total_hebras = img.dimension * img.dimension
				- 4 * (img.dimension - 1);

	if (hebrasNo > 0)
		trabajo_minimo_por_hebra = trabajo_total_hebras / hebrasNo;

	else if (hebrasNo < 0)
		trabajo_minimo_por_hebra = trabajo_total_hebras / (-1 * hebrasNo);

	//error division por 0, No hay hebra auxiliar solo el main.
	else
		trabajo_minimo_por_hebra = -1;

	resto = trabajo_total_hebras - hebrasNo * trabajo_minimo_por_hebra;

	if (debug) {
		printf(
				ANSI_COLOR_CELESTE "DEEBG >> " ANSI_COLOR_VERDE"Trabajo minimo por hebra = %d, resto de trabajo %d, de un total de %d\n"ANSI_COLOR_NORMAL,
				trabajo_minimo_por_hebra, resto, trabajo_total_hebras);
	}

	contador = 0;
	while (contador < hebrasNo) {
		arreglo_hebras[contador].pid_hebra = contador;
		arreglo_hebras[contador].recorrerPorFila = opcion;
		// se le resta uno porque trabajo final es el elemento (fila o dato) ultimo hasta donde leera
		arreglo_hebras[contador].trabajo_final = trabajo_minimo_por_hebra - 1;
		contador++;
	}

	// se asigna equitativamente el resto a las hebras restantes, notar que siempre resto<No hebras
	if (resto != 0) {
		contador = 0;
		while (contador < resto) {
			arreglo_hebras[contador].trabajo_final++;
			contador++;
		}
	}


	arreglo_hebras[0].trabajo_inicial = 1;
	// como se debe comenzar desde la posicion 1,1 se suma 1,1 al vectir, esto implica sumar 1 al final y al inicio
	// inicio de la primera hebra siempre es 1.
	arreglo_hebras[0].trabajo_final++;
	// se continua en el siguiente elemento
	contador = 1; //itera entre 1 u m-1, donde m es la cantidad de hebras, en total m-2 veces
	while (contador < hebrasNo) {
		// para todo elemento menor a la cantidad de hebras [0,hebrasNo[ se calcula su trabajo inicial
		// como el final anterior + 1 (siguiente elemento no revisado)
		arreglo_hebras[contador].trabajo_inicial =
				arreglo_hebras[contador - 1].trabajo_final + 1;
		/* el elemento final debe llegar solo hasta el 11, por ejemplo si mi trabajo inicial es 11
		 * y mi trabajo que debo realizar (guardado por el momento en el trabajo final), corresponde a 1
		 * al calcular mi trabajo final real seria 12, correspondiente a la ultima fila, solo debe terminar en
		 * la 11 ava
		*/
		arreglo_hebras[contador].trabajo_final =
				arreglo_hebras[contador].trabajo_final
						+ arreglo_hebras[contador].trabajo_inicial;
		contador++;
	}

	contador = 0;
	while (contador < hebrasNo) {
		if (debug) {
			printf(
					ANSI_COLOR_CELESTE "DEBUG >> " ANSI_COLOR_VERDE"Hebra numero %d de %d con trabajo desde %d hasta %d\n"ANSI_COLOR_NORMAL,
					contador + 1, hebrasNo,
					arreglo_hebras[contador].trabajo_inicial,
					arreglo_hebras[contador].trabajo_final);

		}

		// ============ FIN PLANIFICACION ===============//

		*numero = contador;
		args[0] = (void*) numero;
		args[1] = (void*) &img;
		args[2] = (void*) &img_ret;
		args[3] = (void*) arreglo_hebras;

		if (debug)
			printf(
					ANSI_COLOR_CELESTE "DEBUG >> " ANSI_COLOR_VERDE "Creando hebra %d ...\n",
					contador + 1);

		pthread_create(&(arreglo_hebras[contador].pid_hijo), NULL, dilatarAux,
				(void*) args);

		if (debug)
			printf(
					ANSI_COLOR_CELESTE "DEBUG >> " ANSI_COLOR_VERDE "Hebra %d creada. Esperando que finalice ...\n",
					contador + 1);

		pthread_join(arreglo_hebras[contador].pid_hijo, NULL);

		if (debug)
			printf(
					ANSI_COLOR_CELESTE "DEBUG >> " ANSI_COLOR_VERDE "Hebra %d finalizo.\n",
					contador + 1);
		contador++;
	}

	if (debug)
		printf(
				ANSI_COLOR_CELESTE "DEBUG >> " ANSI_COLOR_VERDE "Escribiendo en archivo ...\n");
	archivo = fopen(outputFile, "wb");

	if (debug)
		printf(
				ANSI_COLOR_CELESTE "DEBUG >> " ANSI_COLOR_VERDE "Dimension = %d\n",
				img_ret.dimension);

	for (i = 0; i < img_ret.dimension; i++)
		for (j = 0; j < img_ret.dimension; j++)
			fwrite(&img_ret.matriz_imagen[i][j], sizeof(int), 1, archivo);

	if (debug)
		printf(
				ANSI_COLOR_CELESTE "DEBUG >> " ANSI_COLOR_VERDE "Finalizo la escritura de datos. Cerrando archivo ...\n");

	fclose(archivo);

	if (debug)
		printf(
				ANSI_COLOR_CELESTE "DEBUG >> " ANSI_COLOR_VERDE "Archivo cerrado exitosamente.\n");

	return img_ret;
}

void* dilatarAux(void* args) {
	int i, j;

	// dilata la imagen, la guarda en un documento de texto con un output desde una imagen, con el elemento de estructuracion.
	//elem_estructuracion elemento;
	//estruc_crear(&elemento);

	int hebraNo = *((int*) ((void**) args)[0]);
	Imagen img = *((Imagen*) ((void**) args)[1]);
	Imagen img_ret = *((Imagen*) ((void**) args)[2]);
	Hebra hebra = ((Hebra*) ((void**) args)[3])[hebraNo];
	// trabajo inicial comienza en 1, final en n-1
	if (hebra.recorrerPorFila) {
		for (i = hebra.trabajo_inicial; i <= hebra.trabajo_final; i++) {
			for (j = 1; j < img.dimension - 1; j++) {
				if (img.matriz_imagen[i - 1][j] || img.matriz_imagen[i][j - 1]
						|| img.matriz_imagen[i + 1][j]
						|| img.matriz_imagen[i][j + 1]
						|| img.matriz_imagen[i][j]) {
					//colocar 1
					img_ret.matriz_imagen[i][j] = 1;
				}
			}
		}
	} else {
		for (i = (hebra.trabajo_inicial / (img.dimension-2))+1;
				i <= img.dimension-2; i++) {
			for (j = (hebra.trabajo_inicial % (img.dimension-2)); j < (hebra.trabajo_final % (img.dimension-2)); j++) {
				printf("i = %d j = %d\n", i, j);
				if (img.matriz_imagen[i - 1][j] || img.matriz_imagen[i][j - 1]
						|| img.matriz_imagen[i + 1][j]
						|| img.matriz_imagen[i][j + 1]
						|| img.matriz_imagen[i][j]) {
					//colocar 1
					printf("coloque un dato\n", i, j);
					img_ret.matriz_imagen[i][j] = 1;
				}
			}
		}
	}

	return NULL;
}
