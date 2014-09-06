#include <vector>
#include <queue>
#include "sched_rr.h"
#include "basesched.h"
#include <iostream>

using namespace std;

SchedRR::SchedRR(vector<int> argn) {
    auto it = argn.begin();
    it++;
    while (it != argn.end()) {
        ticks.push_back(*it);
        quantums.push_back(*it);
        it++;
    }
}

SchedRR::~SchedRR() {

}


void SchedRR::load(int pid) {
   ready.push(pid);
}

void SchedRR::unblock(int pid) {
    ready.push(pid);
}

int SchedRR::tick(int cpu, const enum Motivo m) {
    if (m == EXIT || m == BLOCK) {
        ticks[cpu] = quantums[cpu];
        if (ready.empty()) {
            return IDLE_TASK;
        } else {
            int sig = ready.front();
            ready.pop();
            return sig;
        }
    } else if (current_pid(cpu) == IDLE_TASK && !ready.empty()) {
        int sig = ready.front();
        ready.pop();
        return sig;
    } else {
        ticks[cpu]--;
        if (ticks[cpu] == 0) {
            ready.push(current_pid(cpu));
            ticks[cpu] = quantums[cpu];
            int sig = ready.front();
            ready.pop();
            return sig; 
        }
    }

    return current_pid(cpu);
}
