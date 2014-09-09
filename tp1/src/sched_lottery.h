#ifndef __SCHED_LOT__
#define __SCHED_LOT__
#include "basesched.h"

using namespace std;


class SchedLottery : public SchedBase {
public:
  SchedLottery(vector<int> argn);
  ~SchedLottery();
  virtual void load(int pid);
  virtual void load(int pid, int deadline);
  virtual void unblock(int pid);
  virtual int tick(int cpu, const enum Motivo m);
 
private:
  struct proceso{
    double compensacion;
    int pid;
    int tickets;
  };

  list<pair<int, int> > procesos;
  map<int, int> bloqueados;
  int ciclos;
  int quantum;
  int semilla;
  /*list<pair<int, int> >::iterator actual;*/
  auto actual;
  int tickets;
};

#endif
