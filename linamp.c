#include "SDL.h"

static SDL_AudioDeviceID audio_device = 0;
static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;

static void panic_and_abort(const char *title, const char *text)
{
	fprintf(stderr, "PANIC: %s ... %s\n", title, text);
	SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, title, text, NULL);
	SDL_Quit();
	exit (1);	
}

int main(int argc, char **argv)
{
	SDL_AudioSpec wavspec;
	Uint8 *wavbuf = NULL;
	Uint32 wavlen = 0;

	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) == -1) {
		panic_and_abort("SDL_Init Falied", SDL_GetError());
	};
	
	window = SDL_CreateWindow("Linamp", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480, 0);
	if (!window) {
		panic_and_abort("SDL_CreateWindow Falied", SDL_GetError());
	}
	
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC);
	if (!renderer) {
		panic_and_abort("SDL_CreateRenderer Falied", SDL_GetError());
	}
	
	SDL_bool paused = SDL_TRUE;
	const SDL_Rect rewind_rect = {100, 100, 100, 100};
	const SDL_Rect pouse_rect = {400, 100, 100, 100};
	
	int green = 0;
	int keep_going = 1;
	
	while (keep_going){
		SDL_Event e;
		while (SDL_PollEvent(&e)) {
			switch (e.type) {
				case SDL_QUIT: 
					keep_going = 0;
					break;
					
				case SDL_MOUSEBUTTONDOWN: {
					const SDL_Point pt = { e.button.x, e.button.y};
					if (SDL_PointInRect(&pt, &rewind_rect)) {
						printf("Dentro do retângulo rewind\n");
						SDL_ClearQueuedAudio(audio_device);
						SDL_QueueAudio (audio_device, wavbuf, wavlen);
					} else if (SDL_PointInRect(&pt, &pouse_rect)) {
						paused = paused ? SDL_FALSE : SDL_TRUE;
						SDL_PauseAudioDevice(audio_device,paused);
						printf("Dentro do retângulo pause\n");
					}
					break;
				}
				case SDL_DROPFILE: {
					if (audio_device) {
						SDL_CloseAudioDevice(audio_device);
						audio_device = 0;
					}

					SDL_FreeWAV(wavbuf);
					wavbuf = NULL;
					wavlen = 0;

					if (SDL_LoadWAV (e.drop.file, &wavspec, &wavbuf, &wavlen) == NULL ) {
						SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,
												 "Não foi possível abrir o arquivo wav.",
												 SDL_GetError(), window);
					}

						#if 0
						SDL_AudioSpec desired;
						SDL_zero(desired);
						desired.freq = 48000;                    /**< DSP frequency -- samples per second */
						desired.format = AUDIO_F32;     		 /**< Audio data format */
						desired.channels = 2;             		 /**< Number of channels: 1 mono, 2 stereo */
						desired.samples = 4096;             	 /**< Audio buffer size in sample FRAMES (total samples divided by channel count) */
						desired.callback = NULL;				 /**< Callback that feeds the audio device (NULL to use SDL_QueueAudio()). */
						#endif

						audio_device = SDL_OpenAudioDevice (NULL, 0, &wavspec, NULL, 0);
						if (audio_device == 0) {
						SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,
												 "Não foi possível abrir o dispositivo de áudio.",
												 SDL_GetError(), window);

							SDL_FreeWAV (wavbuf);
							wavbuf = NULL;
							wavlen = 0;

						} else {
							paused = SDL_FALSE;
							SDL_QueueAudio (audio_device, wavbuf, wavlen);
							SDL_PauseAudioDevice(audio_device, paused);
						}
					SDL_free(e.drop.file);
					break;
				}
			}
		}

		SDL_SetRenderDrawColor(renderer, 0, green, 0, 200);
		SDL_RenderClear(renderer);
		SDL_SetRenderDrawColor (renderer, 255, 255, 255, 255);
		SDL_RenderFillRect (renderer, &rewind_rect );
		SDL_RenderFillRect (renderer, &pouse_rect );
		SDL_RenderPresent(renderer);
		green = (green + 1) % 256;
	}
	
	// Limpar toda a memória.
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_FreeWAV(wavbuf);	
	SDL_CloseAudioDevice(audio_device);
	SDL_Quit();
	return 0;
}
