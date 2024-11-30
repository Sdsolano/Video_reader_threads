#ifndef SYNCHRO_H
#define SYNCHRO_H

#include "ensitheora.h"
#include <stdbool.h>
#include <pthread.h>

extern bool fini;

/* Les extern des variables pour la synchro ici */
extern pthread_mutex_t mutex_window;
extern pthread_cond_t cond_window_size;
extern pthread_cond_t cond_window_ready;
extern bool window_size_received;
extern bool window_ready;

// Variables pour la synchro des textures
extern pthread_mutex_t mutex_texture;
extern pthread_cond_t cond_texture_empty;
extern pthread_cond_t cond_texture_full;
extern int texture_count;  // nombre de textures prêtes à être consommées

extern pthread_mutex_t mutex_hashmap;

void envoiTailleFenetre(th_ycbcr_buffer buffer);
void attendreTailleFenetre();

void attendreFenetreTexture();
void signalerFenetreEtTexturePrete();

void debutConsommerTexture();
void finConsommerTexture();

void debutDeposerTexture();
void finDeposerTexture();

#endif
