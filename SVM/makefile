all: gen_svm_datafiles

gen_svm_datafiles: svd_svm_nov6.cpp gen_svm_datafiles.cpp 
	g++ -lgsl -lgslcblas -lm gen_svm_datafiles.cpp svd_svm_nov6.cpp ../Baseline_Oct25/baseline_oct25.o ../binary_files/binary_files.o -o gen_svm_datafiles

clean:
	rm *.o gen_svm_datafiles
