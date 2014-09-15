TESTS=ej2.tsk

all: $(TESTS)

%.tsk: %.rb
	./$< > lote.$@

clean:
	rm -f lote*.tsk
