#include <stdio.h>
#include <stdint.h>
#include <x86intrin.h>

int x = 1337;

int main(int argc, char ** argv) {
	uint64_t start, end;

	_mm_clflush(&x);
	_mm_mfence();

	start = __rdtsc();
	volatile int y = x;
	_mm_mfence();
	end = __rdtsc();

	printf("Access 1 took %ld cycles\n", end - start);

	start = __rdtsc();	
	volatile int z = x;
	_mm_mfence();
	end = __rdtsc();

	printf("Access 2 took %ld cycles\n", end - start);

	return 0;

}
