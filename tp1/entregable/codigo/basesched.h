#ifndef __BASESCHED_H__
#define __BASESCHED_H__

#define IDLE_TASK -1
#include <vector>

enum Motivo { TICK, BLOCK, EXIT };

class SchedBase {
	public:

	virtual ~SchedBase() {};
	/* Constructor, recibe los parámetros pasados al scheduler como una lista de enteros. */

	/* load(pid) será llamada cuando una tarea nueva (pid) es creada.
		si deadline > 0, significa que la tarea debe terminarse antes de deadline
	*/
	virtual void load(int pid, int deadline);
	virtual void load(int pid) = 0;

	/* unblock(pid) será llamada cuando OTRA tarea (pid), previamente bloqueda,
	 * haya terminado la operación de I/O (pasa de BLOCKED a READY). */
	virtual void unblock(int pid) = 0;

	/* tick() será llamada con cada "tick" del reloj de la máquina (1 ms).
	 * El parámetro m indica si en la ejecución del último ms, la tarea:
	 *  - BLOCK: Ejecutó una syscall bloqueante
	 *  - EXIT: Terminó (return)
	 *  - TICL: Consumió el CPU todo el milisegundo.
	 * Esta función devuelve qué pid utilizará el CPU ahora, o IDLE_TASK. */
	virtual int tick(int cpu, const enum Motivo m) = 0;
};

/* Getters */
//int current_pid();
int current_pid(int cpu);
unsigned int current_time();

/* Factory (ignorar) */
template<typename T>
SchedBase& create_sched(const std::vector<int>& argn) { return ( new T(argn) ); }

#endif
