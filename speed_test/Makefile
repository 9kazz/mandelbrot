.PHONY: all clean

all: intrin_ymm	intrin_xmm unroll4_O3 unroll4_O0 no_opt_O3 no_opt_O0

intrin_ymm: intrin_ymm.cpp
	g++ -mavx -O3 intrin_ymm.cpp -o intrin_ymm

intrin_xmm: intrin_xmm.cpp
	g++ -mavx -O3 intrin_xmm.cpp -o intrin_xmm	

unroll4_O3: unroll4.cpp
	g++ -O3 unroll4.cpp -o unroll4_O3

unroll4_O0: unroll4.cpp
	g++ unroll4.cpp -o unroll4_O0	

no_opt_O3: no_opt.cpp
	g++ -O3 no_opt.cpp -o no_opt_O3	

no_opt_O0: no_opt.cpp
	g++ -O3 no_opt.cpp -o no_opt_O0	

clean:
	rm intrin_ymm intrin_xmm unroll4_O3 unroll4_O0 no_opt_O3 no_opt_O0