#ifndef __SCHED_RR2__
#define __SCHED_RR2__

#include <vector>
#include <queue>
#include <map>
#include "basesched.h"

using namespace std;

class SchedRR2 : public SchedBase {
public:
  SchedRR2(std::vector<int> argn);
  ~SchedRR2();
  virtual void load(int pid);
  virtual void unblock(int pid);
  virtual int tick(int cpu, const enum Motivo m);

private:
  struct core_info {
    int quantum;
    int bloqueados;
    queue<int> ready;
    int ciclos;
    bool corriendo;
    core_info(int q) : quantum(q), bloqueados(0), ciclos(0),
                       corriendo(false) {}
  };

  struct core_cmp {
    bool operator() (const core_info& c1, const core_info& c2) {
      return (c1.bloqueados + c1.ready.size() + (c1.corriendo ? 1 : 0)) <
             (c2.bloqueados + c2.ready.size() + (c2.corriendo ? 1 : 0));
    }
  };
  map<int, int> pid_a_core;
  vector<core_info> cores;
  int min_core();
};

#endif
