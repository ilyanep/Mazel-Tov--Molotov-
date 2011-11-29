all: svd_executable

svd_executable: svd_executable.cpp timesvdpp_nov26.o ../binary_files/binary_files.cpp ../binary_files/binary_files.h ../write_data/write_results.o
	g++ -lgsl -lgslcblas -lm svd_executable.cpp timesvdpp_nov26.o ../binary_files/binary_files.o ../write_data/write_results.o -o svd_executable

timesvdpp_nov26.o: timesvdpp_nov26.cpp ../binary_files/binary_files.cpp ../binary_files/binary_files.h
	g++ -lgsl -lgslcblas -lm -c timesvdpp_nov26.cpp -o timesvdpp_nov26.o

clean:
	rm *.o svd_executable
