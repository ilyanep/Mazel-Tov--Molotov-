#movie_knn/makefile
# simple makefile for the movie_knn library
# files: source files: movie_knn.cpp movie_knn_pearson.cpp generate_movie_knn.cpp
#        header files: movie_knn.h movie_knn_pearson.h
# executable file: generate_movie_knn

generate_movie_knn: ../binary_files/binary_files.o movie_knn.o movie_knn_pearson.o generate_movie_knn.o 
	g++ -o generate_movie_knn ../binary_files/binary_files.o movie_knn.o movie_knn_pearson.o generate_movie_knn.o

movie_knn.o: movie_knn.cpp movie_knn.h ../binary_files/binary_files.h
	g++ -c movie_knn.cpp

movie_knn_pearson.o: movie_knn_pearson.cpp movie_knn_pearson.h movie_knn.h ../binary_files/binary_files.h
	g++ -c movie_knn_pearson.cpp

generate_movie_knn.o: generate_movie_knn.cpp movie_knn.h movie_knn_pearson.h ../binary_files/binary_files.h
	g++ -c generate_movie_knn.cpp

clean:
	rm generate_movie_knn movie_knn.o generate_movie_knn.o movie_knn_pearson.o

# that's all

