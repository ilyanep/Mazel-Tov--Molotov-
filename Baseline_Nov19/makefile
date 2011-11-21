baseline_executable: baseline_nov19.o ../binary_files/binary_files.o ../write_data/write_results.o
	g++ -lgsl -lgslcblas -lm baseline_executable.cpp baseline_nov19.o ../write_data/write_results.o ../binary_files/binary_files.o -o baseline_executable

baseline_nov19.o: baseline_nov19.cpp baseline_nov19.h ../binary_files/binary_files.o
	g++ -lgsl -lgslcblas -lm -c baseline_nov19.cpp ../binary_files/binary_files.o -o baseline_nov19.o

clean:
	rm *.o baseline_executable
