BINS=src/ej1.js src/ej2.js src/ej3.js src/ej4.js src/ej5.js
MONGO=mongo --quiet

all: db run

db:
	bin/csv_loader.py

run: $(BINS)
	$(MONGO) src/ej1.js
	$(MONGO) src/ej2.js
	$(MONGO) src/ej3.js
	$(MONGO) src/ej4.js
	$(MONGO) src/ej5.js

run-ej1:
	$(MONGO) src/ej1.js

run-ej2:
	$(MONGO) src/ej2.js

run-ej3:
	$(MONGO) src/ej3.js

run-ej4:
	$(MONGO) src/ej4.js

run-ej5:
	$(MONGO) src/ej5.js

informe:
	make -C informe
	xdg-open bin/informe.pdf

.PHONY: all db run run-ej1 run-ej2 run-ej3 run-ej4 run-ej5 informe
