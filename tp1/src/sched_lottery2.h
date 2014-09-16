#ifndef __SCHED_LOT__
#define __SCHED_LOT__
#include "basesched.h"

#include <list>
#include <map>
#include <utility>

using namespace std;


class SchedLottery2 : public SchedBase {
public:
  SchedLottery2(vector<int> argn);
  ~SchedLottery2();
  virtual void load(int pid);
  virtual void unblock(int pid);
  virtual int tick(int cpu, const enum Motivo m);
 
private:
  struct procesos_cmp {
    bool operator() (const pair<int, int>& a,
                     const pair<int, int>& b) const
    {
      return a.second < b.second ||
             (a.second == b.second && a.first < b.first);
    }
  };
    
  list<pair<int, int> >::iterator proximo_proceso();
  //El primer elemento es el pid, y el segundo la cantidad de tickets
  list<pair<int, int> > procesos; 
  map<int, int> bloqueados;
  int ciclos;
  int quantum;
  list<pair<int, int> >::iterator actual;
  int tickets;
};

#endif
