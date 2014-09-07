#include <vector>
#include <pthread.h>
#include <errno.h>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <queue>
#include "simu.h"

#include "basetask.h"
#include "basesched.h"

using namespace std;

//#define DBG_THREAD

#ifdef DBG_THREAD
#define _D(X) X
#else
#define _D(X)
#endif

#define forn(i,n) for(int i = 0; i < (int)(n); ++i)
#define DBG(X) _D(cerr << #X << " = " << X << endl;)

typedef struct cpu_ctx {
    int pid;
    int remaining;
} cpu_ctx_t;

/* "Globales" */
// static int cur_pid;
//Habria que modificar el nombre de cur_pid
//lo mantuve para que sea mas facil modificar
//codigo y mirar que hacia antes
static vector<cpu_ctx_t> contexts;
static unsigned int cur_time;
static pthread_mutex_t m_sched;

enum status_t {ST_EXIT, ST_IO, ST_CPU};

struct task_data {
	pthread_t tid;
	int pid, running;
	status_t blk;
	int blkms;
	TaskBase* tsk;
	vector<int>* prms;
	pthread_mutex_t mutex;
    int lastcpu;
};
static task_data* tsks;

void* task_thread(task_data* tsk) {
	pthread_mutex_lock(&tsk->mutex);
	//fprintf(stderr, "Thread %d\n", tsk->pid);
	_D(cerr << "tsk->tsk( " << tsk->pid << ", *tsk->prms)" << endl;)
	tsk->tsk(tsk->pid, *tsk->prms); // Run!
	//
	tsk->blk = ST_EXIT;
	_D(cerr << "TSK unlock(sched) exit" << endl;) pthread_mutex_unlock(&m_sched);
	return NULL;
}


/* Funciones llamadas por el scheduler */
int current_pid(int cpu) { return contexts[cpu].pid; }
int current_remaining(int cpu) { return contexts[cpu].remaining; }
unsigned int current_time(void) { return cur_time; }

/* Funciones llamadas por las tareas */
static void uso_X(task_data* tsk, enum status_t tp, unsigned int ms) {
	if (ms == 0) return;
	tsk->blk = tp;
	tsk->blkms = ms;
	_D(cerr << "TSK unlock(sched)" << endl;) pthread_mutex_unlock(&m_sched);
	pthread_mutex_lock(&tsk->mutex); _D(cerr << "TSK lock(mutex["<<tsk-tsks<<"])" << endl;)
}

//Se agrega pid para poder ver obtener
//los datos en tsks
void uso_CPU(int pid, unsigned int ms) {
	uso_X(&(tsks[pid]), ST_CPU, ms);
}

void uso_IO(int pid, unsigned int ms) {
	uso_X(&(tsks[pid]), ST_IO, ms);
}


void simulate(SchedBase& sch, std::vector<ptsk>& lote, const Settings& settings) {
	int n = lote.size();

	cout.flush();
	if (settings.output_log != "-") {
		/* Oh yeah! */
		freopen(settings.output_log.c_str(), "wt", stdout);
	}

	tsks = (task_data*)malloc(sizeof(task_data)*n);

	if (!tsks) { perror("malloc(task_data)"); return; }
	forn(i, n) {
		tsks[i].pid = i;
		pthread_mutex_init(&(tsks[i].mutex), NULL);
		pthread_mutex_lock(&(tsks[i].mutex));
		tsks[i].tsk = lote[i].tsk;
		tsks[i].prms = &(lote[i].prms);
		tsks[i].running = 0;
		tsks[i].blk = ST_CPU;
		tsks[i].blkms = 0;
        tsks[i].lastcpu = -1;
		if (pthread_create(&(tsks[i].tid), NULL, (void*(*)(void*))task_thread, (void*)(&(tsks[i]))) < 0) {
			perror("Lanzando la tarea (no use muchas tareas, < 500)"); return;
		}
	}
	pthread_mutex_init(&m_sched, NULL);
	pthread_mutex_lock(&m_sched);
	int finished = 0;

	//Inicializa los cpus
    contexts = vector<cpu_ctx_t>(settings.num_cores);
    for (int i = 0; i <settings.num_cores; i++)	{
        contexts[i].pid = IDLE_TASK;
        contexts[i].remaining = 0;
    }
	cur_time = 0;

	priority_queue<pair<int, int> > load;
	forn(i, n) load.push(make_pair(-lote[i].start, -i));

  vector<pair<unsigned int, int> > dlote(0);
  forn(i, n){
    if(lote[i].end > 0)
      dlote.push_back(make_pair(lote[i].end, i));
  }

	priority_queue<pair<int, int> > unblock;

	int context_remain = 0; /* Remaining context_switch ticks */


	while (finished < n || context_remain) {
		if (settings.verbose) {
			//cerr << "--- sched, tm=" << cur_time << " pid=" << cur_pid;
			cerr << "--- sched, tm=" << cur_time << endl;
			for(int i = 0; i < settings.num_cores; i++) {
				int pid=contexts[i].pid;
				cerr << "cpu " << i << " pid = " << pid << " rem " << contexts[i].remaining;
				if (pid != IDLE_TASK) { cerr << " [" << pid << " ST:"<< tsks[pid].blk << " ms:" << tsks[pid].blkms << "]"; }
				cerr << endl;
			}
            cerr << "--------------" << cur_time << endl;
		}

		// Load de las tareas
		while (!load.empty() && load.top().first >= -(int)cur_time) {
			int pid = -load.top().second;
      int deadline = lote[-load.top().second].end;
      load.pop();
			tsks[pid].running = 1;
			sch.load(pid,deadline);
			cout << "LOAD " << cur_time << " " << pid << endl;
		}

		vector<int> to_unblock;
		while (!unblock.empty() && unblock.top().first >= -(int)cur_time) {
			int pid = -unblock.top().second; unblock.pop();
			_D(cerr << "SCH unblock(" << pid << ")" << endl;)
			sch.unblock(pid); // pid
			to_unblock.push_back(pid);
            int unblocked = 0;
			for(int i = 0; i < settings.num_cores && !unblocked; i++) {
                int it = contexts[i].pid;
				if (it == pid) {
					tsks[pid].blkms = -2;
                    unblocked = true;
				}
            }
            if (!unblocked) {
				tsks[pid].blk = ST_CPU;
				tsks[pid].blkms = 0;
			}
		}
		//Itera por cada cpu
		for(int cpu= 0; cpu < settings.num_cores; cpu++){
			int cpu_pid= contexts[cpu].pid;
			int cpu_context_remain = contexts[cpu].remaining;
			if (!cpu_context_remain) {
				int npid;
				if (cpu_pid == IDLE_TASK) {
					npid = sch.tick(cpu, TICK);
					_D(cerr << "SCH tick( " << cpu << " ,TICK) -> " << npid << endl;)
				} else {
                    if (tsks[cpu_pid].blk == ST_CPU && !tsks[cpu_pid].blkms){
                        _D(cerr << "SCH unlock(" << cpu_pid << ") tick" << endl;) pthread_mutex_unlock(&(tsks[cpu_pid].mutex));
                        pthread_mutex_lock(&m_sched); _D(cerr << "SCH lock(sched)" << endl;)
                    }
                    switch (tsks[cpu_pid].blk) {
                        case ST_EXIT:
                            finished++;
                            npid = sch.tick(cpu, EXIT);
                            cout << "EXIT " << cur_time << " " << cpu_pid << " " << cpu << endl;
                            _D(cerr << "SCH tick( " << cpu << " ,EXIT) -> " << npid << endl;)
                            tsks[cpu_pid].running = 0;
                            break;
                        case ST_IO:
                            if (tsks[cpu_pid].blkms >= 0) {
                                unblock.push(make_pair(-(cur_time+tsks[cpu_pid].blkms), -cpu_pid));
                                tsks[cpu_pid].blkms = -1;
                            }
                            if (tsks[cpu_pid].blkms == -2) {
                                tsks[cpu_pid].blk = ST_CPU;
                                tsks[cpu_pid].blkms = 0;
                            }
                            cout << "BLOCK " << cur_time << " " << cpu_pid << endl;
                            npid = sch.tick(cpu, BLOCK);
                            _D(cerr << "SCH tick( " << cpu << " ,BLOCK) -> " << npid << endl;)
                            break;
                        case ST_CPU:
                            if (tsks[cpu_pid].blkms) {
                                tsks[cpu_pid].blkms--;
                            } else {
                                cerr << "FATAL ERROR, this should not happend" << endl;
                            }
                            npid = sch.tick(cpu, TICK);
                            _D(cerr << "SCH tick( " << cpu << " ,TICK) -> " << npid << endl;)
                            break;
                    }
				}
				if (npid == IDLE_TASK) {
					// cerr << "SCH unlock(sched)" << endl;pthread_mutex_unlock(&m_sched);
				} else {
					if (npid < 0 || npid >= n) { cerr << "Error!, scheduler sent an invalid pid="<<npid<< endl; return; }
					if (!tsks[npid].running) { cerr << "Error!, scheduler sent pid="<<npid << " but that process has exited." << endl; return; }
					if (!tsks[npid].blk == ST_IO) { cerr << "Error!, scheduler sent pid="<<npid << " but that process is still blocked." << endl; return; }
				}
                if (cpu_pid != npid){
                    if (npid != IDLE_TASK) {
                        if (settings.switch_cost > 0) {
                            contexts[cpu].remaining += settings.switch_cost;
                        }
                        if (cpu != tsks[npid].lastcpu && tsks[npid].lastcpu != -1) {
                            contexts[cpu].remaining += settings.migrate_cost;
                        }
                        tsks[npid].lastcpu = cpu;
                    }
                }
                cpu_pid = npid;

			} else {
                contexts[cpu].remaining--;
			}

            contexts[cpu].pid = cpu_pid;
		}
		//Hasta aca el codigo para cada CPU


		/* Unblock tasks at the end of the tick */
		for(int j=0; j<(int)to_unblock.size(); j++) cout << "UNBLOCK " << cur_time << " " << to_unblock[j] << endl;
		context_remain= 0;

		//Muestra que esta realizando cada cpu
		//y calcula si hay contexto total restante (ver while)
		for(int i= 0; i < contexts.size(); i++){
			context_remain += contexts[i].remaining;
			if (contexts[i].remaining /*context_remain*/) {
				cout << "# CONTEXT CPU " << i << " " << cur_time << endl;
			} else{
				cout << "CPU "<< cur_time << " " << contexts[i].pid << " " << i <<endl;
			}
		}

    //Muestra si se cumple el deadline de alguna tarea en este tick
    forn(i, dlote.size()){
      if(dlote[i].first == cur_time)
        cout << "DEADLINE "<< cur_time << " " << dlote[i].second << endl;
    }


		cur_time++;
	}
	forn(i, n) {
		pthread_join( tsks[i].tid, NULL);
	}
	free(tsks);
}
