#include <SDL2/SDL.h>
#include <assert.h>
#include <stdio.h>
#include <unistd.h>

#include "oggstream.h"
#include "stream_common.h"
#include "synchro.h"
#include "ensitheora.h"
#include <pthread.h>

// Variable global para el thread
pthread_t theora2sdlthread;

int main(int argc, char *argv[]) {
  int res;

  if (argc != 2) {
    fprintf(stderr, "Usage: %s FILE", argv[0]);
    exit(EXIT_FAILURE);
  }
  assert(argc == 2);

  // Initialisation de la SDL
  res = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS);
  atexit(SDL_Quit);
  assert(res == 0);

  pthread_t thread_vorbis, thread_theora;
    
  // start the vorbis and theora threads
  res = pthread_create(&thread_theora, NULL, theoraStreamReader, argv[1]);
  assert(res == 0);
  
  res = pthread_create(&thread_vorbis, NULL, vorbisStreamReader, argv[1]);
  assert(res == 0);

  // wait for the end of the vorbis thread
  res = pthread_join(thread_vorbis, NULL);
  assert(res == 0);

  // 1 seconde of sound in advance
  sleep(1);

  // cancel the theora thread
  fini = true;  // signal the end of the streams
  res = pthread_cancel(thread_theora);
  assert(res == 0);
  res = pthread_cancel(theora2sdlthread);
  assert(res == 0);
  
  // wait for the end of the video thread
  res = pthread_join(thread_theora, NULL);
  assert(res == 0);
  res = pthread_join(theora2sdlthread, NULL);
  assert(res == 0);
  
  cleanup_streams();

  // cleanup
  pthread_mutex_destroy(&mutex_window);
  pthread_mutex_destroy(&mutex_texture);
  pthread_mutex_destroy(&mutex_hashmap);
  pthread_cond_destroy(&cond_window_size);
  pthread_cond_destroy(&cond_window_ready);
  pthread_cond_destroy(&cond_texture_empty);
  pthread_cond_destroy(&cond_texture_full);

  exit(EXIT_SUCCESS);
}