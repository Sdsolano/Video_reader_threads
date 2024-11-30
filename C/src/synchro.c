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

/* l'implantation des fonctions de synchro ici */
void envoiTailleFenetre(th_ycbcr_buffer buffer) {
    printf("Sending window size: %dx%d\n", buffer[0].width, buffer[0].height);
    pthread_mutex_lock(&mutex_window);
    windowsx = buffer[0].width;
    windowsy = buffer[0].height;
    window_size_received = true;
    pthread_cond_signal(&cond_window_size);
    pthread_mutex_unlock(&mutex_window);
}

void attendreTailleFenetre() {
    printf("Waiting for window size\n");
    pthread_mutex_lock(&mutex_window);
    while (!window_size_received) {
        pthread_cond_wait(&cond_window_size, &mutex_window);
    }
    pthread_mutex_unlock(&mutex_window);
    printf("Got window size: %dx%d\n", windowsx, windowsy);
}

void signalerFenetreEtTexturePrete() {
    pthread_mutex_lock(&mutex_window);
    window_ready = true;
    pthread_cond_broadcast(&cond_window_ready);
    pthread_mutex_unlock(&mutex_window);
    printf("Window and texture ready signaled\n");
}

void attendreFenetreTexture() {
    pthread_mutex_lock(&mutex_window);
    while (!window_ready) {
        pthread_cond_wait(&cond_window_ready, &mutex_window);
    }
    pthread_mutex_unlock(&mutex_window);
    printf("Window texture ready received\n");
}

void debutConsommerTexture() {
    pthread_mutex_lock(&mutex_texture);
    while (texture_count == 0) {
        pthread_cond_wait(&cond_texture_full, &mutex_texture);  // changed to full
    }
}

void finConsommerTexture() {
    texture_count--;
    pthread_cond_signal(&cond_texture_empty);  // We signal that there is a free texture
    pthread_mutex_unlock(&mutex_texture);
}

void debutDeposerTexture() {
    pthread_mutex_lock(&mutex_texture);
    while (texture_count >= NBTEX) {
        pthread_cond_wait(&cond_texture_empty, &mutex_texture);  // Wait for an empty texture
    }
}

void finDeposerTexture() {
    texture_count++;
    pthread_cond_signal(&cond_texture_full);  // Signal that there is a full texture
    pthread_mutex_unlock(&mutex_texture);
}