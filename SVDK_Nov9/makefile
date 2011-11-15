all: generate_bias_file svd_executable

svd_executable: svd_executable.o svdk_nov9.o ../binary_files/binary_files.cpp ../binary_files/binary_files.h ../write_data/write_results.o
	g++ -lgsl -lgslcblas -lm svd_executable.o svdk_nov9.o ../binary_files/binary_files.o ../write_data/write_results.o -o svd_executable

generate_bias_file: generate_bias_file.o svdk_nov9.h ../binary_files/binary_files.cpp ../binary_files/binary_files.h
	g++ -lgsl -lgslcblas -lm generate_bias_file.o svdk_nov9.o ../binary_files/binary_files.o -o generate_bias_file

svdk_nov9.o: svdk_nov9.cpp ../binary_files/binary_files.cpp ../binary_files/binary_files.h
	g++ -lgsl -lgslcblas -lm -c svdk_nov9.cpp -o svdk_nov9.o

svd_executable.o: svd_executable.cpp svdk_nov9.o
	g++ -lgsl -lgslcblas -lm -c svd_executable.cpp -o svd_executable.o

generate_bias_file.o: generate_bias_file.cpp svdk_nov9.o ../binary_files/binary_files.cpp ../binary_files/binary_files.h
	g++ -lgsl -lgslcblas -lm -c generate_bias_file.cpp -o generate_bias_file.o

clean:
	rm *.o svd_executable generate_bias_file
