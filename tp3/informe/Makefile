BINS=informe.pdf
BINDIR=../bin


informe.pdf: informe.tex
	pdflatex $<
	mv $@ $(BINDIR)

all: $(BINS)

clean:
	rm -f $(BINDIR)/$(BINS)

.PHONY: all clean
