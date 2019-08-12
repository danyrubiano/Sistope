/*
Sistemas Operativos
Departamento de Ingenierıa en Informatica
LAB1: programacion con hebras Pthreads

Autores: Oscar Camilo Cerda Zuñiga 18541106-0
         Dany Efrain Rubiano Jimenez 22250855-k
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>   /* for pthreads */
#include <unistd.h>    /* for getopt */

/* Estructura de los parametro para la funcion dilatar, 
   de esta manera poder pasrlo como argumento a las hebras */
typedef struct param{
  int limitei;
  int limitej;
  int i;
  int j;
  int id;
  int **matriz;
  int **matriz2;
  int trabajo;
} Param;


/* Funcion que lee la imagen en el archivo y la guarda en la matriz */
void leeimagen(char archivo[], int **im, int dimension){
  FILE *fp;
  int ch;
  int i,j;
  i=0;
  j=0;
  fp=fopen(archivo, "rb");
  if ( fp == NULL ){
  printf("No puedo abrir el archivo \n");
  exit(1);
  }
  while(j<dimension){
    while(i<dimension){
      fread(&ch,sizeof(ch),1,fp);
      im[i][j]=ch;
      //printf("%i",ch );
      i=i+1;
    }
    //printf("\n");
    i=0;
    j=j+1;
  }
  fclose(fp);    
}


/* Función que aplica dilation sobre la imagen, 
   teniendo en cuenta el trabajo que realiza cada hebra */
void corrobora(struct param *dato){
  int i,j;
  i=dato->i;
  j=dato->j;
  //printf("entre con hebra %i\n",dato->id);
  while(j <dato->limitej){
    while(i <dato->limitei){
      if(dato->matriz[i][j]!=1&&(dato->matriz[i+1][j]==1||dato->matriz[i-1][j]==1||dato->matriz[i][j+1]==1||dato->matriz[i][j-1]==1)){
        dato->matriz2[i][j]=1;
      }
      i=i+1;
    }
    i=1;
    j=j+1;
  }         
}


/* Funcion que imprime la imagen por pantalla.
   Esto en el caso de que la opcion debug sea 1 */
void impresar(int **im, int dimension){
  int i,j;
  i=0;
  j=0;
  int ch;
  while(j<dimension){
    while(i<dimension){
      ch=im[i][j];
      printf("%i",ch );
      i=i+1;
    }
    printf("\n");
    i=0;
    j=j+1;
  }
}


/* Funcion que imprime la imagen en un archivo  */
void escribirImagen(char archivo[], int **matriz, int dimension){
  FILE *fp;
  int i, j;

  fp = fopen(archivo, "wb");
  i = 0;
  j = 0;
  while(j<dimension){
    while(i<dimension){
      fwrite(&matriz[i][j], sizeof(int), 1, fp);
      i=i+1;
    }
    i=0;
    j=j+1;
  }

  fclose(fp);
}


/* Funcion de planificación para calcualr el trabajo por cada hebra, de esta manera distribuir
   sus filas de recorrido*/
void planificacion(Param hebras[], int dimension, int cant_hebras, int **matriz, int **matriz2){
  int trabajo_hebra, resto, i;
  
  /* Para no tener problemas con los pıxeles de los bordes, 
  solo aplicaremos la operacion a los pıxeles entre [1, 1] y [N − 2, N − 2]. 

  Se toma en cuenta que NUMERO_DE_HEBRAS < DIMENSION - 2 */

  trabajo_hebra = (dimension-2) / cant_hebras;
  resto = (dimension-2) % cant_hebras;
  
  /* Inicializacion del id de la hebra y parametros iniciales */
  i = 0;
  for(i; i<cant_hebras; i++){
    hebras[i].id = i;
    hebras[i].trabajo = trabajo_hebra;
    hebras[i].matriz = matriz;
    hebras[i].matriz2 = matriz2;
  }
  /* Si resto no es cero quedan filas sin ser repartidas, 
  por lo tanto se le reparte el trabajo a algunas hebras */
  i = 0;
  if(resto != 0){
    while(i< resto){
      hebras[i].trabajo++;
      i++;
    }
  }

  /* Calculado el trabajo que realiza cada hebra, se procede a inicializar los limites y 
  index en los que opera cada hebra */
  hebras[0].i = 1;
  hebras[0].limitei = hebras[0].trabajo + 1;
  hebras[0].j = 1;
  hebras[0].limitej = dimension-1;

  i = 1;
  for(i; i<cant_hebras; i++){
    hebras[i].i = hebras[i-1].i + hebras[i-1].trabajo;
    hebras[i].limitei = hebras[i].i + hebras[i].trabajo;
    hebras[i].j = 1;
    hebras[i].limitej = dimension-1;
  }
  
  /*
  i=0;
  for(i; i<cant_hebras; i++){
    printf("%d, %d, %d, %d, %d, %d\n", hebras[i].limitei, hebras[i].limitej, hebras[i].i, hebras[i].j, hebras[i].id, hebras[i].trabajo);
  } */
}

int main(int argc, char* argv[]){

  char *arch_in;
  char *arch_out;
  int dimension, cant_hebras, op_debug, opcion, index;
  
  /* Uso de getopt() para los parametros en la linea de comandos */
  index = 0;
  while ((opcion = getopt(argc, argv, "i:I:o:O:n:N:h:H:d:D:")) != -1) {
    switch (opcion) {
    case 'i':
    case 'I':
      arch_in = (char*) optarg;
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
      printf("ERROR: Parámetro no válido, %s\n", optarg);
    default:
      abort();
      break;
    }
  }

  if(cant_hebras > dimension -2){
    printf("ERROR: Cantidad de hebras debe ser menor que dimension - 2\n");
    exit(1);
  }

  if (index != 5){
    printf("ERROR: Faltan parámetros en la línea de comandos\n");
    printf("USO: ./dilation -i imagen_entrada.raw -0 imagen_salida.raw -N ancho_imagen -H numero_hebras -D opcion_debug\n");
    exit(1);
  }

  /*int dimension, cant_hebras, op_debug;
  dimension = 12;
  cant_hebras = 9;
  op_debug = 1;
  */

  int **matriz;
  int **matriz2; 
  int i,fila,columna;
  fila=dimension;
  columna=dimension;
  matriz = (int **)malloc (fila*sizeof(int *));

  for(i=0;i<dimension;i++){
    matriz[i] = (int *) malloc (columna*sizeof(int));
  }
  
  matriz2 = (int **)malloc (fila*sizeof(int *));
      
  for (i=0;i<dimension;i++){
    matriz2[i] = (int *) malloc (columna*sizeof(int));
  }
  
  /* lectura de archivos */
  /*leeimagen("test1.raw", matriz, dimension);
  leeimagen("test1.raw", matriz2, dimension);*/

  leeimagen(arch_in, matriz, dimension);
  leeimagen(arch_in, matriz2, dimension);

  Param hebras[cant_hebras];
  
  /* planificación del trabajo por hebra */
  planificacion(hebras, dimension, cant_hebras, matriz, matriz2);
  

  /* Creacion de las hebras que aplicaran dilation sobre la imagen */
  pthread_t threads[cant_hebras];

  i = 0;
  for(i; i<cant_hebras; i++){

    pthread_create(&threads[i], NULL, (void*)corrobora, &(hebras[i]));

    pthread_join(threads[i], NULL);
  }
  

  /* Opcion de imprimir por pantalla */
  if(op_debug == 1){
    printf("Imagen original:\n\n");
    impresar(matriz, dimension);
    printf("\n\nImagen con dilation:\n\n");
    impresar(matriz2, dimension);
    printf("\n");
  }
  
  /* Escribe la imagen en el archivo con nombre dado */
  escribirImagen(arch_out, matriz2, dimension);
	
	return 0;
}
