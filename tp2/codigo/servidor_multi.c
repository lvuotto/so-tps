#include <signal.h>
#include <errno.h>
#include "biblioteca.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>


/* Estructura que almacena los datos de una reserva. */
typedef struct {
  int posiciones[ANCHO_AULA][ALTO_AULA];
  int cantidad_de_personas;
  int rescatistas_disponibles;
  int para_salir;
  bool saliendo;
  pthread_mutex_t m_posiciones[ANCHO_AULA][ALTO_AULA];
  pthread_mutex_t m_rescatistas;
  pthread_mutex_t m_estado;
  pthread_cond_t vc_rescatistas;
  pthread_cond_t vc_estado;
} t_aula;

typedef struct {
  int fd;
  t_aula* aula;
} thread_args;


void t_aula_iniciar_vacia(t_aula *un_aula)
{
  for (int i = 0; i < ANCHO_AULA; i++) {
    for (int j = 0; j < ALTO_AULA; j++) {
      un_aula->posiciones[i][j] = 0;
      pthread_mutex_init(&un_aula->m_posiciones[i][j], NULL);
    }
  }

  un_aula->cantidad_de_personas = 0;
  un_aula->para_salir = 0;
  un_aula->rescatistas_disponibles = RESCATISTAS;

  un_aula->saliendo = false;

  pthread_mutex_init(&un_aula->m_rescatistas, NULL);
  pthread_mutex_init(&un_aula->m_estado, NULL);

  pthread_cond_init(&un_aula->vc_rescatistas, NULL);
  pthread_cond_init(&un_aula->vc_estado, NULL);
}


/**
 * Lockeamos los mutex de la cantidad de personas y de posición, antes de
 * modificar las variables.
 **/
void t_aula_ingresar(t_aula *un_aula, t_persona *alumno)
{
  pthread_mutex_lock(&un_aula->m_estado);
  un_aula->cantidad_de_personas++;
  pthread_mutex_unlock(&un_aula->m_estado);

  pthread_mutex_lock(&un_aula->m_posiciones[alumno->posicion_fila][alumno->posicion_columna]);
  un_aula->posiciones[alumno->posicion_fila][alumno->posicion_columna]++;
  pthread_mutex_unlock(&un_aula->m_posiciones[alumno->posicion_fila][alumno->posicion_columna]);
}


void t_aula_liberar(t_aula *un_aula, t_persona *alumno)
{
  pthread_mutex_lock(&un_aula->m_estado);
  un_aula->cantidad_de_personas--;
  pthread_mutex_unlock(&un_aula->m_estado);

  pthread_mutex_lock(&un_aula->m_posiciones[alumno->posicion_fila][alumno->posicion_columna]);
  un_aula->posiciones[alumno->posicion_fila][alumno->posicion_columna]--;
  pthread_mutex_unlock(&un_aula->m_posiciones[alumno->posicion_fila][alumno->posicion_columna]);
}


static void terminar_servidor_de_alumno(int socket_fd, t_aula *aula, t_persona *alumno) {
  printf(">> Se interrumpió la comunicación con una consola.\n");

  close(socket_fd);
  if (aula != NULL)
    t_aula_liberar(aula, alumno);

  pthread_exit(NULL);
}


t_comando intentar_moverse(t_aula *aula, t_persona *alumno, t_direccion dir)
{
  int fila = alumno->posicion_fila;
  int columna = alumno->posicion_columna;
  alumno->salio = direccion_moverse_hacia(dir, &fila, &columna);

  bool entre_limites = fila >= 0 && columna >= 0 &&
                       fila < ANCHO_AULA && columna < ALTO_AULA;

  bool pudo_moverse = alumno->salio ||
                     (entre_limites &&
                      aula->posiciones[fila][columna] < MAXIMO_POR_POSICION);

  /* De nuevo, ¿no hay que pedir ambos mutexes a la vez? */
  if (pudo_moverse) {
    if (!alumno->salio) {
      int i1, i2, j1, j2;

      /**
       * Ordenamiento para pedir mutexes y evitar deadlocks.
       * Se considerará
       *      (a, b) < (c, d) <=> a < c ó a = c y b < d
       * De este modo, al pedir siempre con un mismo orden los mutexes, se
       * evita el deadlock pues elimina la espera circular.
       **/
      if (fila < alumno->posicion_fila ||
          (fila == alumno->posicion_fila &&
           columna < alumno->posicion_columna))
      {
        /* La nueva posición es "menor" que la vieja, entonces voy a pedir
         * primero su mutex. */
        i1 = fila;
        j1 = columna;
        i2 = alumno->posicion_fila;
        j2 = alumno->posicion_columna;
      } else {
        /* La nueva posición es "mayor" que la vieja, entonces voy a pedir
         * primero el mutex de la posición vieja. */
        i1 = alumno->posicion_fila;
        j1 = alumno->posicion_columna;
        i2 = fila;
        j2 = columna;
      }

      pthread_mutex_lock(&aula->m_posiciones[i1][j1]);
      pthread_mutex_lock(&aula->m_posiciones[i2][j2]);
      aula->posiciones[fila][columna]++;
      aula->posiciones[alumno->posicion_fila][alumno->posicion_columna]--;
      pthread_mutex_unlock(&aula->m_posiciones[i2][j2]);
      pthread_mutex_unlock(&aula->m_posiciones[i1][j1]);
    } else {
      pthread_mutex_lock(&aula->m_posiciones[alumno->posicion_fila][alumno->posicion_columna]);
      aula->posiciones[alumno->posicion_fila][alumno->posicion_columna]--;
      pthread_mutex_unlock(&aula->m_posiciones[alumno->posicion_fila][alumno->posicion_columna]);
    }

    alumno->posicion_fila = fila;
    alumno->posicion_columna = columna;
  }

  return pudo_moverse;
}


/**
 * Tener en cuenta que esto puede hacerse de otro modo.
 **/
void colocar_mascara(t_aula *el_aula, t_persona *alumno)
{
  printf("Esperando rescatista. Ya casi %s!\n", alumno->nombre);
  pthread_mutex_lock(&el_aula->m_rescatistas);
  while(el_aula->rescatistas_disponibles <= 0)
    pthread_cond_wait(&el_aula->vc_rescatistas, &el_aula->m_rescatistas);
  el_aula->rescatistas_disponibles--;
  pthread_cond_signal(&el_aula->vc_rescatistas);
  pthread_mutex_unlock(&el_aula->m_rescatistas);

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
         alumno.nombre,
         alumno.posicion_fila,
         alumno.posicion_columna);

  t_aula_ingresar(el_aula, &alumno);

  /* Loop de espera de pedido de movimiento. */
  while (1) {
    t_direccion direccion;

    /* Esperamos un pedido de movimiento. */
    if (recibir_direccion(socket_fd, &direccion) != 0) {
      /* O la consola cortó la comunicación, o hubo un error. Cerramos todo. */
      terminar_servidor_de_alumno(socket_fd, el_aula, &alumno);
    }

    /* Tratamos de movernos en nuestro modelo */
    bool pudo_moverse = intentar_moverse(el_aula, &alumno, direccion);

    printf("%s se movio a: (%d, %d)\n", alumno.nombre, alumno.posicion_fila, alumno.posicion_columna);

    /* Avisamos qué ocurrió. */
    enviar_respuesta(socket_fd, pudo_moverse ? OK : OCUPADO);

    if (alumno.salio) break;
  }

  colocar_mascara(el_aula, &alumno);

  /* No se usa `t_aula_liberar` porque ahora se realiza todo bajo un mutex. */

  /* Si hay personas saliendo, espero sin modificar las cantidades. */
  pthread_mutex_lock(&el_aula->m_estado);
  while (el_aula->saliendo) {
    pthread_cond_wait(&el_aula->vc_estado, &el_aula->m_estado);
  }

  /* Una vez que sé que no hay gente saliendo, me anoto para salir. */
  el_aula->cantidad_de_personas--;
  el_aula->para_salir++;

  /* Si soy el quinto o el último, empezamos a salir. */
  el_aula->saliendo = el_aula->para_salir == 5 ||
                      el_aula->cantidad_de_personas == 0;

  while (!el_aula->saliendo) {
    pthread_cond_wait(&el_aula->vc_estado, &el_aula->m_estado);
  }
  el_aula->para_salir--;
  pthread_cond_broadcast(&el_aula->vc_estado);
  /* Si somos 0, habilitamos a la gente a entrar, migo. */
  el_aula->saliendo = el_aula->para_salir != 0;
  pthread_mutex_unlock(&el_aula->m_estado);

  enviar_respuesta(socket_fd, LIBRE);
  printf("Listo, %s es libre!\n", alumno.nombre);
}


void * start_routine(void* args)
{
  thread_args arg = *(thread_args *) args;
  free(args);
  atendedor_de_alumno(arg.fd, arg.aula);
  return NULL;
}


void destruir_aula(t_aula* aula)
{
  if (aula != NULL) {
    /* Destruyo los mutexes de la matriz. */
    for (int i = 0; i < ANCHO_AULA; i++)
      for (int j = 0; j < ALTO_AULA; j++)
        pthread_mutex_destroy(&aula->m_posiciones[i][j]);

    /* Destruyo los mutex. */
    pthread_mutex_destroy(&aula->m_rescatistas);
    pthread_mutex_destroy(&aula->m_estado);

    /* Destruyo las variables de condición. */
    pthread_cond_destroy(&aula->vc_rescatistas);
    pthread_cond_destroy(&aula->vc_estado);
  }
}


void servidor(t_aula *aula)
{
  int socketfd_cliente, socket_servidor, socket_size;
  struct sockaddr_in local, remoto;

  /* Crear un socket de tipo INET con TCP (SOCK_STREAM). */
  socket_servidor = socket(AF_INET, SOCK_STREAM, 0);
  if (socket_servidor < 0) {
    perror("creando socket");
    exit(3);
  }

  /* Crear nombre, usamos INADDR_ANY para indicar que cualquiera puede conectarse aquí. */
  local.sin_family = AF_INET;
  local.sin_addr.s_addr = INADDR_ANY;
  local.sin_port = htons(PORT);

  if (bind(socket_servidor, (struct sockaddr *)&local, sizeof(local)) < 0) {
    perror("haciendo bind");
    exit(3);
  }

  /* Escuchar en el socket y permitir 5 conexiones en espera. */
  if (listen(socket_servidor, 1000) < 0) {
    perror("escuchando");
    exit(3);
  }

  /* Aceptar conexiones entrantes. */
  socket_size = sizeof(remoto);
  while (1) {
    socketfd_cliente = accept(socket_servidor,
                              (struct sockaddr *) &remoto,
                              (socklen_t *) &socket_size);
    if (socketfd_cliente < 0) {
      printf("!! Error al aceptar conexion\n");
    } else {
      thread_args *args = (thread_args *) malloc(sizeof(*args));
      args->fd = socketfd_cliente;
      args->aula = aula;

      pthread_t tid;
      pthread_attr_t attr;

      pthread_attr_init(&attr);
      pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
      pthread_create(&tid, &attr, start_routine, args);
      pthread_attr_destroy(&attr);
    }
  }

  exit(3);
}


int main()
{
  t_aula el_aula;
  t_aula_iniciar_vacia(&el_aula);

  pid_t serv_pid = fork();

  if (serv_pid < 0) {
    perror("fork");
    exit(1);
  } else if (serv_pid == 0) {
    servidor(&el_aula);
    return 0;
  }

  int stat;
  if (wait(&stat) < 0)
    perror("wait");
  destruir_aula(&el_aula);

  return 0;
}
