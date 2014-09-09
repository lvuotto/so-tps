SRCDIR = src
BINDIR = bin
TESTDIR = test
BENCHDIR = benchmark
INFORMEDIR = informe


.PHONY: all clean test benchmark informe

tp:
	make -C $(SRCDIR)

test:
	make -C $(TESTDIR)

benchmark:
	make -C $(BENCHDIR)

informe:
	make -C $(INFORME)
	xdg-open $(BINDIR)/informe.pdf


all: tp test bechmark

clean:
	rm -f $(BINDIR)/*
