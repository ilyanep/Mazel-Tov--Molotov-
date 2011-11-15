all: generate_bias_file svd_executable

svd_executable: svd_executable.o svd.o ../binary_files/binary_files.cpp ../binary_files/binary_files.h ../write_data/write_results.o
	g++ -lgsl -lgslcblas -lm svd_executable.o svd.o ../binary_files/binary_files.o ../write_data/write_results.o -o svd_executable

generate_bias_file: generate_bias_file.o svd.h ../binary_files/binary_files.cpp ../binary_files/binary_files.h
	g++ -lgsl -lgslcblas -lm generate_bias_file.o svd.o ../binary_files/binary_files.o -o generate_bias_file

svd.o: svd.cpp ../binary_files/binary_files.cpp ../binary_files/binary_files.h
	g++ -lgsl -lgslcblas -lm -c svd.cpp -o svd.o

svd_executable.o: svd_executable.cpp svd.o
	g++ -lgsl -lgslcblas -lm -c svd_executable.cpp -o svd_executable.o

generate_bias_file.o: generate_bias_file.cpp svd.o ../binary_files/binary_files.cpp ../binary_files/binary_files.h
	g++ -lgsl -lgslcblas -lm -c generate_bias_file.cpp -o generate_bias_file.o

clean:
	rm *.o svd_executable generate_bias_file
