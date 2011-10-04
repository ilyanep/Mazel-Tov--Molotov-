#binary_files/makefile
# simple makefile for the binary_files library
# files: source files: binary_files.cpp 
#        header files: 
# executable file: binary_files
#

# first define target file : binary_files
#              dependencies are the object files that build the program

binary_files: binary_files.o 
	g++ -o binary_files binary_files.o

binary_files.o: binary_files.cpp
	g++ -c binary_files.cpp

clean:
	rm binary_files binary_files.o

# that's all

