export LC_ALL=C
CXXFLAGS= -Wall -Wextra -pedantic -g -ggdb -std=c++0x
LDFLAGS= -lpthread
BINDIR = ../bin

OBJS=main.o simu.o basesched.o basetask.o tasks.o sched_rr.o sched_fcfs.o sched_rr2.o sched_lottery.o sched_lottery2.o
MAIN=simusched

.PHONY: all
all: $(MAIN)

$(MAIN): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)
	mv $@ $(BINDIR)

basesched.o: basesched.cpp basesched.h
basetask.o: basetask.cpp basetask.h
main.o: main.cpp simu.h basetask.h basesched.h tasks.h sched_fcfs.h \
 sched_rr.h sched_rr2.h
sched_fcfs.o: sched_fcfs.cpp sched_fcfs.h basesched.h
sched_rr.o: sched_rr.cpp sched_rr.h basesched.h
sched_rr2.o: sched_rr2.cpp sched_rr2.h basesched.h
sched_lottery.o: sched_lottery.cpp sched_lottery.h basesched.h
sched_lottery2.o: sched_lottery2.cpp sched_lottery2.h basesched.h
simu.o: simu.cpp simu.h basetask.h basesched.h
simusched.o: simusched.cpp basetask.h basesched.h tasks.h
tasks.o: tasks.cpp tasks.h basetask.h

.PHONY: clean
clean:
	rm -f $(OBJS) $(BINDIR)/$(MAIN)

.PHONY: new
new: clean all
