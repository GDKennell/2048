all: 2048.cpp SmallBoard.h move_precompute.cpp heur_precompute.cpp precompute.h
	g++ -g -std=c++11 2048.cpp heur_precompute.cpp move_precompute.cpp -o Play2048
