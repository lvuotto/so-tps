#include <signal.h>
#include <errno.h>

#include "biblioteca.h"

/* Estructura que almacena los datos de una reserva. */
typedef struct {
	int posiciones[ANCHO_AULA][ALTO_AULA];
	int cantidad_de_personas;
	pthread_mutex_t m_posiciones[ANCHO_AULA][ALTO_AULA];
	int rescatistas_disponibles;
	pthread_mutex_t m_rescatistas;
	pthread_mutex_t m_cant_personas;
	pthread_cond_t vc_rescatistas;
	int para_salir;
	pthread_cond_t vc_salida;
} t_aula;

typedef struct {
	int fd;
	t_aula* aula;
} thread_args;


void t_aula_iniciar_vacia(t_aula *un_aula)
{
	int i, j;
	for(i = 0; i < ANCHO_AULA; i++)
	{
		for (j = 0; j < ALTO_AULA; j++)
		{
			un_aula->posiciones[i][j] = 0;
			pthread_mutex_init(&un_aula->m_posiciones[i][j], NULL);
		}
	}
		
	pthread_mutex_init(&un_aula->m_rescatistas, NULL);
	pthread_mutex_init(&un_aula->m_cant_personas, NULL);
	
	pthread_cond_init(&un_aula->vc_rescatistas, NULL);
	un_aula->cantidad_de_personas = 0;
	un_aula->para_salir = 0;
	pthread_cond_init(&un_aula->vc_salida, NULL);
	
	un_aula->rescatistas_disponibles = RESCATISTAS;
}
/*
 * Lockeamos los mutex de la cantidad de personas y de posicion, antes de modificar las variables
 * */
void t_aula_ingresar(t_aula *un_aula, t_persona *alumno)
{
	pthread_mutex_lock(&un_aula->m_cant_personas);
	un_aula->cantidad_de_personas++;
	pthread_mutex_unlock(&un_aula->m_cant_personas);
	
	pthread_mutex_lock(&un_aula->m_posiciones[alumno->posicion_fila][alumno->posicion_columna]);	
	un_aula->posiciones[alumno->posicion_fila][alumno->posicion_columna]++;
	pthread_mutex_unlock(&un_aula->m_posiciones[alumno->posicion_fila][alumno->posicion_columna]);	
}

void t_aula_liberar(t_aula *un_aula, t_persona *alumno)
{
	pthread_mutex_lock(&un_aula->m_cant_personas);	
	un_aula->cantidad_de_personas--;
	pthread_mutex_unlock(&un_aula->m_cant_personas);	
}

static void terminar_servidor_de_alumno(int socket_fd, t_aula *aula, t_persona *alumno) {
	printf(">> Se interrumpió la comunicación con una consola.\n");
		
	close(socket_fd);
	
	t_aula_liberar(aula, alumno);
	exit(-1);
}


t_comando intentar_moverse(t_aula *el_aula, t_persona *alumno, t_direccion dir)
{
	int fila = alumno->posicion_fila;
	int columna = alumno->posicion_columna;
	alumno->salio = direccion_moverse_hacia(dir, &fila, &columna);

	///char buf[STRING_MAXIMO];
	///t_direccion_convertir_a_string(dir, buf);
	///printf("%s intenta moverse hacia %s (%d, %d)... ", alumno->nombre, buf, fila, columna);
	
	
	bool entre_limites = (fila >= 0) && (columna >= 0) &&
	     (fila < ANCHO_AULA) && (columna < ALTO_AULA);
	     
	bool pudo_moverse = alumno->salio ||
	    (entre_limites && el_aula->posiciones[fila][columna] < MAXIMO_POR_POSICION);
	
	
	if (pudo_moverse)
	{
		if (!alumno->salio){
			pthread_mutex_lock(&el_aula->m_posiciones[fila][columna]);	
			el_aula->posiciones[fila][columna]++;
			pthread_mutex_unlock(&el_aula->m_posiciones[fila][columna]);	
		}
		pthread_mutex_lock(&el_aula->m_posiciones[alumno->posicion_fila][alumno->posicion_columna]);	
		el_aula->posiciones[alumno->posicion_fila][alumno->posicion_columna]--;
		pthread_mutex_unlock(&el_aula->m_posiciones[alumno->posicion_fila][alumno->posicion_columna]);	
		
		alumno->posicion_fila = fila;
		alumno->posicion_columna = columna;
	}
	
	
	//~ if (pudo_moverse)
		//~ printf("OK!\n");
	//~ else
		//~ printf("Ocupado!\n");


	return pudo_moverse;
}

/**
 * Tener en cuenta que esto puede hacerse de otro modo.
 **/
void colocar_mascara(t_aula *el_aula, t_persona *alumno)
{
	printf("Esperando rescatista. Ya casi %s!\n", alumno->nombre);
	pthread_mutex_lock(&el_aula->m_rescatistas);
	while(el_aula->rescatistas_disponibles <= 0){
		pthread_cond_wait(&el_aula->vc_rescatistas, &el_aula->m_rescatistas);
	}
	el_aula->rescatistas_disponibles--;
	pthread_mutex_unlock(&el_aula->m_rescatistas);	
	pthread_cond_signal(&el_aula->vc_rescatistas);

	alumno->tiene_mascara = true;

	pthread_mutex_lock(&el_aula->m_rescatistas);
	el_aula->rescatistas_disponibles++;
	pthread_mutex_unlock(&el_aula->m_rescatistas);	
}


void atendedor_de_alumno(int socket_fd, t_aula *el_aula)
{
	t_persona alumno;
	t_persona_inicializar(&alumno);
	
	if (recibir_nombre_y_posicion(socket_fd, &alumno) != 0) {
		/* O la consola cortó la comunicación, o hubo un error. Cerramos todo. */
		terminar_servidor_de_alumno(socket_fd, NULL, NULL);
	}
	
	printf("Atendiendo a %s en la posicion (%d, %d)\n", 
			alumno.nombre, alumno.posicion_fila, alumno.posicion_columna);
		
	t_aula_ingresar(el_aula, &alumno);
	
	/// Loop de espera de pedido de movimiento.
	for(;;) {
		t_direccion direccion;
		
		/// Esperamos un pedido de movimiento.
		if (recibir_direccion(socket_fd, &direccion) != 0) {
			/* O la consola cortó la comunicación, o hubo un error. Cerramos todo. */
			terminar_servidor_de_alumno(socket_fd, el_aula, &alumno);
		}
		
		/// Tratamos de movernos en nuestro modelo
		bool pudo_moverse = intentar_moverse(el_aula, &alumno, direccion);

		printf("%s se movio a: (%d, %d)\n", alumno.nombre, alumno.posicion_fila, alumno.posicion_columna);

		/// Avisamos que ocurrio
		enviar_respuesta(socket_fd, pudo_moverse ? OK : OCUPADO);		
		//printf("aca3\n");
		
		if (alumno.salio)
			break;
	}
	
	colocar_mascara(el_aula, &alumno);

	t_aula_liberar(el_aula, &alumno);
	/* Esperamos a que haya 5 para salir. */
	/*pthread_mutex_lock(&el_aula->m_cant_personas);
	el_aula->para_salir++;
	while (el_aula->cant_personas > 0 && el_aula->para_salir < 5)
	  pthread_cond_wait(&el_aula->vc_salida, &el_aula->m_cant_personas);
	pthread_mutex_unlock(&el_aula->m_cant_personas);
	el_aula->*/
	enviar_respuesta(socket_fd, LIBRE);
	
	printf("Listo, %s es libre!\n", alumno.nombre);
}

void * start_routine(void* args){
	atendedor_de_alumno(((thread_args *) args)->fd, ((thread_args *) args)->aula);
	return NULL;
}


int main(void)
{
	//signal(SIGUSR1, signal_terminar);
	int socketfd_cliente, socket_servidor, socket_size;
	struct sockaddr_in local, remoto;

	

	/* Crear un socket de tipo INET con TCP (SOCK_STREAM). */
	if ((socket_servidor = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("creando socket");
	}

	/* Crear nombre, usamos INADDR_ANY para indicar que cualquiera puede conectarse aquí. */
	local.sin_family = AF_INET;
	local.sin_addr.s_addr = INADDR_ANY;
	local.sin_port = htons(PORT);
	
	if (bind(socket_servidor, (struct sockaddr *)&local, sizeof(local)) == -1) {
		perror("haciendo bind");
	}

	/* Escuchar en el socket y permitir 5 conexiones en espera. */
	if (listen(socket_servidor, 5) == -1) {
		perror("escuchando");
	}
	
	t_aula el_aula;
	t_aula_iniciar_vacia(&el_aula);
		
	/// Aceptar conexiones entrantes.
	socket_size = sizeof(remoto);
	for(;;){		
		if (-1 == (socketfd_cliente = 
					accept(socket_servidor, (struct sockaddr*) &remoto, (socklen_t*) &socket_size)))
		{			
			printf("!! Error al aceptar conexion\n");
		}
		else{
			pthread_t tid;
			thread_args args;
			args.fd = socketfd_cliente;
			args.aula = &el_aula;
			pthread_create(&tid, NULL, start_routine, &args);
		}
	}


	return 0;
}
