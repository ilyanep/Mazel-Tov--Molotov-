write_files_test: write_files_test.o write_results.o
	g++ write_files_test.o write_results.o -o write_files_test

write_files_test.o: write_files_test.cpp write_results.cpp
	g++ -c write_files_test.cpp write_results.cpp

clean:
	rm write_files_test.o write_files_test
