SRCDIR=src
BINDIR=bin
TESTDIR=test
BENCHDIR=benchmark
INFORMEDIR=informe
IMGDIR=$(INFORMEDIR)/imagenes
BENCHMARKS=run.4.sh


.PHONY: all clean test benchmark informe

tp:
	make -C $(SRCDIR)

test:
	make -C $(TESTDIR)

benchmark:
	make
	make test
	BINDIR=$(BINDIR) TESTDIR=$(TESTDIR)\
  BENCHDIR=$(BENCHDIR) IMGDIR=$(IMGDIR)\
  $(BENCHDIR)/$(BENCHMARKS)

plot:
	make benchmark
	BENCHDIR=$(BENCHDIR) IMGDIR=$(IMGDIR) $(BENCHDIR)/plotter.sh

informe:
	make -C $(INFORMEDIR)
	xdg-open $(BINDIR)/informe.pdf


all: tp test bechmark

clean:
	rm -f $(BINDIR)/*
