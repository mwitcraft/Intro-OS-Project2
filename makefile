all:
	gcc project2.c -o project2
clean:
	mv project2.c makefile README.txt ../
	rm -r *
	mv ../project2.c ../makefile ../README.txt ./
