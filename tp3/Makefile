BINS=src/ej1.js src/ej2.js src/ej3.js src/ej4.js src/ej5.js

all: db run

db:
	bin/csv_loader.py

run: $(BINS)

run-ej1:
	mongo src/ej1.js

run-ej2:
	mongo src/ej2.js

run-ej3:
	mongo src/ej3.js

run-ej4:
	mongo src/ej4.js

run-ej5:
	mongo src/ej5.js

informe:
	make -C informe
	xdg-open bin/informe.pdf

.PHONY: all db run run-ej1 run-ej2 run-ej3 run-ej4 run-ej5 informe
