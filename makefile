all:
	gcc project2.c -o project2
clean:
	find . ! -name "project2.c" -name "makefile" -name "README.txt" -delete
