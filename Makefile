all: clean
	gcc -w -fno-asynchronous-unwind-tables -fno-exceptions -fno-rtti *.c -o gtcl -lm -O2
	strip ./gtcl
	chmod +x ./gtcl

.SILENT: run clean
run: clean all
	./gtcl -i

test: all
	./gtcl ./tests/count.tcl

clean:
	@rm -f gtcl
