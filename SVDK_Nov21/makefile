all: svd_executable

svd_executable: svd_executable.cpp svdk_nov21.o ../binary_files/binary_files.cpp ../binary_files/binary_files.h ../write_data/write_results.o
	g++ -lgsl -lgslcblas -lm svd_executable.cpp svdk_nov21.o ../binary_files/binary_files.o ../write_data/write_results.o -o svd_executable

svdk_nov21.o: svdk_nov21.cpp ../binary_files/binary_files.cpp ../binary_files/binary_files.h
	g++ -lgsl -lgslcblas -lm -c svdk_nov21.cpp -o svdk_nov21.o

clean:
	rm *.o svd_executable
