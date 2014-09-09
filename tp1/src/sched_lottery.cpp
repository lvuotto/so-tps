#include "sched_lottery.h"

using namespace std;

SchedLottery::SchedLottery(vector<int> argn) : actual(procesos.begin()) {
  quantum = argn[0];
  semilla = argn[1];
  ciclos = 0;
}

SchedLottery::~SchedLottery() {
}

void SchedLottery::load(int pid) { 
  procesos.push_back(pair<int, int>(1, pid));
  tickets++;
}
void SchedLottery::load(int pid,int deadline) {
}

void SchedLottery::unblock(int pid) {
}

int SchedLottery::tick(int cpu, const enum Motivo m) {
  if (m == EXIT) {
    procesos.erase(actual);
    ciclos = 0;
    tickets -= actual->first;
    actual = proximo_proceso();
    return actual->second;
  } else if (m == BLOCK) {
     procesos.erase(actual);
     tickets -= actual->first;
     double f = ciclos / (double) quantum;
    
     ciclos = 0;
     actual = proximo_proceso();

      
    

  }
      
  }
  return 0;
}
