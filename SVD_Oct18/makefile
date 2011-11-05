generate_bias_file: generate_bias_file.o svd_oct18.h ../binary_files/binary_files.cpp ../binary_files/binary_files.h
	g++ -lgsl -lgslcblas -lm generate_bias_file.o svd_oct18.o ../binary_files/binary_files.o -o generate_bias_file

svd_oct18.o: svd_oct18.cpp ../binary_files/binary_files.cpp ../binary_files/binary_files.h
	g++ -lgsl -lgslcblas -lm -c svd_oct18.cpp -o svd_oct18.o

svd_executable.o: svd_executable.cpp svd_oct18.o
	g++ -lgsl -lgslcblas -lm -c svd_executable.cpp -o svd_executable.o

generate_bias_file.o: generate_bias_file.cpp svd_oct18.o ../binary_files/binary_files.cpp ../binary_files/binary_files.h
	g++ -lgsl -lgslcblas -lm -c generate_bias_file.cpp -o generate_bias_file.o

clean:
	rm *.o svd_executable generate_bias_file
