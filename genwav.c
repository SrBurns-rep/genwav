#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <stdlib.h>
#include <varargs.h>

// Defaults
#define SAMPLE_RATE		44100
#define BITS_PER_SAMPLE 16
#define NUM_CHANNELS 	1
#define AMPLITUDE 		INT16_MAX
#define DURATION_SECS	10

typedef struct WavFile {
	union {
		uint8_t raw[44];
		struct {
			uint8_t RIFF[4]; 			// {'R', 'I', 'F', 'F'}
			uint32_t file_size;			// File size - 4 bytes
			uint8_t WAVE[4];			// {'W', 'A', 'V', 'E'}
			uint8_t fmt[4];				// "fmt" Format chunk marker. Includes trailing null
			uint32_t fmt_length;		// Length of the format data (16 for PCM)
			uint16_t fmt_type;			//
			uint16_t num_channels;		//
			uint32_t sample_rate; 		// 44100
			uint32_t byte_rate; 		// (Sample Rate * BitsPerSample * Channels) / 8
			uint16_t block_align; 		// (BitsPerSample * Channels) / 8
								  		// 1 - 8 bit mono
								  		// 2 - 8 bit stereo / 16 bit mono
								  		// 4 - 16 bit stereo
			uint16_t bits_per_sample;
			uint8_t DATA[4]; 		// {'d', 'a', 't', 'a'} Marks the beginning of the data section.
			uint32_t data_size;			// Size of the data section.
		}format;
	}header;

	int16_t data[];
}WavFile;

void sin_tone(WavFile *file, float amp, float base_freq) {

	const float sample_time = 1.f / SAMPLE_RATE;
	float dt = 0;


	for(size_t i = 0; i < file->header.format.data_size / 2; i++) {
		
		file->data[i] = (int16_t)( amp * sinf(base_freq * dt * 2 * M_PI));

		dt += sample_time;
		dt = fmodf(dt, 1);
	}
}

WavFile *alloc_file_mono(size_t sample_size) {

	const size_t num_channels = 1;

	size_t data_size = sample_size * sizeof(int16_t);
	size_t total_size = data_size + sizeof(WavFile);

	WavFile *file_buff = malloc(total_size);

	(*file_buff) = (WavFile){
		.header.format = {
			.RIFF = {'R', 'I', 'F', 'F'},
			.file_size = total_size,
			.WAVE = {'W', 'A', 'V', 'E'},
			.fmt = {'f', 'm', 't', ' '},
			.fmt_length = 16,
			.fmt_type = 1,
			.num_channels = num_channels,
			.sample_rate = SAMPLE_RATE,
			.byte_rate = (SAMPLE_RATE * BITS_PER_SAMPLE * num_channels) / 8,
			.block_align = (BITS_PER_SAMPLE * num_channels),
			.bits_per_sample = BITS_PER_SAMPLE,
			.DATA = {'d', 'a', 't', 'a'},
			.data_size = data_size
		}
	};


	return file_buff;
}

int main(void) {

	//const size_t data_size = SAMPLE_RATE * DURATION_SECS * 2;
	const size_t data_size = 101 * 2;
	const size_t total_size = sizeof(WavFile) + data_size;

	WavFile *file_buff = malloc(total_size);

	(*file_buff) = (WavFile){
		.header.format = {
			.RIFF = {'R', 'I', 'F', 'F'},
			.file_size = total_size,
			.WAVE = {'W', 'A', 'V', 'E'},
			.fmt = {'f', 'm', 't', ' '},
			.fmt_length = 16,
			.fmt_type = 1,
			.num_channels = NUM_CHANNELS,
			.sample_rate = SAMPLE_RATE,
			.byte_rate = (SAMPLE_RATE * BITS_PER_SAMPLE * NUM_CHANNELS) / 8,
			.block_align = (BITS_PER_SAMPLE * NUM_CHANNELS),
			.bits_per_sample = BITS_PER_SAMPLE,
			.DATA = {'d', 'a', 't', 'a'},
			.data_size = data_size
		}
	};

	const float sample_time = 1.f / SAMPLE_RATE;
	float dt = 0;
	const float base_freq = 440.f;
	size_t duration = 101;
	
	for(size_t i = 0; i < duration; i++) {
		file_buff->data[i] = sin_tone(dt, powf(2.f, 15.f) / 2.f, base_freq);

		dt += sample_time;
		dt = fmodf(dt, 1);
	}

	FILE *file = fopen("test.wav", "w");
	if(file == NULL){
		perror("fopen");
		return -1;
	}
	
	size_t bytes_wrote = fwrite(file_buff, 1, total_size, file);
	if(bytes_wrote == 0){
		perror("fwrite");
		return -1;
	}

	fclose(file);
	free(file_buff);

	return 0;
}
