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
              'EXIT': other_event,
              'CONTEXT':context_switch_event,
              'WAITING':None,
              'NOT_LOAD': None,
              'CPU_BLOCK': None}
    
    @classmethod
    def get_event(cls, event_line):
        splited_event_line= event_line.split()
        if splited_event_line[0] in EventFactory.Events.keys():
            print splited_event_line
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
    print fin
    for line in fin:
        ln += 1
        vls = line.split()
        if line and line[0] == '#':
            if line.startswith('# SETTINGS '):
                settings = line[11:].strip()
                continue
            else:
                line= line[2:].strip() # Queda --> 'CONTEXT CPU[cpu] time

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
    core_resume= dict() # core: {processing_time: #, switching_time: #, idle_time: #}
    core_timeline= dict() # core: list(pids) NOTA: se supone que en cada tick hay un pid
    pid_resume= dict() # pid: {load_time: #, running_time: #, blocked: #, end_time: #}
    pids_timeline= dict() # pid: list((event_code, core))
    
    block_lapse= dict()

    for event in data:
        #core_time
        if event.core != -1:
             if event.core not in core_timeline: core_timeline[event.core]= []
             core_timeline[event.core].append(event.pid)
        
        #core_time
        if event.core != -1:
            if event.core not in core_resume: core_resume[event.core]= {'processing_time': 0, 'switching_time': 0, 'idle_time': 0}
            if event.event_type == 'CPU':
                if event.pid != -1: core_resume[event.core]['processing_time'] = core_resume[event.core]['processing_time'] + 1
                else: core_resume[event.core]['idle_time'] = core_resume[event.core]['idle_time'] + 1
            elif event.event_type == 'CONTEXT': core_resume[event.core]['switching_time'] = core_resume[event.core]['switching_time'] + 1
        
        if event.pid != -2 and event.pid != -1:
            #task_resume
            if event.pid not in pid_resume: pid_resume[event.pid]= {'running_time': 0, 'blocked': 0}
            if event.event_type == 'LOAD': pid_resume[event.pid]['load_time']= event.time
            if event.event_type == 'CPU': pid_resume[event.pid]['running_time']= pid_resume[event.pid]['running_time'] + 1
            if event.event_type == 'BLOCK': 
                if event.pid not in block_lapse: block_lapse[event.pid]= -1
                if block_lapse[event.pid] == -1:
                    block_lapse[event.pid]= event.time
            if event.event_type == 'UNBLOCK':
                pid_resume[event.pid]['blocked']= pid_resume[event.pid]['blocked'] + event.time - block_lapse[event.pid] + 1
                block_lapse[event.pid]= -1
            if event.event_type == 'EXIT': pid_resume[event.pid]['end_time']= event.time
        
            #task_timeline no hay interes en mostrar los context switchs
            
            if event.pid not in pids_timeline: pids_timeline[event.pid]= []
            if event.event_type == 'LOAD':
                #NOT LOADED
                for i in range(0, event.time):
                    pids_timeline[event.pid].append((EventFactory.Events.keys().index('NOT_LOAD'),-1))
            elif event.event_type == 'CPU':
                prev_event_code, prev_event_core= pids_timeline[event.pid][-1]
                #POSIBLE BLOCK ANTES QUE CPU
                if len(pids_timeline[event.pid]) == event.time:
                    prev_event_code, prev_event_core= pids_timeline[event.pid][-1]
                    if (EventFactory.Events.keys().index('BLOCK') == prev_event_code or
                        EventFactory.Events.keys().index('CPU_BLOCK') == prev_event_code):
                        pid_resume[event.pid]['running_time']= pid_resume[event.pid]['running_time'] - 1    
                        pids_timeline[event.pid][-1]= (EventFactory.Events.keys().index('CPU_BLOCK'),event.core)
                        continue
                #WAITING EVENT
                for i in range(len(pids_timeline[event.pid])+1, event.time):
                    pids_timeline[event.pid].append((EventFactory.Events.keys().index('LOAD'),-1))
            elif event.event_type == 'BLOCK':
                #ESTA BLOQUEADO Y EJECUTANDO
                if len(pids_timeline[event.pid]) == event.time:
                    prev_event_code, prev_event_core= pids_timeline[event.pid][-1]
                    if (EventFactory.Events.keys().index('CPU') == prev_event_code or
                        EventFactory.Events.keys().index('CPU_BLOCK') == prev_event_code):
                        pids_timeline[event.pid][-1]= (EventFactory.Events.keys().index('CPU_BLOCK'),prev_event_core)
                        continue
            elif event.event_type == 'UNBLOCK':
                #Agrego el gap que queda de todo el tiempo bloqueado
                block_code, block_dummy_core= pids_timeline[event.pid][-1]
                if block_code != EventFactory.Events.keys().index('CPU_BLOCK'): 
                    for i in range(len(pids_timeline[event.pid])+1, event.time+1):
                        pids_timeline[event.pid].append((block_code,block_dummy_core))

            if event.event_type != 'UNBLOCK' and event.event_type != 'EXIT':
                pids_timeline[event.pid].append((event.event_code,event.core))
    
    return core_resume, core_timeline, pid_resume, pids_timeline

def draw_cores_resume_bars(cores_resume, filename):
    ''' pre: cores_resume = { core: {processing_time: #, switching_time: #, idle_time: #}}'''
    ''' post: horizonal bar diagram in filenaname.png '''
    
    colors={'processing_time': 'green', 'switching_time': 'yellow', 'idle_time': 'red'}
    labels={'processing_time': 'Procesando', 'switching_time': 'Cambiando contexto', 'idle_time': 'Sin uso'}
    
    fig= figure(figsize=(11.8,8.3))
    
    
    bars_lenghts= []
    for core in cores_resume:
        bars_lenghts.append(sum(cores_resume[core].values()))
        values= sorted(zip(cores_resume[core].values(), cores_resume[core].keys()), reverse= True)
        base= 0
        for value in values:
            broken_barh([(base,value[0])], (core-0.4,0.8), color=colors[value[1]], edgecolor='black')
            base= base+value[0]
    
    title('Tiempo total por tipo de tarea por core')
    yticks(cores_resume.keys())
    xlabel('Tiempo total')
    ylabel('Core')
    xlim((0,max(bars_lenghts)+1))
    ylim((-1,len(cores_resume.keys())+1))
    tight_layout()
    legend()
    savefig(filename+'.png', dpi=300, format='png')
    return None

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
        last_pid= -1
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

def draw_tasks_resume_bars(pids_resume, filename):
    ''' pre: pid: {load_time: #, running_time: #, blocked: #, end_time: #}'''
    ''' post: horizonal bar diagram in filenaname.png '''
    
    colors={'running_time': '#c0ffc0', 'blocked': '#b7b7f7', 'waiting_time': '#d0d0d0'}
    labels={'running_time': 'En ejecucion', 'blocked': 'Bloqueado', 'waiting_time': 'Esperando'}
    
    fig= figure(figsize=(11.8,8.3))
    pids_resume.pop(-1,None)

    max_time= 0
    for pid in pids_resume:
        load_time= pids_resume[pid].pop('load_time',-1)
        end_time= pids_resume[pid].pop('end_time',-1)
        max_time= max([end_time-load_time,max_time])
        pids_resume[pid]['waiting_time']= end_time - load_time - pids_resume[pid]['running_time'] - pids_resume[pid]['blocked']
        values= sorted(zip(pids_resume[pid].values(), pids_resume[pid].keys()), reverse= True)
        base= 0
        for value in values:
            broken_barh([(base,value[0])], (pid-0.25, 0.5), facecolor=colors[value[1]])
            #if pid != 0:
            #    barh(pid, base + value[0], align='center', color=colors[value[1]], edgecolor='black')
            #else:
            #    barh(pid, base + value[0], align='center', color=colors[value[1]], edgecolor='black', label=labels[value[1]])
            base= base + value[0]

    running_dummy = Rectangle((0, 0), 1, 1, fc=colors['running_time'])
    blocked_dummy = Rectangle((0, 0), 1, 1, fc=colors['blocked'])
    waiting_dummy = Rectangle((0, 0), 1, 1, fc=colors['waiting_time'])
    legend([running_dummy, blocked_dummy, waiting_dummy],[labels['running_time'],labels['blocked'], labels['waiting_time']],loc='best', ncol=3)
    title('Tiempo total de la tarea divido en estados')
    yticks(pids_resume.keys())
    xlabel('Tiempo total')
    ylabel('Tarea')
    ylim((-1,len(pids_resume.keys())+1))
    xlim((0,max_time+2))
    tight_layout()
    #show()
    savefig(filename+'.png', dpi=300, format='png')
    return None

def draw_pids_timeline_gannt(pids_timeline, filename):
    ''' pre: pids_timeline = { pid: list((event_code, core))}'''
    ''' post: horizonal bar diagram in filenaname.png '''
    
    # Necesitaria tener la info algo asi como
    # core: {pid: ini_1,fin_1,ini_2,fin_2} (es decir por intervalos)
    # broken_barh necesita (inicio, longitud)
    
    colors={'CPU':'#f7b7b7','BLOCK':'#b7b7f7', 'WAITING':'#e7ffe7', 'LOAD':'#e7ffe7', 'NOT_LOAD':'#d0d0d0', 'CPU_BLOCK':'#b7b7f7'}

    fig= figure(figsize=(11.8,8.3))
    ax = fig.add_subplot(111) 
    title('Estado de las tareas por tiempo')
    yticks(pids_timeline.keys(), sorted(pids_timeline.keys(),reverse=True))
#    ax.xaxis.set_major_locator( 
    ax.xaxis.set_major_locator( IndexLocator(5,1) )
    #xticks(range(len(cores_timeline[0])),range(0,len(cores_timeline[0]),5))
    xlabel('Tiempo')
    ylabel('Tarea')
    ax.grid(True)            

    for pid in pids_timeline:
        pid_state_by_time= pids_timeline[pid]
        intervals= dict()
        last_state= (None,None)
        for time in range(len(pid_state_by_time)):
            if last_state != pid_state_by_time[time]:
                last_state = pid_state_by_time[time]
                if last_state not in intervals: intervals[last_state]= []
                intervals[last_state].append((time, 1))
                #intervals.push((last_pid,time,1))
            else:
                #pid, time, interval_size= intervals.pop()
                #intervals.push((pid, time, interval_size+1))
                time, interval_size= intervals[last_state].pop()
                intervals[last_state].append((time, interval_size+1))

        for event_code, core in intervals.keys():
            event_name= EventFactory.Events.keys()[event_code]
            rect= ax.broken_barh(intervals[(event_code, core)], ((len(pids_timeline)-pid-1)-0.25, 0.5), facecolor=colors[event_name])
            if core >= 0:
                for init, size in intervals[(event_code, core)]:
                    ax.text(init+(size/2.0),(len(pids_timeline)-pid-1), str(core), ha="center", va="center", size=9, weight='bold')
            #else:
                #if event_code == -1:
                #elif event_code == -2:
                    #rect= ax.broken_barh(intervals[(event_code, core)], (pid-0.25, 0.5), facecolor=colors['not_loaded'])
                #elif event_name == 'LOAD':
                    #rect= ax.broken_barh(intervals[(event_code, core)], (pid-0.25, 0.5), facecolor=colors['load'])
                #elif event_name == 'BLOCK':
                    #rect= ax.broken_barh(intervals[(event_code, core)], (pid-0.25, 0.5), facecolor=colors['blocked'])
                #else:
                #    print 'ERROR!', event_code, core
    
    running_dummy = Rectangle((0, 0), 1, 1, fc=colors['CPU'])
    waiting_dummy = Rectangle((0, 0), 1, 1, fc=colors['WAITING'])
    not_loaded_dummy = Rectangle((0, 0), 1, 1, fc=colors['NOT_LOAD'])
    #load_dummy = Rectangle((0, 0), 1, 1, fc=colors['LOAD'])
    blocked_dummy = Rectangle((0, 0), 1, 1, fc=colors['BLOCK'])
    legend([not_loaded_dummy,running_dummy, waiting_dummy, blocked_dummy], ['No cargada','En ejecucion','Lista','Bloqueada'],loc='best', ncol=4)
    ylim((-1,len(pids_timeline.keys())+1))
    xlim((0,max([len(x) for x in pids_timeline.values()])+1))
    tight_layout()
    fig.autofmt_xdate()
    ax.legend()
    #show()
    savefig(filename+'.png', dpi=300, format='png')
    return None


def main(argv):
    if '-c' in argv or '--caption' in argv:
        hit = '-c' if '-c' in argv else '--caption'
        pos = argv.index(hit)
        argv.pop(pos)
        caption = argv.pop(pos)
    else:
        caption = None

    if len(argv) <= 1:
        fin = sys.stdin
        fout_cores_resume= 'out_cores_resume'
        fout_cores_timeline= 'out_cores_timeline'
        fout_pids_resume= 'out_pids_resume'
        fout_pids_timeline= 'out_pids_timeline'
    else:
        fin = open(argv[1], 'r')
        preffix= argv[1]
        fout_cores_resume= preffix + '_cores_resume' 
        fout_cores_timeline= preffix + '_cores_timeline' 
        fout_pids_resume= preffix + '_pids_resume' 
        fout_pids_timeline= preffix + '_pids_timeline' 
    
    print 'parsing input'
    settings, data, cores, pids = parseInput(fin)
    
    print 'data gathering'
    cores_resume, cores_timeline, pids_resume, pids_timeline= dataGathering(data, cores, pids)
    #todo dump de los datos
    print 'drawing cores resumen'
    draw_cores_resume_bars(cores_resume, fout_cores_resume)
    print 'drawing cores timeline'
    draw_cores_timeline_gannt(cores_timeline, fout_cores_timeline)
    print 'drawing cores timeline'
    draw_tasks_resume_bars(pids_resume, fout_pids_resume)
    print 'drawing pids timeline'
    draw_pids_timeline_gannt(pids_timeline, fout_pids_timeline)

if __name__ == "__main__":
    main(sys.argv)
