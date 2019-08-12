#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>   /* for threads */
#include <unistd.h>    /* for getopt */

/* Estructura de Hebra */
typedef struct hebra{
	int id;
	int fila_i;
	int fila_f;
	int trabajo;
} Hebra;

/* Estructura de imagen */
typedef struct imagen{
	int **matriz;
	int dimension;
} Imagen;

/* Estructura de los parametro para la funcion dilatar, 
   de esta manera poder pasrlo como argumento a las hebras */
/*typedef struct params{
	int i;
	Imagen img_i;
	Imagen img_f;
	Hebra hebra[];
} Params;
*/

/* Funcion que lee la imagen en el archivo y la guarda en la matriz */
void leerimagen(char archivo[], Imagen img){
  FILE *fp;
  int ch;
  int i, j;
  
  //fp = fopen("text1.raw", "rb");
  fp = fopen(archivo, "rb");

  if ( fp == NULL ){
    printf("No puedo abrir el archivo \n");
    exit(1);
  }
  
  i = 0;
  j = 0;
  for(i; i< img.dimension; i++){
    for(j; j<img.dimension; j++){
      fread(&ch,sizeof(ch),1,fp);
      img.matriz[i][j] = ch;
      printf("%i",ch );
    }
    printf("\n");
  }
  fclose(fp);    
}

/* Funcion que imprime la imagen por pantalla.
   Esto en el caso de que la opcion debug sea 1 */
void imprimir(Imagen img){
  int i,j;
  i=0;
  j=0;
  for(i; i<img.dimension; i++) {
    for(j; j<img.dimension; j++) {
    	printf("%i", img.matriz[i][j]);
    }
    printf("\n");
  }
}

void escribirImagen(char archivo[], Imagen img){
	FILE *fp;
	int i, j;

	fp = fopen(archivo, "wb");
	//fp = fopen("hola.raw", "wb");
	i = 0;
	j = 0;
	for (i; i < img.dimension; i++){
		for (j; j < img.dimension; j++){
			fwrite(&img.matriz[i][j], sizeof(int), 1, fp);
		}
	}

	fclose(fp);

}

/* Funcion de planificación para calcualr el trabajo por cada hebra, de esta manera distribuir
   sus filas de recorrido*/
void planificar(Hebra hebras[], int dimension, int cant_hebras){
	int trabajo_hebra, resto, i;

	trabajo_hebra = (dimension-2) / cant_hebras;
	resto = (dimension-2) % cant_hebras;
	
	i = 0;
  for(i; i<cant_hebras; i++){
  	hebras[i].id = i;
  	hebras[i].trabajo = trabajo_hebra;
  }
  
  i = 0;
  if(resto != 0){
  	while(i< resto){
  		hebras[i].trabajo++;
  		i++;
  	}
  }

  hebras[0].fila_i = 1;
  hebras[0].fila_f = hebras[0].trabajo;

  i = 1;
  for(i; i<cant_hebras; i++){
  	hebras[i].fila_i = hebras[i-1].fila_f + 1;
  	hebras[i].fila_f = hebras[i].fila_i + hebras[i].trabajo - 1;
  }
}

/* Función que aplica dilation sobre la imagen, 
   teniendo en cuenta el trabajo que realiza cada hebra */
void *dilatar(void *params) {

	Imagen img_i = *((Imagen *) ((void **) params)[0]);
	Imagen img_f = *((Imagen *) ((void **) params)[1]);
	Hebra hebra = *((Hebra*) ((void**) params)[2]);

	int i, j;

	for (i = hebra.fila_i; i <= hebra.fila_f; i++) {
		for (j = 1; j < img_i.dimension - 1; j++) {
			if (img_i.matriz[i - 1][j] == 1 || img_i.matriz[i][j - 1] == 1 || img_i.matriz[i + 1][j] == 1 || img_i.matriz[i][j + 1] == 1 || img_i.matriz[i][j] == 1){
					img_f.matriz[i][j] = 1;
			}
		}
	}
}

int main(int argc, char* argv[]) {

	char *arch_in;
	char *arch_out;
	int dimension, cant_hebras, op_debug, opcion, index;

	/*index = 0;
	while ((opcion = getopt(argc, argv, "i:I:o:O:n:N:h:H:d:D:")) != -1) {
		switch (opcion) {
		case 'i':
		case 'I':
			arch_sin = (char*) optarg;
			index ++;
			break;
		case 'o':
		case 'O':
			arch_out = (char*) optarg;
			index ++;
			break;
		case 'n':
		case 'N':
			dimension = atoi(optarg);
			index ++;
			break;
		case 'h':
		case 'H':
			cant_hebras = atoi(optarg);
			index ++;
			break;
		case 'd':
		case 'D':
			op_debug = atoi(optarg);
			index ++;
			break;
		case '?':
			printf("Parametro no válido, %s", optarg);
		default:
			abort();
			break;
		}
	}

	if (index != 5){
		printf("Faltan parametros en la linea de comandos\n");
		exit(1);
	}*/
  ////////////////////////////////////7
  //preubas
	dimension = 12;
	op_debug = 1;
	cant_hebras = 3;
	////////////////////////////////////777

	Imagen img_i, img_f;

	img_i.dimension = dimension;
	img_f.dimension = dimension;

	img_i.matriz = (int **)malloc (dimension*sizeof(int *));
	img_f.matriz = (int **)malloc (dimension*sizeof(int *));

	int i;
	i = 0;
  for (i; i<dimension; i++){
    img_i.matriz[i] = (int *) malloc (dimension*sizeof(int));
    img_f.matriz[i] = (int *) malloc (dimension*sizeof(int));
  }

  void** aux = malloc(sizeof(void*) * 3);
	
	leerimagen("text1.raw", img_i);

	aux[0] = (void *) &img_i;

  Hebra hebras[cant_hebras];

  planificar(hebras, dimension, cant_hebras);


  i = 0;
  for(i; i<cant_hebras; i++){
  	aux[1] = (void *) &img_f;
    aux[2] = (void *) &hebras[i]; 	 	

  	pthread_create(&(hebras[i].id), NULL, dilatar, (void *) aux);

  	pthread_join(hebras[i].id, NULL);
  }
  

  if(op_debug == 1){

  	printf("Imagen Inicial:\n\n");
  	imprimir(img_i);

  	printf("\n\n\nImagen Final:\n\n");
  	imprimir(img_f);
  }

}