#include "biblioteca.h"

static void cleanup(int s);

int crear_conexion(int argc, char *argv[])
{
	struct sockaddr_in remote;
	int s;
	/* Crear un socket de tipo INET con TCP (SOCK_STREAM). */
	if ((s = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("creando socket");
		cleanup(s);
	}

	/* Establecer la dirección a la cual conectarse. */
	if (inet_aton(argv[1], &(remote.sin_addr)) == 0) {
		fprintf(stderr, "%s: host desconocido\n", argv[1]);
		cleanup(s);
	}
	remote.sin_family = AF_INET;
	remote.sin_port = htons(PORT);

   	/* Conectarse. */
	if (connect(s, (struct sockaddr *)&remote, sizeof(remote)) == -1) {
		perror("conectandose");
		cleanup(s);
	}

	return s;
}

int main(int argc, char *argv[])
{
	/* Comprobar argumentos de línea de comandos. */
	if(argc < 2) {
		printf("Uso: cliente host\n");
		return 0;
	}

	int s = crear_conexion(argc, argv);
	t_persona el_alumno;

	printf("Ingrese el nombre del alumno: ");
	fgets(el_alumno.nombre, STRING_MAXIMO, stdin);
	
	printf("Ingrese la fila inicial: ");
	scanf("%d", &el_alumno.posicion_fila);
	
	printf("Ingrese la columna inicial: ");
	scanf("%d", &el_alumno.posicion_columna);
	
	if (enviar_nombre_y_posicion(s, &el_alumno) != 0) {
		/* Hubo un error o el servidor cortó la comunicación. Cerramos todo. */
		cleanup(s);
	}
	
	/* Loop de movimiento. */
	for(;;) {
		int item = 0;
		printf("Indique hacia donde desea moverse:\n");
		printf("1 - Arriba\n");
		printf("2 - Abajo\n");
		printf("3 - Izquierda\n");
		printf("4 - Derecha\n");
		
		scanf("%d", &item);
		t_direccion direccion = item - 1;
		if(direccion < 0 || direccion > 3) {
			printf(">> Ingrese una direccion valida por favor (zopenco!).\n");
			continue;
		}
		
		
		if (enviar_direccion(s, direccion) != 0)
			cleanup(s);
		
		t_comando comando;	
		if (esperar_respuesta(s, &comando) != 0)
			cleanup(s);
		
		if (comando == OK)
			direccion_moverse_hacia(direccion, &el_alumno.posicion_fila, &el_alumno.posicion_columna);
		
		char buf[STRING_MAXIMO]; t_comando_a_string(comando, buf);
		printf("Respuesta: <<%s>>\n", buf);
		
		if (el_alumno.posicion_fila    == FILA_SALIDA &&
		    el_alumno.posicion_columna == COLUMNA_SALIDA )
		    break; // ya salio del aula.

	}

	t_comando comando;

	/// Esperamos que nos den la mascara
	if (esperar_respuesta(s, &comando) != 0)
		cleanup(s);
		
	if (comando != LIBRE)
		perror("El server esta diciendo sandeces!\n");
		
	printf("Ya tengo mascara, soy libre!\n");

	return 0;
}

static void cleanup(int s) {
	printf(">> Se interrumpió la comunicación con el servidor.\n");
	close(s);
	exit(1);
}
