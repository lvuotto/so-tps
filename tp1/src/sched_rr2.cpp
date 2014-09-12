#include <vector>
#include <queue>
#include "sched_rr2.h"
#include "basesched.h"
#include <algorithm>

using namespace std;

SchedRR2::SchedRR2(vector<int> argn) {
// Round robin recibe la cantidad de cores y sus cpu_quantum por parámetro
  for (int i = 0; i < argn[0]; i++){
    cores.push_back(core_info(argn[i + 1]));
  }
}

SchedRR2::~SchedRR2() {

}

int SchedRR2::min_core() {
  return min_element(cores.begin(), cores.end(), core_cmp()) - cores.begin();
}  

void SchedRR2::load(int pid) {
  pid_a_core.insert(pair<int,int>(pid, min_core()));
  cores[min_core()].ready.push(pid);
}

void SchedRR2::unblock(int pid) {
  cores[pid_a_core[pid]].ready.push(pid);
  cores[pid_a_core[pid]].bloqueados--;
}

int SchedRR2::tick(int cpu, const enum Motivo m) {
  if (cores[cpu].ready.empty() && !cores[cpu].corriendo){ 
    return IDLE_TASK;
  }
  
  if (m == EXIT) {
    pid_a_core.erase(current_pid(cpu));
    cores[cpu].ciclos = 0;
    cores[cpu].corriendo = false;
    if (cores[cpu].ready.empty()) {
      return IDLE_TASK;
    }
  } else if (m == BLOCK) {
    cores[cpu].bloqueados++;
    cores[cpu].ciclos = 0;
    cores[cpu].corriendo = false;
    if (cores[cpu].ready.empty()) {
      return IDLE_TASK;
    }
  } else {
    if (current_pid(cpu) == IDLE_TASK) {
       cores[cpu].ciclos = 0;
    } else {
      cores[cpu].ciclos++;
    }
    if (cores[cpu].ciclos == cores[cpu].quantum) {
      cores[cpu].ciclos = 0;
      cores[cpu].ready.push(current_pid(cpu));
    } else if (current_pid(cpu) != IDLE_TASK) {
      return current_pid(cpu);
    }
  }
  
  cores[cpu].corriendo = true;
  int n = cores[cpu].ready.front();
  cores[cpu].ready.pop();
  return n;
}
