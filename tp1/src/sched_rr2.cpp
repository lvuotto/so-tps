#include <vector>
#include <queue>
#include "sched_rr2.h"
#include "basesched.h"
#include <iostream>

using namespace std;

SchedRR2::SchedRR2(vector<int> argn) {
	// Round robin recibe la cantidad de cores y sus cpu_quantum por parámetro
}

SchedRR2::~SchedRR2() {

}


void SchedRR2::load(int pid) {
}

void SchedRR2::unblock(int pid) {
}

int SchedRR2::tick(int cpu, const enum Motivo m) {
  return IDLE_TASK;
}

int SchedRR2::next(int cpu) {
	return 0;
}
