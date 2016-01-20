censorship: censorship.cpp
	g++ censorship.cpp -o censorship -Wall --std=c++0x -O3 -lgmp

censorship_par: censorship_par.cpp
	g++ censorship_par.cpp -o censorship_par -Wall --std=c++0x -O3 -lgmp -pthread
