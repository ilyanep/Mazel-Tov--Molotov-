#binary_files/makefile
# simple makefile for the binary_files library
# files: source files: binary_files.cpp generate_binary_files.cpp
#        header files: binary_files.h
# executable file: generate_binary_files
#

# first define target file : binary_files
#              dependencies are the object files that build the program

binary_files: binary_files.o generate_binary_files.o
	g++ -o generate_binary_files binary_files.o generate_binary_files.o

binary_files.o: binary_files.cpp binary_files.h
	g++ -c binary_files.cpp

generate_binary_files.o: generate_binary_files.cpp binary_files.h
	g++ -c generate_binary_files.cpp

clean:
	rm generate_binary_files binary_files.o generate_binary_files.o

# that's all

