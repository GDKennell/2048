all: 2048.cpp SmallBoard.h move_precompute.cpp heur_precompute.cpp precompute.h clink.o
	g++ -g -std=c++11 2048.cpp heur_precompute.cpp move_precompute.cpp clink.o -o Play2048

clean:
	rm -f ./*.cl.c
	rm -f ./*.cl.h
	rm -f ./*.o
	rm -f ./*.bc
	rm -f test

test: test.c compute_moves.cl
	/System/Library/Frameworks/OpenCL.framework/Libraries/openclc -x cl -cl-std=CL1.1 -cl-auto-vectorize-enable -emit-gcl compute_moves.cl
	/System/Library/Frameworks/OpenCL.framework/Libraries/openclc -x cl -cl-std=CL1.1 -Os -triple i386-applecl-darwin -emit-llvm-bc -o compute_moves.cl.i386.bc compute_moves.cl
	/System/Library/Frameworks/OpenCL.framework/Libraries/openclc -x cl -cl-std=CL1.1 -Os -triple x86_64-applecl-darwin -emit-llvm-bc -o compute_moves.cl.x86_64.bc compute_moves.cl
	/System/Library/Frameworks/OpenCL.framework/Libraries/openclc -x cl -cl-std=CL1.1 -Os -triple gpu_32-applecl-darwin -emit-llvm-bc -o compute_moves.cl.gpu_32.bc compute_moves.cl
	clang -c -Os -Wall -arch x86_64 -o compute_moves.cl.o compute_moves.cl.c
	clang -c -Os -Wall -arch x86_64 -o test.o test.c
	clang -framework OpenCL -o test compute_moves.cl.o test.o

clink.o: clink.c clink.h
	clang -c -Os -Wall -arch x86_64 -o clink.o clink.c

