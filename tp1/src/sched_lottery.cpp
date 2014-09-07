#include "sched_lottery.h"

using namespace std;

SchedLottery::SchedLottery(vector<int> argn) {
  // FCFS recibe la cantidad de cores.
}

SchedLottery::~SchedLottery() {
}

void SchedLottery::load(int pid) {
  load(pid,0);
}

void SchedLottery::load(int pid,int deadline) {
}

void SchedLottery::unblock(int pid) {
}

int SchedLottery::tick(int cpu, const enum Motivo m) {
  return 0;
}
