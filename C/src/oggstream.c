#include "ensitheora.h"
#include "ensivorbis.h"
#include "stream_common.h"
#include "synchro.h"
#include <assert.h>
#include <ogg/ogg.h>
#include <stdio.h>

ogg_sync_state oggtheorastate,
    oggvorbisstate; /* sync and verify incoming physical bitstream */
ogg_page theorapage,
    vorbispage; /* one Ogg bitstream page. Vorbis packets are inside */

void *theoraStreamReader(void *arg) {
  printf("Starting theora stream reader\n");
  char *filename = (char *)arg;

  FILE *vf = fopen(filename, "r");
  if (!vf) {
    printf("Error opening file for theora\n");
  }
  assert(vf);
   printf("Theora file opened successfully\n");

  // Initialisation de l'automate OGG
  int res = ogg_sync_init(&oggtheorastate);
  printf("Theora ogg_sync_init result: %d\n", res);
  assert(res == 0);

  int respac = 0;
  struct streamstate *s;

  while (!fini) {
    printf("theora loop\n");// vérifier si le fichier ne serait pas fini
    if (feof(vf)) {
      printf("End of file reached\n");
      fini = true;
      fclose(vf);
      return 0;
    }

    if (respac == 0) {
      printf("Reading page\n");
      pageReader(vf, &oggtheorastate, &theorapage);
      s = getStreamState(&oggtheorastate, &theorapage, TYPE_THEORA);

      // ignorer le stream vorbis
      if (s->strtype == TYPE_VORBIS){
        printf("Skipping vorbis stream\n");
        continue;
      }
      respac = addPageGetPacket(&theorapage, s);
      printf("Page read, respac = %d\n", respac); 
    } else {
      respac = getPacket(s);
      printf("Got packet, respac = %d\n", respac);
    }
    switch (respac) {
    case -1:
      s->nbpacketoutsync++;
      printf("out of sync: gap in data\n");
      break;
    case 0:
      // more pages (data) are needed to build a full packet
      continue;
      break;
    case 1:
      s->nbpacket++;
      break;
    }

    if (decodeAllHeaders(respac, s, TYPE_THEORA)){
      printf("Headers decoded\n");
      continue;
    }
    if (s->strtype == TYPE_THEORA && s->headersRead) {
      printf("Processing theora frame\n");
      theora2SDL(s);
    }
  }
  fclose(vf);
  return 0;
}

void *vorbisStreamReader(void *arg) {
  printf("Starting vorbis stream reader\n");
  char *filename = (char *)arg;

  FILE *vf = fopen(filename, "r");
  if (!vf) {
        printf("Error opening file for vorbis\n");  // Añadir esta línea
  }
  assert(vf);

  // Initialisation de l'automate OGG
  int res = ogg_sync_init(&oggvorbisstate);
  assert(res == 0);

  int respac = 0;
  struct streamstate *s;

  while (!fini) {
    // printf ("vorbis loop \n");
    // vérifier si le fichier ne serait pas fini
    if (feof(vf)) {
      fini = true;
      printf("FIN de la lecture de VORBIS !");
      break;
    }

    if (respac == 0) {
      pageReader(vf, &oggvorbisstate, &vorbispage);
      s = getStreamState(&oggvorbisstate, &vorbispage, TYPE_VORBIS);

      // ignorer le stream theora
      if (s->strtype == TYPE_THEORA)
        continue;

      // ajouter la page dans le décodeur et tenter d'extraire un
      // packet
      respac = addPageGetPacket(&vorbispage, s);
    } else {
      respac = getPacket(s);
    }

    switch (respac) {
    case -1:
      s->nbpacketoutsync++;
      printf("out of sync: gap in data\n");
      break;
    case 0:
      // more pages (data) are needed to build a full packet
      continue;
      break;
    case 1:
      s->nbpacket++;
      break;
    }

    if (decodeAllHeaders(respac, s, TYPE_THEORA))
      continue;

    // boucle principale de lecture vorbis
    if (s->strtype == TYPE_VORBIS && s->headersRead) {
      vorbis2SDL(s);
    }
  }

  fclose(vf);
  return 0;
}
