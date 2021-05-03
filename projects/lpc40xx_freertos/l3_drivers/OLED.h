#pragma once
#include "DAC_functions.h"
#include "OLED_spi.h"
#include "ff.h"
#include "key_functions.h"
#include "keypad.h"
#include <stddef.h> // size_t
#include <stdio.h>
#include <string.h>

SemaphoreHandle_t next_song;
SemaphoreHandle_t rewind_song;
typedef char song_memory_t[128];

typedef struct {
  char tagMark[3];
  char trackName[30];
  char artistName[30];
  char albumName[30];
  char year[4];
  char comment[30];
  char genre;
} Tag_s;

/* Do not declare variables in a header file */
#if 0
static song_memory_t list_of_songs[32];
static size_t number_of_songs;
#endif

void song_list__populate(void);
size_t song_list__get_item_count(void);
const char *song_list__get_name_for_item(size_t item_number);

void oled_songname_print(void);
void OLED_Finte_State_Machine(char key);

void add_that_song_to_queue();

void print_now_playing_page(bool play_pause);
void print_play_pause_icon(bool play_pause);

void OLED_song_end_play_page(void);