#ifndef __SCHED_LOT__
#define __SCHED_LOT__

#include "basesched.h"


class SchedLottery : public SchedBase {
  public:
    SchedLottery(std::vector<int> argn);
        ~SchedLottery();
    virtual void load(int pid);
    virtual void load(int pid, int deadline);
    virtual void unblock(int pid);
    virtual int tick(int cpu, const enum Motivo m);

};

#endif
