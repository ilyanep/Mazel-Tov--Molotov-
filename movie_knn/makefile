#movie_knn/makefile
# simple makefile for the movie_knn library
# files: source files: movie_knn.cpp movie_knn_pearson.cpp generate_movie_knn.cpp
#        header files: movie_knn.h movie_knn_pearson.h
# executable file: generate_movie_knn

all: movie_knn_train_pearson_1 movie_knn_train_pearson_2 movie_knn_train_pearson_3 movie_knn_train_pearson_4 generate_movie_knn movie_knn_train_pearson_with_residuals_3 movie_knn_train_pearson_with_alpha_3

movie_knn_train_pearson_1: ../binary_files/binary_files.o movie_knn.o movie_knn_pearson.o movie_knn_train_pearson_1.o 
	g++ -o movie_knn_train_pearson_1 ../binary_files/binary_files.o movie_knn.o movie_knn_pearson.o movie_knn_train_pearson_1.o

movie_knn_train_pearson_2: ../binary_files/binary_files.o movie_knn.o movie_knn_pearson.o movie_knn_train_pearson_2.o 
	g++ -o movie_knn_train_pearson_2 ../binary_files/binary_files.o movie_knn.o movie_knn_pearson.o movie_knn_train_pearson_2.o

movie_knn_train_pearson_3: ../binary_files/binary_files.o movie_knn.o movie_knn_pearson.o movie_knn_train_pearson_3.o 
	g++ -o movie_knn_train_pearson_3 ../binary_files/binary_files.o movie_knn.o movie_knn_pearson.o movie_knn_train_pearson_3.o

movie_knn_train_pearson_4: ../binary_files/binary_files.o movie_knn.o movie_knn_pearson.o movie_knn_train_pearson_4.o 
	g++ -o movie_knn_train_pearson_4 ../binary_files/binary_files.o movie_knn.o movie_knn_pearson.o movie_knn_train_pearson_4.o

movie_knn_train_pearson_with_alpha_3: ../binary_files/binary_files.o movie_knn.o movie_knn_pearson.o movie_knn_train_pearson_with_alpha_3.o movie_knn_pearson_with_alpha.o
	g++  -lgsl -lgslcblas -lm -o movie_knn_train_pearson_with_alpha_3 ../binary_files/binary_files.o movie_knn.o movie_knn_pearson.o movie_knn_train_pearson_with_alpha_3.o movie_knn_pearson_with_alpha.o

movie_knn_train_pearson_with_residuals_3: ../binary_files/binary_files.o movie_knn.o movie_knn_pearson.o movie_knn_train_pearson_with_residuals_3.o movie_knn_pearson_with_residuals.o   ../Baseline_Nov12/baseline_nov12.h   ../Baseline_Nov12/baseline_nov12.o
	g++  -lgsl -lgslcblas -lm -o movie_knn_train_pearson_with_residuals_3 ../binary_files/binary_files.o movie_knn.o movie_knn_pearson.o movie_knn_pearson_with_residuals.o movie_knn_train_pearson_with_residuals_3.o ../Baseline_Nov12/baseline_nov12.o

generate_movie_knn: ../binary_files/binary_files.o movie_knn.o movie_knn_pearson.o generate_movie_knn.o 
	g++ -o generate_movie_knn ../binary_files/binary_files.o movie_knn.o movie_knn_pearson.o generate_movie_knn.o

movie_knn.o: movie_knn.cpp movie_knn.h ../binary_files/binary_files.h
	g++ -c movie_knn.cpp

movie_knn_pearson.o: movie_knn_pearson.cpp movie_knn_pearson.h movie_knn.h ../binary_files/binary_files.h
	g++ -c movie_knn_pearson.cpp

movie_knn_pearson_with_residuals.o: movie_knn_pearson.cpp movie_knn_pearson.h movie_knn_pearson_with_residuals.h movie_knn.h ../binary_files/binary_files.h  ../Baseline_Nov12/baseline_nov12.h   ../Baseline_Nov12/baseline_nov12.o
	g++  -lgsl -lgslcblas -lm -c movie_knn_pearson_with_residuals.cpp

movie_knn_pearson_with_alpha.o: movie_knn_pearson.cpp movie_knn_pearson.h movie_knn_pearson_with_alpha.h movie_knn.h ../binary_files/binary_files.h
	g++  -lgsl -lgslcblas -lm -c movie_knn_pearson_with_alpha.cpp

generate_movie_knn.o: generate_movie_knn.cpp movie_knn.h movie_knn_pearson.h ../binary_files/binary_files.h
	g++ -c generate_movie_knn.cpp

movie_knn_train_pearson_1.o: movie_knn_train_pearson_1.cpp movie_knn.h movie_knn_pearson.h ../binary_files/binary_files.h
	g++ -c movie_knn_train_pearson_1.cpp

movie_knn_train_pearson_2.o: movie_knn_train_pearson_2.cpp movie_knn.h movie_knn_pearson.h ../binary_files/binary_files.h
	g++ -c movie_knn_train_pearson_2.cpp

movie_knn_train_pearson_3.o: movie_knn_train_pearson_3.cpp movie_knn.h movie_knn_pearson.h ../binary_files/binary_files.h
	g++ -c movie_knn_train_pearson_3.cpp

movie_knn_train_pearson_4.o: movie_knn_train_pearson_4.cpp movie_knn.h movie_knn_pearson.h ../binary_files/binary_files.h
	g++ -c movie_knn_train_pearson_4.cpp

movie_knn_train_pearson_with_residuals_3.o: movie_knn_train_pearson_with_residuals_3.cpp movie_knn.h movie_knn_pearson.h movie_knn_pearson_with_residuals.h ../binary_files/binary_files.h   ../Baseline_Nov12/baseline_nov12.h   ../Baseline_Nov12/baseline_nov12.o
	g++  -lgsl -lgslcblas -lm -c movie_knn_train_pearson_with_residuals_3.cpp

movie_knn_train_pearson_with_alpha_3.o: movie_knn_train_pearson_with_alpha_3.cpp movie_knn.h movie_knn_pearson.h movie_knn_pearson_with_alpha.h ../binary_files/binary_files.h
	g++  -lgsl -lgslcblas -lm -c movie_knn_train_pearson_with_alpha_3.cpp

clean:
	rm generate_movie_knn movie_knn.o generate_movie_knn.o movie_knn_pearson.o movie_knn_pearson_with_residuals.o movie_knn_pearson_with_alpha.o movie_knn_train_pearson_1.o movie_knn_train_pearson_1 movie_knn_train_pearson_2.o movie_knn_train_pearson_2 movie_knn_train_pearson_3.o movie_knn_train_pearson_3 movie_knn_train_pearson_4.o movie_knn_train_pearson_4 movie_knn_train_pearson_with_residuals_3.o movie_knn_train_pearson_with_residuals_3 movie_knn_train_pearson_with_alpha_3.o movie_knn_train_pearson_with_alpha_3

# that's all

