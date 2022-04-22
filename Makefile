build:
	mpicc HW2.c functions.c  -o HW2 -lm
clean:
	rm -rf *.o HW2
