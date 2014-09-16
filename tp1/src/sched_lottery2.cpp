#include "sched_lottery2.h"

#include <list>
#include <map>
#include <utility>
#include <algorithm>
#include <cstdlib>

using namespace std;

SchedLottery2::SchedLottery2(vector<int> argn) : actual(procesos.begin()) {
  srand(argn[2]);
  quantum = argn[1];
  ciclos = 0;
}

SchedLottery2::~SchedLottery2() {
}

void SchedLottery2::load(int pid) { 
  procesos.push_back(pair<int, int>(pid, 1));
  tickets++;
}

void SchedLottery2::unblock(int pid) {
  pair<int, int> a_insertar;
  a_insertar.first = pid;
  a_insertar.second = bloqueados[pid];
  //lo inserto ordenado en la lista
  auto pista = lower_bound(procesos.begin(),
                           procesos.end(),
                           a_insertar,
                           procesos_cmp());
  procesos.insert(pista, a_insertar);
  bloqueados.erase(pid);
  tickets += a_insertar.second;
}

list<pair<int, int> >::iterator SchedLottery2::proximo_proceso() {
  auto it = procesos.begin();
  if (it == procesos.end())
    return it;

  int ganador = rand() % tickets;
  int suma_parcial = 0;

  while (suma_parcial + it->second <= ganador) {
    suma_parcial += it->second;
    it++;
  }
  for (auto i = procesos.begin(); i != procesos.end(); i++) {
    i->second = 1;
  }
  return it;
}

int SchedLottery2::tick(int cpu, const enum Motivo m) {
  if (m == EXIT) {
    
    ciclos = 0;
    tickets -= actual->second;
    procesos.erase(actual);
    actual = proximo_proceso();
    
  } else if (m == BLOCK) {

    bloqueados.insert(pair<int, int>(actual->first, actual->second));
    ciclos = 0;
    tickets -= actual->second;
    procesos.erase(actual);
    actual = proximo_proceso();
  
  } else {

    ciclos++;
    if (ciclos == quantum || current_pid(cpu) == IDLE_TASK) {
      actual = proximo_proceso();
      ciclos = 0;
    }
     
  }

  return procesos.empty() ? IDLE_TASK : actual->first;
}
