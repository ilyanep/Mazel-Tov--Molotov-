all: svm_executable svd_svm_nov6.o

svm_executable: svd_svm_nov6.o svm_executable.cpp libsvm-3.1/svm.cpp
	g++ -lgsl -lgslcblas -lm svm_executable.cpp svd_svm_nov6.o ../Baseline_Oct25/baseline_oct25.o ../binary_files/binary_files.o ../write_data/write_results.o libsvm-3.1/svm.cpp -o svm_executable

svd_svm_nov6.o: svd_svm_nov6.cpp
	g++ -lgsl -lgslcblas -lm -c svd_svm_nov6.cpp -o svd_svm_nov6.o

clean:
	rm *.o svm_executable
