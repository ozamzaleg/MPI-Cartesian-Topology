build:
	mpicc HW2.c functions.c  -o HW2 -lm
clean:
	rm -rf *.o HW2
run:
	mpiexec -np 25 ./HW2 <data.txt
