TPS = tp1 tp2 tp3

.PHONY: all clean

all: $(TPS)

tp1:
	make -C tp1

tp1:
	make -C tp2

tp1:
	make -C tp3

clean:
	make -C tp1 clean
	make -C tp2 clean
	make -C tp3 clean
