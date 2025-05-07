#include <stdio.h>
#include <stdint.h>
#include <x86intrin.h>
#include <string.h>

#define CACHE_LINE_SIZE 64 
#define BUFF_SIZE 256 

char buffer[BUFF_SIZE * CACHE_LINE_SIZE];

void flush_buffer() {
	for (int i = 0; i < CACHE_LINE_SIZE * BUFF_SIZE; i++)
		_mm_clflush(buffer + i);
}

void victim_process(int y) {
 	volatile char x = buffer[CACHE_LINE_SIZE * y];
}

uint64_t measure(void * addr) {
	uint64_t start, end;
	
	_mm_mfence();
	start = __rdtsc();
	
	volatile uint64_t y  = * (uint64_t *) addr;
	_mm_mfence();

	end = __rdtsc();
	
	uint64_t time = end - start;
	
	return time;
}

int main(int argc, char ** argv) {
	uint64_t measurements[256];

	memset(buffer, 0, sizeof(buffer));
	flush_buffer();

	for (int i = 0; i < BUFF_SIZE; i++) {
		
		_mm_clflush(buffer + i * CACHE_LINE_SIZE);
		victim_process(atoi(argv[1]));
		measurements[i] = measure(buffer + i*CACHE_LINE_SIZE);
		
	}
	for (int i = 0; i<BUFF_SIZE; i++)	{
		if (measurements[i] < 150)
			printf("Cache hit at %d. Time: %lu\n", i, measurements[i]);
	}

}
