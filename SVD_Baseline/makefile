all: svd_executable

svd_executable: svd_executable.o learn_svd_baseline.o ../binary_files/binary_files.cpp ../binary_files/binary_files.h ../write_data/write_results.o ../Baseline/baseline_predictor.o
	g++ -lgsl -lgslcblas -lm svd_executable.o learn_svd_baseline.o ../binary_files/binary_files.o ../write_data/write_results.o ../Baseline/baseline_predictor.o -o svd_executable

learn_svd_baseline.o: learn_svd_baseline.cpp ../binary_files/binary_files.cpp ../binary_files/binary_files.h ../Baseline/baseline_predictor.o
	g++ -lgsl -lgslcblas -lm -c learn_svd_baseline.cpp ../Baseline/baseline_predictor.o -o learn_svd_baseline.o

svd_executable.o: svd_executable.cpp learn_svd_baseline.o
	g++ -lgsl -lgslcblas -lm -c svd_executable.cpp -o svd_executable.o

clean:
	rm *.o svd_executable
