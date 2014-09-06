#!/usr/bin/env python
# coding: utf-8

import sys, os
import matplotlib
matplotlib.use('Agg')
from pylab import *
from matplotlib.transforms import TransformedBbox

class EventFactory(object):
    #CPU time pid cpu (si pid == -1 -> idle)
    cpu_event= lambda x: Event(x[0],EventFactory.Events.keys().index(x[0]),int(x[1]),int(x[2]),int(x[3]))
    #EVENT time pid
    other_event= lambda x: Event(x[0],EventFactory.Events.keys().index(x[0]),int(x[1]),int(x[2]),-1)
    #CONTEXT CPU cpu time (se pone pid == -2)
    context_switch_event= lambda x: Event(x[0] ,EventFactory.Events.keys().index(x[0]),int(x[3]),-2,int(x[2]))
    Events = {'LOAD': other_event,
              'CPU':cpu_event,
              'BLOCK': other_event,
              'UNBLOCK': other_event,
              'DEADLINE': other_event,
              'EXIT': other_event,
              'CONTEXT':context_switch_event,
              'WAITING':None,
              'NOT_LOAD': None,
              'CPU_BLOCK': None}

    @classmethod
    def get_event(cls, event_line):
        splited_event_line= event_line.split()
        if splited_event_line[0] in EventFactory.Events.keys():
            return EventFactory.Events[splited_event_line[0]](splited_event_line)
        else:
            return None


class Event(object):
    def __init__(self, event_type, event_code, time, pid, core):
        self.event_type= event_type
        self.event_code= event_code
        self.time= time
        self.pid= pid
        self.core= core
    def __str__(self):
        return 'Type: ' + self.event_type + ', Code: ' + str(self.event_code) + ', Time: ' + str(self.time) + ', Pid: ' + str(self.pid) + ', Core: ' + str(self.core)

def parseInput(fin):
    ln = 0
    result = []
    cores = 0
    pids= 0
    settings = None
    cpus_timeline= dict()
    for line in fin:
        ln += 1
        vls = line.split()
        if line and line[0] == '#':
            if line.startswith('# SETTINGS '):
                settings = line[11:].strip()
                continue
            else:
                line= line[2:].strip() # Queda --> 'CONTEXT CPU cpu  time

        event= EventFactory.get_event(line)
        result.append(event)

        if event.event_type == 'CPU':
            if (cores <=event.core):
                cores = event.core+1

        if event.event_type == 'LOAD':
            if(pids <= event.pid):
                pids = event.pid +1

    return settings, result, cores, pids

def dataGathering(data, cores, pids):
    core_timeline= dict() # core: list(pids) NOTA: se supone que en cada tick hay un pid

    block_lapse= dict()

    for event in data:
        #core_time
        if event.core != -1:
             if event.core not in core_timeline: core_timeline[event.core]= []
             core_timeline[event.core].append(event.pid)

    return core_timeline

def draw_cores_timeline_gannt(cores_timeline, filename):
    ''' pre: cores_timeline = {core: list(pid) } '''

    # Necesitaria tener la info algo asi como
    # core: {pid: ini_1,fin_1,ini_2,fin_2} (es decir por intervalos)
    # broken_barh necesita (inicio, longitud)

    colors={'pid':'#c0ffc0','switch':'#b7b7f7', 'idle':'#d0d0d0'}

    fig= figure(figsize=(11.8,8.3))
    ax = fig.add_subplot(111)
    title('Tareas en Core por tiempo')
    yticks(cores_timeline.keys())
#    ax.xaxis.set_major_locator(
    ax.xaxis.set_major_locator( IndexLocator(2,1) )
    #xticks(range(len(cores_timeline[0])),range(0,len(cores_timeline[0]),5))
    xlabel('Tiempo')
    ylabel('Core')
    ylim((-1,len(cores_timeline.keys())))
    ax.grid(True)

    for core in cores_timeline:
        pids_by_time= cores_timeline[core]
        intervals= dict()
        last_pid= None
        for time in range(len(pids_by_time)):
            if last_pid != pids_by_time[time]:
                last_pid = pids_by_time[time]
                if last_pid not in intervals: intervals[last_pid]= []
                intervals[last_pid].append((time, 1))
                #intervals.push((last_pid,time,1))
            else:
                #pid, time, interval_size= intervals.pop()
                #intervals.push((pid, time, interval_size+1))
                time, interval_size= intervals[last_pid].pop()
                intervals[last_pid].append((time, interval_size+1))

        for pid in intervals:
            if pid >= 0:
                rect= ax.broken_barh(intervals[pid], (core-0.25, 0.5), facecolor=colors['pid'])
                for init, size in intervals[pid]:
                    ax.text(init+(size/2.0),core, str(pid), ha="center", va="center", size=9, weight='bold')
            elif pid == -1:
                rect= ax.broken_barh(intervals[pid], (core-0.25, 0.5), facecolor=colors['idle'])
            elif pid == -2:
                rect= ax.broken_barh(intervals[pid], (core-0.25, 0.5), facecolor=colors['switch'])
            else:
                print 'ERROR!'

    tarea_dummy = Rectangle((0, 0), 1, 1, fc=colors['pid'])
    switch_dummy = Rectangle((0, 0), 1, 1, fc=colors['switch'])
    idle_dummy = Rectangle((0, 0), 1, 1, fc=colors['idle'])
    legend([tarea_dummy, switch_dummy, idle_dummy], ['Tarea','Cambio de contexto','Inactivo'])
    tight_layout()
    fig.autofmt_xdate()
    ax.legend()
    savefig(filename+'.png', dpi=300, format='png')

def main(argv):
    if len(argv) <= 1:
        fin = sys.stdin
        fout_cores_timeline= 'out_cores_timeline'
    else:
        fin = open(argv[1], 'r')
        preffix= argv[1]
        fout_cores_timeline= preffix + '_cores_timeline'

    print 'parsing input'
    settings, data, cores, pids = parseInput(fin)

    print 'data gathering'
    cores_timeline= dataGathering(data, cores, pids)
    print cores_timeline
    #todo dump de los datos
    print 'drawing cores timeline'
    draw_cores_timeline_gannt(cores_timeline, fout_cores_timeline)

if __name__ == "__main__":
    main(sys.argv)
