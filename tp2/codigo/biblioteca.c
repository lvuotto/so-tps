
#include "biblioteca.h"

t_direccion t_direccion_crear_de_string(char *direccion_str)
{
	if (strncmp(direccion_str, "IZQUIERDA", MENSAJE_MAXIMO) == 0)
		return IZQUIERDA;
	
	if (strncmp(direccion_str, "DERECHA", MENSAJE_MAXIMO) == 0)
		return DERECHA;
		
	if (strncmp(direccion_str, "ARRIBA", MENSAJE_MAXIMO) == 0)
		return ARRIBA;
		
	if (strncmp(direccion_str, "ABAJO", MENSAJE_MAXIMO) == 0)
		return ABAJO;
		
	return DIRECCION_NULA;
}

void t_direccion_convertir_a_string(t_direccion dir, char *direccion_str)
{
	if (dir == IZQUIERDA)
		strncpy(direccion_str, "IZQUIERDA", 30);
	
	else if (dir == DERECHA)
		strncpy(direccion_str, "DERECHA", 30);
		
	else if (dir == ARRIBA)
		strncpy(direccion_str, "ARRIBA", 30);
		
	else if (dir == ABAJO)
		strncpy(direccion_str, "ABAJO", 30);

}

bool direccion_moverse_hacia(t_direccion dir, int *fila, int *columna)
{
	int f = *fila, c = *columna;
	if (dir == ARRIBA)
		f--;
	else if (dir == ABAJO)
		f++;
	else if (dir == IZQUIERDA)
		c--;
	else if (dir == DERECHA)
		c++;
	
	*fila = f;
	*columna = c;
	
	if (f == FILA_SALIDA && c == COLUMNA_SALIDA)
		return true;
	else
		return false;
}

t_comando t_comando_crear_de_string(char *comando_str)
{
	if (strncmp(comando_str, "OK", MENSAJE_MAXIMO) == 0)
		return OK;
	
	if (strncmp(comando_str, "OCUPADO", MENSAJE_MAXIMO) == 0)
		return OCUPADO;
		
	if (strncmp(comando_str, "LIBRE!", MENSAJE_MAXIMO) == 0)
		return LIBRE;
		
	return COMANDO_NULO;
}

char* t_comando_a_string(t_comando comando, char *buffer)
{
	if (comando == OK)
		snprintf(buffer, MENSAJE_MAXIMO, "OK");
	else if (comando == OCUPADO)
		snprintf(buffer, MENSAJE_MAXIMO, "OCUPADO");
	else if (comando == LIBRE)
		snprintf(buffer, MENSAJE_MAXIMO, "LIBRE!");
		
	return buffer;
}


void t_persona_inicializar(t_persona *persona)
{
	strncpy(persona->nombre, "Sin Nombre", STRING_MAXIMO);
	persona->posicion_fila    = -1;
	persona->posicion_columna = -1;
	
	persona->salio = false;
	persona->tiene_mascara = false;
}


int enviar_nombre_y_posicion(int socketfd, t_persona *persona)
{
	char buf[MENSAJE_MAXIMO+1];
	sprintf(buf, "NOMBRE: %s FILA: %d COLUMNA: %d\n",
				persona->nombre,
				persona->posicion_fila,
				persona->posicion_columna);
	return enviar(socketfd, buf);
}

int recibir_nombre_y_posicion(int socket_fd, t_persona *persona)
{
	char buf[MENSAJE_MAXIMO+1];

	if (recibir(socket_fd, buf) != 0) {
		return -1;
	}
	
	int res = sscanf(buf, "NOMBRE: %40s FILA: %d COLUMNA: %d\n",
				persona->nombre,
				&persona->posicion_fila,
				&persona->posicion_columna);
				
	if (res == EOF || res != 3)
	{
		printf("Error leyendo informacion del cliente. Res dio %d\n", res);
		return -1;
	}
	return 0;
}

int enviar_direccion(int socketfd, t_direccion direccion)
{
	char buf[MENSAJE_MAXIMO+1];
	t_direccion_convertir_a_string(direccion, buf);
	return enviar(socketfd, buf);
}

int recibir_direccion(int socketfd, t_direccion *direccion)
{
	char buf[MENSAJE_MAXIMO+1];

	if (recibir(socketfd, buf) != 0)
		return -1;

	*direccion = t_direccion_crear_de_string(buf);

	return 0;
}

int  enviar_respuesta(int socketfd, t_comando respuesta)
{
	char buf[MENSAJE_MAXIMO+1];
	t_comando_a_string(respuesta, buf);
	return enviar(socketfd, buf);
}

int esperar_respuesta(int socketfd, t_comando *respuesta)
{
	char buf[MENSAJE_MAXIMO+1];

	if (recibir(socketfd, buf) != 0)
		return -1;
	
	*respuesta = t_comando_crear_de_string(buf);
				
	return 0;
}


__thread char colabuf[MENSAJE_MAXIMO*2+1] = { 0 };

int recibir(int s, char* buf) {
	
	int i = 0;
	
	while (colabuf[i] != '|' && colabuf[i] != 0)
		i++;
		
	//~ printf("colabuf: %s, (%d)\n", colabuf, i);


	if (colabuf[i] == 0) // el mensaje fue cortado (porque si no habria un |)
	{
		//~ printf("Receiving...");
		ssize_t n = recv(s, &colabuf[i], MENSAJE_MAXIMO, 0);

		if (n == 0) 
			return -1; /* Se terminó la conexión. */
		if (n < 0) { 
			printf(">> Error recibiendo %s\n", buf);
			return -2; /* Se produjo un error. */
		}
		
		colabuf[n+i] = '\0'; /* Agregar caracter de fin de cadena a lo recibido. */
		//printf("received: %s, (%d bytes)\n", &colabuf[i], n);
		
		while (colabuf[i] != '|' && colabuf[i] != 0)
			i++;
	}
	
	strncpy(buf, colabuf, i);
	buf[i] = 0;
	
	while (colabuf[++i] == '\n');
		
	int j = 0;
	while (colabuf[i] != 0)
	{
		colabuf[j] = colabuf[i];
		i++; j++;
	}
	
	colabuf[j] = 0;
	//~ printf("<<BUF>> %s", buf);
	//~ printf("<<COLABUF>> %s", buf);
	return 0;
}

int enviar(int s, char* buf)
{
	int  len = strlen(buf);
	char env_buf[MENSAJE_MAXIMO+3];
	
	strncpy(env_buf, buf, MENSAJE_MAXIMO);
	env_buf[len] = '|';
	env_buf[len+1] = '\n';
	env_buf[len+2] = 0;
	
	ssize_t n = send(s, env_buf, len + 1, 0);
	if (n < 0) {
		printf(">> Error enviando %s", buf);
		return -2; /* Se produjo un error. */
	}
	return 0;
}

