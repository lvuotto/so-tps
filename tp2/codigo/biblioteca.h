#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <pthread.h>

#define PORT 5555
#define MENSAJE_MAXIMO 1024
#define DATO_MAXIMO 100
#define STRING_MAXIMO 200

#define ANCHO_AULA 10
#define ALTO_AULA 10
#define FILA_SALIDA 0
#define COLUMNA_SALIDA -1
#define RESCATISTAS 3
#define MAXIMO_POR_POSICION 3



/* Enumerado que indica el estado de un asiento. */
enum e_direccion { ARRIBA, ABAJO, IZQUIERDA, DERECHA, DIRECCION_NULA /* para errores */ };
typedef enum e_direccion t_direccion;

t_direccion t_direccion_crear_de_string(char *direccion_str);
void t_direccion_convertir_a_string(t_direccion dir, char *direccion_str);
bool direccion_moverse_hacia(t_direccion dir, int *fila, int *columna);

enum e_comando { OK, OCUPADO, LIBRE, COMANDO_NULO /* para errores */ };
typedef enum e_comando t_comando;

char* t_comando_a_string(t_comando comando, char *buffer);


typedef struct {
	char nombre[STRING_MAXIMO];
	int  posicion_fila;
	int  posicion_columna;
	bool salio;
	bool tiene_mascara;
} t_persona;

void t_persona_inicializar(t_persona *persona);

int  enviar_nombre_y_posicion(int socketfd, t_persona *persona);
int recibir_nombre_y_posicion(int socketfd, t_persona *persona);

int enviar_direccion(int socketfd, t_direccion direccion);
int recibir_direccion(int socketfd, t_direccion *direccion);

int  enviar_respuesta(int socketfd, t_comando respuesta);
int esperar_respuesta(int socketfd, t_comando *respuesta);

int recibir(int s, char* buf);
int enviar(int s, char* buf);

