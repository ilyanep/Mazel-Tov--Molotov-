all: rbm_executable rbm_executable2

rbm_executable: rbm_executable.o rbm.o ../binary_files/binary_files.o
	g++ -O2 rbm_executable.o rbm.o ../binary_files/binary_files.o -o rbm_executable

rbm_executable2: rbm_executable2.o rbm.o ../binary_files/binary_files.o
	g++ -O2 rbm_executable2.o rbm.o ../binary_files/binary_files.o -o rbm_executable2

rbm_executable.o: rbm_executable.cpp
	g++ -O2 -c rbm_executable.cpp -o rbm_executable.o

rbm_executable2.o: rbm_executable2.cpp
	g++ -O2 -c rbm_executable2.cpp -o rbm_executable2.o

rbm.o: rbm.cpp rbm.h 
	g++ -O2 -c rbm.cpp

../binary_files/binary_files.o::
	@$(MAKE) -C $(@D) $(@F) -q || \
	$(MAKE) -C $(@D) $(@F)

clean:
	rm *.o rbm_executable
