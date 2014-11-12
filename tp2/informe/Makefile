
BINS = informe.pdf
BINDIR = ../bin


.PHONY: all clean


informe.pdf: informe.tex
	pdflatex $<
#	mv $@ $(BINDIR)

all: $(BINS)

clean:
	rm -f $(BINS)
