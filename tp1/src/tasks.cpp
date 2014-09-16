#include "tasks.h"
#include <cstdlib>
using namespace std;


inline int get_rand(int bmin, int bmax){
     return rand() % (bmax - bmin + 1) + bmin;    
}
//EJERCICIO 1
void TaskConsola(int pid,vector<int> input){
    int n = input[0];
    int bmin = input[1];
    int bmax = input[2];
    
    for(int i=0; i < n; i++){
        int t = get_rand(bmin,bmax);
        uso_IO(pid,t);
    }
}

//EJERCICIO 6
/**
 * Para poder correr los n bloqueos de forma aleatoria
 * generamos un arreglo de bools 
 * que nos indica que hacer en cada ciclo de clock.
 **/

void TaskBatch(int pid, vector<int> input){
    int cpu_total = input[0];
    int cant_bloqueos = input[1];
    vector<bool> bloqueos(cpu_total, false);

    for (int i = 0; i < cant_bloqueos; i++){
        int r;
        do {
            r = get_rand(0, cpu_total - 1);
        } while (bloqueos[r]);
        bloqueos[r] = true;
    }
    for (int i = 0; i < cpu_total; i++){
        if (bloqueos[i]){
            uso_IO(pid,1);
        } else {
            uso_CPU(pid,1);
        }
    }
}


void TaskCPU(int pid, vector<int> params) { // params: n
	uso_CPU(pid, params[0]); // Uso el CPU n milisegundos.
}

void TaskIO(int pid, vector<int> params) { // params: ms_pid, ms_io,
	uso_CPU(pid, params[0]); // Uso el CPU ms_pid milisegundos.
	uso_IO(pid, params[1]); // Uso IO ms_io milisegundos.
}

void TaskAlterno(int pid, vector<int> params) { // params: ms_pid, ms_io, ms_pid, ...
	for(int i = 0; i < (int)params.size(); i++) {
		if (i % 2 == 0) uso_CPU(pid, params[i]);
		else uso_IO(pid, params[i]);
	}
}



void tasks_init(void) {
	/* Todos los tipos de tareas se deben registrar acá para poder ser usadas.
	 * El segundo parámetro indica la cantidad de parámetros que recibe la tarea
	 * como un vector de enteros, o -1 para una cantidad de parámetros variable. */
	register_task(TaskCPU, 1);
	register_task(TaskIO, 2);
	register_task(TaskAlterno, -1);
    
    /* Tareas propias */
    register_task(TaskConsola, 3);
    register_task(TaskBatch, 2);
}
