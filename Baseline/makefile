all: baseline_executable

baseline_executable: baseline_executable.o baseline_predictor.o ../binary_files/binary_files.cpp ../binary_files/binary_files.h ../write_data/write_results.o
	g++ -lgsl -lgslcblas -lm baseline_executable.o baseline_predictor.o ../binary_files/binary_files.o ../write_data/write_results.o -o baseline_executable

baseline_predictor.o: baseline_predictor.cpp ../binary_files/binary_files.cpp ../binary_files/binary_files.h
	g++ -lgsl -lgslcblas -lm -c baseline_predictor.cpp -o baseline_predictor.o

baseline_executable.o: baseline_executable.cpp baseline_predictor.o
	g++ -lgsl -lgslcblas -lm -c baseline_executable.cpp -o baseline_executable.o


clean:
	rm *.o baseline_executable
