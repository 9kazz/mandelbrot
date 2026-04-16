TARGETS = intrin_ymm_O3 intrin_ymm_O2 intrin_ymm_O0 \
		  intrin_xmm_O3 intrin_xmm_O2 intrin_xmm_O0 \
		  unroll4_O3 unroll4_O2 \
		  unroll8_O3 unroll8_O2 \
		  no_opt_O3 no_opt_O2 no_opt_O0 \

# TARGETS = intrin_ymm_O3 intrin_ymm_O2 intrin_ymm_O0 \
# 		  intrin_xmm_O3 intrin_xmm_O2 intrin_xmm_O0 \
# 		  unroll4_O3 unroll8_O3 no_opt_O3 \
# 		  unroll4_O2 unroll4_O0 \
# 		  unroll8_O2 no_opt_O2 \
# 		  unroll8_O0 no_opt_O0 \
# 
.PHONY: all run clean

all: $(TARGETS)

intrin_ymm_O3: intrin_ymm.cpp
	g++ -O3 -DO3 -mavx2 -march=native intrin_ymm.cpp -o intrin_ymm_O3

intrin_ymm_O2: intrin_ymm.cpp
	g++ -O2 -DO2 -mavx2 -march=native intrin_ymm.cpp -o intrin_ymm_O2	

intrin_ymm_O0: intrin_ymm.cpp
	g++ -mavx2 -march=native intrin_ymm.cpp -o intrin_ymm_O0


intrin_xmm_O3: intrin_xmm.cpp
	g++ -mavx2 -march=native -O3 -DO3 intrin_xmm.cpp -o intrin_xmm_O3

intrin_xmm_O2: intrin_xmm.cpp
	g++ -mavx2 -march=native -O2 -DO2 intrin_xmm.cpp -o intrin_xmm_O2	

intrin_xmm_O0: intrin_xmm.cpp
	g++ -mavx2 -march=native intrin_xmm.cpp -o intrin_xmm_O0


unroll4_O3: unroll4.cpp
	g++ -O3 -DO3 -mavx2 -march=native unroll4.cpp -o unroll4_O3

unroll4_O2: unroll4.cpp
	g++ -O2 -DO2 -mavx2 -march=native unroll4.cpp -o unroll4_O2

unroll4_O0: unroll4.cpp
	g++ -mavx2 -march=native unroll4.cpp -o unroll4_O0	


unroll8_O3: unroll8.cpp
	g++ -O3 -DO3 -mavx2 -march=native unroll8.cpp -o unroll8_O3

unroll8_O2: unroll8.cpp
	g++ -O2 -DO2 -mavx2 -march=native unroll8.cpp -o unroll8_O2

unroll8_O0: unroll8.cpp
	g++ -mavx2 -march=native unroll8.cpp -o unroll8_O0	


no_opt_O3: no_opt.cpp
	g++ -mavx2 -march=native -O3 -DO3 no_opt.cpp -o no_opt_O3	

no_opt_O2: no_opt.cpp
	g++ -mavx2 -march=native -O2 -DO2 no_opt.cpp -o no_opt_O2	

no_opt_O0: no_opt.cpp
	g++ -mavx2 -march=native no_opt.cpp -o no_opt_O0	


run:
	for prog in $(TARGETS); do ./$$prog; done; \


clean:
	rm $(TARGETS)