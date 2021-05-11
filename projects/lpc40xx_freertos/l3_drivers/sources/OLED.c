#include "OLED.h"

extern QueueHandle_t Q_key_to_OLED;
extern QueueHandle_t Q_songname;
extern bool song_play_status;
extern bool SKIP_status;
// extern char byte_128_metadata[128];
extern Tag_s mp3_tag;

extern uint16_t volume_value;
extern uint16_t bass_treble_value;

static song_memory_t list_of_songs[32]; /// 128 locations 2-demention char array,
/// each array location contain 32 byte.
static size_t number_of_songs;

typedef char metadata_t[128];
metadata_t metadata = {0};

// static char key_press;

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//
//                    S O N G  R E A D   F U N C T I O N S
//
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
static void song_list__handle_filename(const char *filename) {
  // This will not work for cases like "file.mp3.zip"
  if (NULL != strstr(filename, ".mp3")) {
    // printf("Filename: %s\n", filename);

    // Dangerous function: If filename is > 128 chars, then it will copy extra bytes leading to memory corruption
    // strcpy(list_of_songs[number_of_songs], filename);

    // Better: But strncpy() does not guarantee to copy null char if max length encountered
    // So we can manually subtract 1 to reserve as NULL char
    strncpy(list_of_songs[number_of_songs], filename, sizeof(song_memory_t) - 1);

    // Best: Compensates for the null, so if 128 char filename, then it copies 127 chars, AND the NULL char
    // snprintf(list_of_songs[number_of_songs], sizeof(song_memory_t), "%.149s", filename);

    ++number_of_songs;
    // or
    // number_of_songs++;
  }
}

void song_list__populate(void) {
  FRESULT res;
  static FILINFO file_info;
  const char *root_path = "/";

  DIR dir;
  res = f_opendir(&dir, root_path);

  if (res == FR_OK) {
    for (;;) {
      res = f_readdir(&dir, &file_info); /* Read a directory item */
      if (res != FR_OK || file_info.fname[0] == 0) {
        break; /* Break on error or end of dir */
      }

      if (file_info.fattrib & AM_DIR) {
        /* Skip nested directories, only focus on MP3 songs at the root */
      } else { /* It is a file. */
        song_list__handle_filename(file_info.fname);
      }
    }
    f_closedir(&dir);
  }
}

size_t song_list__get_item_count(void) { return number_of_songs; }

const char *song_list__get_name_for_item(size_t item_number) {
  const char *return_pointer = "";

  if (item_number >= number_of_songs) {
    return_pointer = "";
  } else {
    return_pointer = list_of_songs[item_number];
  }

  return return_pointer;
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//
//                 O L E D  D I S P L A Y   F U N C T I O N S
//
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

/********keypad keys usage Idel State************/
//    [vol-]    [arrow_down]   [vol+]     []
//
//      []      [play/pause]     []       []
//
//   [replay]   [arrow_down]   [skip]     []
//
//  [add&play]       []          []       []
/*************************************************/
/********keypad keys usage Play State*************/
//    [vol-]         []       [vol+]     [bass+]
//
//      []      [play/pause]    []       [bass-]
//
//   [replay]        []       [skip]     [treble+]
//
//  [add&play]       []    [back idel]   [treble-]
/*************************************************/

void oled_songname_print(void) {

  song_list__populate();
  for (size_t song_number = 0; song_number < 6; song_number++) {
    // fprintf(stderr, "Song %2d: %s\n", (1 + song_number), song_list__get_name_for_item(song_number));
    OLED_print_string((uint8_t)song_number + 2, 1, 0, song_list__get_name_for_item(song_number), 16);
  }
}

typedef enum {
  Idle_State,
  Play_State,
  Info_State,
} OLED_State;

// static OLED_fsm f;
// f.state = Idle_State;

static int state = Idle_State; // has to be declared outside of the state machine
static int arrow_location = 2;
static size_t song_index = 0;

#define OLED__ENABLE_DEBUGGING 0

#if OLED__ENABLE_DEBUGGING
#include <stdio.h>
#define OLED__DEBUG_PRINTF(f_, ...)                                                                                    \
  do {                                                                                                                 \
    fprintf(stderr, "OLED");                                                                                           \
    fprintf(stderr, (f_), ##__VA_ARGS__);                                                                              \
    fprintf(stderr, "\n");                                                                                             \
  } while (0)
#else
#define OLED__DEBUG_PRINTF(f_, ...) /* NOOP */
#endif

// static bool play_pause_status = true;

static void print_song_list_page(void) { // back to main page
  song_index = 0;
  arrow_location = 2;                       // reset indexes
  state = Idle_State;                       // reset to idle state
  OLED_Horizontal_Scroll(3, 3, 7, 0x27, 0); // disable scrolling
  OLED_print_string(0, 0, 0, "     [MP3]      ", 16);
  for (size_t song_number = 0; song_number < 6; song_number++) {
    // fprintf(stderr, "Song %2d: %s\n", (1 + song_number), song_list__get_name_for_item(song_number));
    OLED_print_string((uint8_t)song_number + 2, 1, 0, song_list__get_name_for_item(song_number), 16);
  }
}

void add_that_song_to_queue() {
  size_t that_song_number = song_index;
  char *that_song_name = song_list__get_name_for_item(that_song_number);
  // songname_backup new_name = list_of_songs[arrow_location];
  fprintf(stderr, "%s has been added to queue\n", that_song_name);
  xQueueSend(Q_songname, that_song_name, 0);
}

void play_the_song(void) {
  if (!song_play_status) {
    add_that_song_to_queue();
  } else {
    xSemaphoreGive(next_song);
    add_that_song_to_queue();
  }
}

void skip_the_song(void) {
  if (!SKIP_status) {
    SKIP_status = true;
    xSemaphoreGive(next_song);
  } else {
    ;
  }
}

void OLED_song_end_play_page(void) { print_song_list_page(); }

void print_now_playing_page(bool play_pause) {
  if (play_pause) {
    OLED_print_string(1, 0, 0, "----------------", 16);
    OLED_print_string(2, 0, 0, "                ", 16);
    OLED_print_string(3, 1, 0, song_list__get_name_for_item(song_index), 16);
    OLED_print_string(4, 0, 0, "                ", 16);
    OLED_print_string(5, 0, 0, "    Playing...  ", 16);
    OLED_print_string(6, 0, 0, "                ", 16);
    OLED_print_string(7, 0, 0, "                ", 16);
  } else {
    OLED_print_string(1, 0, 0, "----------------", 16);
    OLED_print_string(2, 0, 0, "                ", 16);
    OLED_print_string(3, 1, 0, song_list__get_name_for_item(song_index), 16);
    OLED_print_string(4, 0, 0, "                ", 16);
    OLED_print_string(5, 0, 0, "    Pausing...  ", 16);
    OLED_print_string(6, 0, 0, "                ", 16);
    OLED_print_string(7, 0, 0, "                ", 16);
  }
}

void print_play_pause_icon(bool play_pause) {
  if (play_pause) {
    OLED_print_string(0, 15, 0, "]>", 2);
  } else {
    OLED_print_string(0, 15, 0, "]]", 2);
  }
}

static void OLED_print_volume(bool plus_or_minus) {
  if (plus_or_minus) {
    OLED_print_string(0, 0, 0, "vol+ ", 5);
    vTaskDelay(200);
    OLED_print_string(0, 0, 0, "     ", 5);
  } else {
    OLED_print_string(0, 0, 0, "vol- ", 5);
    vTaskDelay(200);
    OLED_print_string(0, 0, 0, "     ", 5);
  }
}

static void OLED_print_bass(bool plus_or_minus) {
  if (plus_or_minus) {
    OLED_print_string(0, 0, 0, "bass+", 5);
    vTaskDelay(200);
    OLED_print_string(0, 0, 0, "     ", 5);
  } else {
    OLED_print_string(0, 0, 0, "bass-", 5);
    vTaskDelay(200);
    OLED_print_string(0, 0, 0, "     ", 5);
  }
}

static void OLED_print_treble(bool plus_or_minus) {
  if (plus_or_minus) {
    OLED_print_string(0, 0, 0, "treb+", 5);
    vTaskDelay(200);
    OLED_print_string(0, 0, 0, "     ", 5);
  } else {
    OLED_print_string(0, 0, 0, "treb-", 5);
    vTaskDelay(200);
    OLED_print_string(0, 0, 0, "     ", 5);
  }
}

void OLED_print_volume_value(int value) {
  // char dB[1] = 'dB';
  char str[12];
  sprintf(str, "volume: -%ddB", value);
  OLED_print_string(7, 0, 0, str, 13);
  vTaskDelay(300);
  OLED_print_string(7, 0, 0, "             ", 13);
}

void OLED_print_bass_value(int value) {
  char str[12];
  sprintf(str, "bass: +%ddB", value);
  OLED_print_string(7, 0, 0, str, 13);
  vTaskDelay(300);
  OLED_print_string(7, 0, 0, "             ", 13);
}

void OELD_print_treble_value(int value) {
  char str[13];
  sprintf(str, "treble: %ddB", value);
  OLED_print_string(7, 0, 0, str, 14);
  vTaskDelay(300);
  OLED_print_string(7, 0, 0, "             ", 14);
}

static void song_page_roll(bool up_or_down) {
  if (up_or_down) { // roll up to see next song
    for (size_t count = 6; count > 0; count--) {
      OLED_print_string(8 - count, 1, 0, song_list__get_name_for_item(song_index - (count - 2)), 16);
    }
  } else { // roll down to see prev song
    for (size_t count = 6; count > 0; count--) {
      OLED_print_string(8 - count, 1, 0, song_list__get_name_for_item(song_index - (count)), 16);
    }
  }
}
// fprintf(stderr, "mp3 name: %s\n", mp3_tag.trackName);
// fprintf(stderr, "artist Name: %s\n", mp3_tag.artistName);
// fprintf(stderr, "album Name: %s\n", mp3_tag.albumName);
// fprintf(stderr, "year: %s\n", mp3_tag.year);
void OLED_print_song_info(void) {
  OLED_print_string(1, 0, 0, "----------------", 16);
  OLED_print_string(2, 0, 0, mp3_tag.trackName, 16);
  OLED_print_string(3, 0, 0, " Artist Name:   ", 16);
  OLED_print_string(4, 0, 0, mp3_tag.artistName, 16);
  OLED_print_string(5, 0, 0, " Album Name:    ", 16);
  OLED_print_string(6, 0, 0, mp3_tag.albumName, 16);
  OLED_print_string(7, 0, 0, mp3_tag.year, 16);
}

void OLED_Finte_State_Machine(char key) {
  // OLED__DEBUG_PRINTF("current state: %d\n", state);
  switch (state) {
  case Idle_State:
    switch (key) {
    case '2': // move up the arrow
      OLED__DEBUG_PRINTF(" state:%d arrow up", state);
      if (song_index > 0) {
        if (song_index < 6) {
          if (arrow_location == 2) {
            // state = Idle_State
            // arrow_location--;
            // song_index--;
            // song_page_roll(0);

            OLED__DEBUG_PRINTF(" 1 song index: %d", song_index);
            OLED__DEBUG_PRINTF(" 1 arrow location: %d", arrow_location);
            break;
          } else if (arrow_location > 2) {
            OLED_print_string(arrow_location, 0, 0, "  ", 2);
            OLED_print_string((arrow_location - 1), 0, 0, ">>", 2);
            arrow_location--;
            song_index--;
            // song_page_roll(0);
            OLED__DEBUG_PRINTF(" 2 song index: %d", song_index);
            OLED__DEBUG_PRINTF(" 2 arrow location: %d", arrow_location);
            // state = Idle_State;
            break;
          }
        } else { // song index >= 6
          song_page_roll(0);
          OLED_print_string(arrow_location, 0, 0, ">>", 2);
          song_index--;
          OLED__DEBUG_PRINTF(" 3 song index: %d", song_index);
          OLED__DEBUG_PRINTF(" 3 arrow location: %d", arrow_location);
          break;
        }
      } else
        OLED__DEBUG_PRINTF(stderr, "song index is %d", song_index);
      break;

    case '8': // move down the arrow
      OLED__DEBUG_PRINTF(" state:%d arrow down", state);
      if (song_index < (song_list__get_item_count() - 1)) {

        if (arrow_location == 7) {
          // state = Idle_State;
          song_page_roll(1);
          // OLED_print_string(arrow_location - 7, 0, 0, "  ", 2);
          // OLED_print_string((arrow_location - 6), 0, 0, ">>", 2);
          OLED_print_string(arrow_location, 0, 0, ">>", 2);
          song_index++;
          OLED__DEBUG_PRINTF(" 4 song index: %d", song_index);
          OLED__DEBUG_PRINTF(" 4 arrow location: %d", arrow_location);
          break;
        } else if (arrow_location < 7) {
          OLED_print_string(arrow_location, 0, 0, "  ", 2);
          OLED_print_string((arrow_location + 1), 0, 0, ">>", 2);
          arrow_location++;
          song_index++;
          OLED__DEBUG_PRINTF(" 5 song index: %d", song_index);
          OLED__DEBUG_PRINTF(" 5 arrow location: %d", arrow_location);
          // state = Idle_State;
          break;
        }
      } else
        OLED__DEBUG_PRINTF("there are only %d songs", song_index);
      break;
    case '1': // vol--
      OLED__DEBUG_PRINTF(" state:%d vol --", state);
      OLED_print_volume(0);
      break;

    case '3': // vol++
      OLED__DEBUG_PRINTF(" state:%d vol ++", state);
      OLED_print_volume(1);
      break;

    case '*': // add song to queue
      OLED__DEBUG_PRINTF(" state:%d play song", state);
      play_the_song();
      print_now_playing_page(1);
      print_play_pause_icon(1);
      OLED_Horizontal_Scroll(3, 3, 7, 0x27, 1);

      state = Play_State;
      break;

    case '#':
      OLED__DEBUG_PRINTF(" state:%d already in main", state);
      // state = Idle_State;

      print_song_list_page();
      break;

    case '0':
      break;

    case '9':
      skip_the_song();
      break;

    default:

      break;
    }
    break;

  case Play_State:
    switch (key) {
    case '1': // vol--
      OLED__DEBUG_PRINTF(" state:%d vol --", state);
      // OLED_print_volume(0);
      OLED_print_volume_value(volume_value & 0xFF);
      OLED_Horizontal_Scroll(3, 3, 7, 0x27, 1);
      break;

    case '2':
      break;

    case '3': // vol++
      OLED__DEBUG_PRINTF(" state:%d vol ++", state);
      // OLED_print_volume(1);
      OLED_print_volume_value(volume_value & 0xFF);
      OLED_Horizontal_Scroll(3, 3, 7, 0x27, 1);
      break;

    case 'A':
      OLED__DEBUG_PRINTF(" state:%d bass ++", state);
      // OLED_print_bass(1);
      DAC_bass_control(1);
      OLED_print_bass_value((bass_treble_value >> 4) & 0xF);
      OLED_Horizontal_Scroll(3, 3, 7, 0x27, 1);
      break;

    case '4':
      OLED_print_song_info();
      state = Info_State;
      break;

    case '5':
      OLED_play_pause();
      break;

    case '6':
      break;

    case 'B':
      OLED__DEBUG_PRINTF(" state:%d bass --", state);
      // OLED_print_bass(0);
      DAC_bass_control(0);
      OLED_print_bass_value((bass_treble_value >> 4) & 0xF);
      OLED_Horizontal_Scroll(3, 3, 7, 0x27, 1);
      break;

    case '7':
      break;

    case '8':
      break;

    case '9':
      skip_the_song();
      print_song_list_page(); // state = Idle_State;
      // song_index = 0;
      // arrow_location = 2;
      // state = Idle_State;
      // OLED_Horizontal_Scroll(3, 3, 7, 0x27, 0);
      break;

    case 'C':
      OLED__DEBUG_PRINTF(" state:%d treble ++", state);
      // OLED_print_treble(1);
      DAC_treble_control(1);
      OLED_Horizontal_Scroll(3, 3, 7, 0x27, 1);
      break;

    case '*':
      break;

    case '0':
      // for (int i = 0; i < 3; i++)
      //   fprintf(stderr, "mp3 tag: %c\n", mp3_tag.tagMark[i]);

      // for (int i = 0; i < 30; i++)
      //   fprintf(stderr, "mp3 name: %c\n", mp3_tag.trackName[i]);

      // for (int i = 0; i < 30; i++)
      //   fprintf(stderr, "mp3 name: %c\n", mp3_tag.artistName[i]);

      // for (int i = 0; i < 30; i++)
      //   fprintf(stderr, "mp3 name: %c\n", mp3_tag.albumName[i]);

      // fprintf(stderr, "\nmp3 tag: %s\n", mp3_tag.tagMark);
      fprintf(stderr, "mp3 name: %s\n", mp3_tag.trackName);
      fprintf(stderr, "artist Name: %s\n", mp3_tag.artistName);
      fprintf(stderr, "album Name: %s\n", mp3_tag.albumName);
      fprintf(stderr, "year: %s\n", mp3_tag.year);

      break;

    case '#':
      OLED__DEBUG_PRINTF(" state:%d back to main", state);
      print_song_list_page(); // state = Idle_State;
      // song_index = 0;
      // arrow_location = 2;
      // OLED_Horizontal_Scroll(3, 3, 7, 0x27, 0);
      break;

    case 'D':
      OLED__DEBUG_PRINTF(" state:%d treble --", state);
      // OLED_print_treble(0);
      DAC_treble_control(0);
      OLED_Horizontal_Scroll(3, 3, 7, 0x27, 1);
      break;

    default:
      // print_now_playing_page(1);
      break;
    }
    break;

  case Info_State:
    switch (key) {
    case '1': // vol--
      OLED__DEBUG_PRINTF(" state:%d vol --", state);
      OLED_print_volume(0);
      break;

    case '3': // vol++
      OLED__DEBUG_PRINTF(" state:%d vol ++", state);
      OLED_print_volume(1);
      break;

    case 'A':
      OLED__DEBUG_PRINTF(" state:%d bass ++", state);
      // OLED_print_bass(1);
      break;

    case '4':
      break;

    case '5':
      break;

    case '6':
      print_now_playing_page(1);
      print_play_pause_icon(1);
      OLED_Horizontal_Scroll(3, 3, 7, 0x27, 1);

      state = Play_State;
      break;

    case 'B':
      OLED__DEBUG_PRINTF(" state:%d bass --", state);
      // OLED_print_bass(0);
      break;

    case '7':
      break;

    case '8':
      break;

    case '9':
      break;

    case 'C':
      OLED__DEBUG_PRINTF(" state:%d treble ++", state);
      // OLED_print_treble(1);
      break;

    case '*':
      break;

    case '0':
      break;

    case '#':
      break;

    case 'D':
      OLED__DEBUG_PRINTF(" state:%d treble --", state);
      // OLED_print_treble(0);
      break;

    default:
      // print_now_playing_page(1);
      break;
    }
    break;

  default:
    break;
  }
}

void string_char(char *string[128]) {
  // const char *s = "aaaaaa<BBBB>TEXT TO EXTRACT</BBBB>aaaaaaaaa";

  const char *PATTERN1 = "TIT2";
  const char *PATTERN2 = "TPE1";

  char *target = NULL;
  char *start, *end;

  if (start = strstr(string, PATTERN1)) {
    start += strlen(PATTERN1);
    if (end = strstr(start, PATTERN2)) {
      target = (char *)malloc(end - start + 1);
      memcpy(target, start, end - start);
      // target[end - start] = '\0';
    }
  }

  if (target)
    for (int i = 0; i < 25; i++)
      printf("%c", target[i]);

  free(target);
}