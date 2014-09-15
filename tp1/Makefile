SRCDIR = src
BINDIR = bin
TESTDIR = test
BENCHDIR = benchmark
INFORMEDIR = informe
IMGDIR = $(INFORMEDIR)/imagenes


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
  $(BENCHDIR)/run.4.sh
  
informe:
	make -C $(INFORME)
	xdg-open $(BINDIR)/informe.pdf


all: tp test bechmark

clean:
	rm -f $(BINDIR)/*
