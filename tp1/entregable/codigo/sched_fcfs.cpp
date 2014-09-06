#include "sched_fcfs.h"

using namespace std;

SchedFCFS::SchedFCFS(vector<int> argn) {
	// FCFS recibe la cantidad de cores.
}

SchedFCFS::~SchedFCFS() {
}

void SchedFCFS::load(int pid) {
	q.push(pid); // llegó una tarea nueva
}

void SchedFCFS::unblock(int pid) {
	// Uy! unblock!... bueno, ya seguir'a en el próximo tick
}

int SchedFCFS::tick(int cpu, const enum Motivo m) {
	if (m == EXIT) {
		// Si el pid actual terminó, sigue el próximo.
		if (q.empty()) return IDLE_TASK;
		else {
			int sig = q.front(); q.pop();
			return sig;
		}
	} else {
		// Siempre sigue el pid actual mientras no termine.
		if (current_pid(cpu) == IDLE_TASK && !q.empty()) {
			int sig = q.front(); q.pop();
			return sig;
		} else {
			return current_pid(cpu);
		}
	}
}
