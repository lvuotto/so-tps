TESTS=ej2.tsk \
      ej4-1.tsk ej4-2.tsk

all: $(TESTS)

%.tsk: %.rb
	./$< > lote.$@

clean:
	rm -f lote*.tsk
