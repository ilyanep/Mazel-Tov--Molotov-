all: svd_executable

svd_executable: svd_executable.cpp svdk_nov13.o ../binary_files/binary_files.cpp ../binary_files/binary_files.h ../write_data/write_results.o ../Baseline_Nov12/baseline_nov12.o
	g++ -lgsl -lgslcblas -lm svd_executable.cpp svdk_nov13.o ../binary_files/binary_files.o ../write_data/write_results.o ../Baseline_Nov12/baseline_nov12.o -o svd_executable

svdk_nov13.o: svdk_nov13.cpp ../binary_files/binary_files.cpp ../binary_files/binary_files.h ../Baseline_Nov12/baseline_nov12.o
	g++ -lgsl -lgslcblas -lm -c svdk_nov13.cpp ../Baseline_Nov12/baseline_nov12.o -o svdk_nov13.o

clean:
	rm *.o svd_executable
