EXE1=image.out
EXE2=video.out

all:
	#g++ -o $(EXE1) `pkg-config --cflags opencv` tp1.cpp `pkg-config --libs opencv`
	clang++ -o $(EXE2) `pkg-config --cflags opencv` video.cpp `pkg-config --libs opencv`

clean:
	rm -rf *.o *.out *~
