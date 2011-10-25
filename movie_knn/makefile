#movie_knn/makefile
# simple makefile for the movie_knn library
# files: source files: movie_knn.cpp generate_movie_knn.cpp
#        header files: movie_knn.h
# executable file: generate_movie_knn
#

# first define target file : movie_knn
#              dependencies are the object files that build the program

generate_movie_knn: ../binary_files/binary_files.o movie_knn.o generate_movie_knn.o 
	g++ -o generate_movie_knn ../binary_files/binary_files.o movie_knn.o generate_movie_knn.o

movie_knn.o: movie_knn.cpp movie_knn.h ../binary_files/binary_files.h
	g++ -c movie_knn.cpp

generate_movie_knn.o: generate_movie_knn.cpp movie_knn.h
	g++ -c generate_movie_knn.cpp

clean:
	rm generate_movie_knn movie_knn.o generate_movie_knn.o

# that's all

