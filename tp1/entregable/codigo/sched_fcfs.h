#ifndef __SCHED_FCFS__
#define __SCHED_FCFS__

#include <vector>
#include <queue>
#include "basesched.h"

class SchedFCFS : public SchedBase {
	public:
		SchedFCFS(std::vector<int> argn);
        ~SchedFCFS();
		virtual void load(int pid);
		virtual void unblock(int pid);
		virtual int tick(int cpu, const enum Motivo m);

	private:
		std::queue<int> q;
};

#endif
