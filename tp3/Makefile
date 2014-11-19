BINS=bin/ej1.js bin/ej2.js bin/ej3.js bin/ej4.js bin/ej5.js

all: db run

db:
	bin/csv_loader.py

run: $(BINS)

run-ej1:
	mongo bin/ej1.js

run-ej2:
	mongo bin/ej2.js

run-ej3:
	mongo bin/ej3.js

run-ej4:
	mongo bin/ej4.js

run-ej5:
	mongo bin/ej5.js

.PHONY: all db run run-ej1 run-ej2 run-ej3 run-ej4 run-ej5
