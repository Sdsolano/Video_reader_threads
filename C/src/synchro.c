#include "ensitheora.h"
#include "synchro.h"
#include <pthread.h>

/* Sync variables init */
pthread_mutex_t mutex_window = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_window_size = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond_window_ready = PTHREAD_COND_INITIALIZER;
bool window_size_received = false;
bool window_ready = false;

pthread_mutex_t mutex_texture = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_texture_empty = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond_texture_full = PTHREAD_COND_INITIALIZER;
int texture_count = 0;

pthread_mutex_t mutex_hashmap = PTHREAD_MUTEX_INITIALIZER;

void envoiTailleFenetre(th_ycbcr_buffer buffer) {
    pthread_mutex_lock(&mutex_window);
    windowsx = buffer[0].width;
    windowsy = buffer[0].height;
    window_size_received = true;
    pthread_cond_signal(&cond_window_size);
    pthread_mutex_unlock(&mutex_window);
}

void attendreTailleFenetre() {
    pthread_mutex_lock(&mutex_window);
    while (!window_size_received) {
        pthread_cond_wait(&cond_window_size, &mutex_window);
    }
    pthread_mutex_unlock(&mutex_window);
}

void signalerFenetreEtTexturePrete() {
    pthread_mutex_lock(&mutex_window);
    window_ready = true;
    pthread_cond_broadcast(&cond_window_ready);
    pthread_mutex_unlock(&mutex_window);
}

void attendreFenetreTexture() {
    pthread_mutex_lock(&mutex_window);
    while (!window_ready) {
        pthread_cond_wait(&cond_window_ready, &mutex_window);
    }
    pthread_mutex_unlock(&mutex_window);
}

void debutConsommerTexture() {
    pthread_mutex_lock(&mutex_texture);
    while (texture_count == 0) {
        pthread_cond_wait(&cond_texture_full, &mutex_texture);
    }
    pthread_mutex_unlock(&mutex_texture);
}

void finConsommerTexture() {
    pthread_mutex_lock(&mutex_texture);
    texture_count--;
    pthread_cond_signal(&cond_texture_empty);
    pthread_mutex_unlock(&mutex_texture);
}

void debutDeposerTexture() {
    pthread_mutex_lock(&mutex_texture);
    while (texture_count >= NBTEX) {
        pthread_cond_wait(&cond_texture_empty, &mutex_texture);
    }
    pthread_mutex_unlock(&mutex_texture);
}

void finDeposerTexture() {
    pthread_mutex_lock(&mutex_texture);
    texture_count++;
    pthread_cond_signal(&cond_texture_full);
    pthread_mutex_unlock(&mutex_texture);
}