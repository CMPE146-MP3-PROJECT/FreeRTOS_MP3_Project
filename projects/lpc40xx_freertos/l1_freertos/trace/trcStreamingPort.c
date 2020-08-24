/*******************************************************************************
 * Trace Recorder Library for Tracealyzer v4.3.7
 * Percepio AB, www.percepio.com
 *
 * trcStreamingPort.c
 *
 * Supporting functions for trace streaming, used by the "stream ports"
 * for reading and writing data to the interface.
 * Existing ports can easily be modified to fit another setup, e.g., a
 * different TCP/IP stack, or to define your own stream port.
 *
 * Terms of Use
 * This file is part of the trace recorder library (RECORDER), which is the
 * intellectual property of Percepio AB (PERCEPIO) and provided under a
 * license as follows.
 * The RECORDER may be used free of charge for the purpose of recording data
 * intended for analysis in PERCEPIO products. It may not be used or modified
 * for other purposes without explicit permission from PERCEPIO.
 * You may distribute the RECORDER in its original source code form, assuming
 * this text (terms of use, disclaimer, copyright notice) is unchanged. You are
 * allowed to distribute the RECORDER with minor modifications intended for
 * configuration or porting of the RECORDER, e.g., to allow using it on a
 * specific processor, processor family or with a specific communication
 * interface. Any such modifications should be documented directly below
 * this comment block.
 *
 * Disclaimer
 * The RECORDER is being delivered to you AS IS and PERCEPIO makes no warranty
 * as to its use or performance. PERCEPIO does not and cannot warrant the
 * performance or results you may obtain by using the RECORDER or documentation.
 * PERCEPIO make no warranties, express or implied, as to noninfringement of
 * third party rights, merchantability, or fitness for any particular purpose.
 * In no event will PERCEPIO, its technology partners, or distributors be liable
 * to you for any consequential, incidental or special damages, including any
 * lost profits or lost savings, even if a representative of PERCEPIO has been
 * advised of the possibility of such damages, or for any claim by any third
 * party. Some jurisdictions do not allow the exclusion or limitation of
 * incidental, consequential or special damages, or the exclusion of implied
 * warranties or limitations on how long an implied warranty may last, so the
 * above limitations may not apply to you.
 *
 * Tabs are used for indent in this file (1 tab = 4 spaces)
 *
 * Copyright Percepio AB, 2018.
 * www.percepio.com
 ******************************************************************************/

#include <stdio.h>
#include <string.h>

#include "ff.h"

#include "trcRecorder.h"

/**
 * Somewhat experimental f_write() functionality that writes aligned 4K buffer
 * to minimize the SD card write latency.
 *   - Set to 0 to disable it
 *   - Set to size of aligned memory write for the SD card
 */
#define ALIGN_FWRITE (4 * 1024)

#if (TRC_CFG_RECORDER_MODE == TRC_RECORDER_MODE_STREAMING)
#if (TRC_USE_TRACEALYZER_RECORDER == 1)

static size_t page_number;
static FIL trace_file;

// Open is called when our SPI is not even initialized so we do not open a file here
void openFile(char *file_name) {
  if (FR_OK == f_open(&trace_file, file_name, (FA_WRITE | FA_CREATE_ALWAYS))) {
#if 0
    const FSIZE_t preallocate_size = (64 * 1024 * 1024);
    const BYTE allocate_now = 1;
    f_expand(&trace_file, preallocate_size, allocate_now);
#endif
    f_sync(&trace_file);
    printf("  --> RTOS trace: Opened %s on your SD card\n", file_name);
  } else {
    printf("  --> RTOS trace: Failed to open %s on your SD card\n", file_name);
  }
}

int32_t writeToFile(void *data, uint32_t data_size_in_bytes, int32_t *num_of_bytes_written) {
  FRESULT result = FR_INVALID_PARAMETER;
  UINT bytes_written = 0;

#if ALIGN_FWRITE
  static uint8_t file_buffer[ALIGN_FWRITE];
  static size_t buffered_byte_count = 0;

  const size_t available = sizeof(file_buffer) - buffered_byte_count;                               // 4096 - 4090 = 6
  const size_t bytes_to_buffer = (data_size_in_bytes < available) ? data_size_in_bytes : available; // 6
  const size_t unbuffered_byte_count = (data_size_in_bytes - bytes_to_buffer);                      // 4090 - 6

  memcpy((file_buffer + buffered_byte_count), data, bytes_to_buffer); // copy 6
  buffered_byte_count += bytes_to_buffer;                             // 4096

  if (buffered_byte_count >= sizeof(file_buffer)) {
    if (FR_OK == (result = f_write(&trace_file, file_buffer, buffered_byte_count, &bytes_written))) {
      f_sync(&trace_file);
    } else {
      fprintf(stderr, "RTOS trace: Failed to write page %d, error %d\n", page_number, result);
    }

    ++page_number;
    buffered_byte_count = 0;
  }

  if (unbuffered_byte_count > 0) {
    memcpy(file_buffer, (data + bytes_to_buffer), unbuffered_byte_count);
    buffered_byte_count += unbuffered_byte_count;
  }
#else
  if (FR_OK == (result = f_write(&trace_file, data, data_size_in_bytes, &bytes_written))) {
    f_sync(&trace_file);
  } else {
    fprintf(stderr, "RTOS trace: Failed to write page %d, error %d\n", page_number, result);
  }
#endif

  *num_of_bytes_written = data_size_in_bytes;
  return 0;
}

void closeFile(void) {
  printf("RTOS trace: Closed trace file\n");
  f_close(&trace_file);
}

#endif /*(TRC_USE_TRACEALYZER_RECORDER == 1)*/
#endif /*(TRC_CFG_RECORDER_MODE == TRC_RECORDER_MODE_STREAMING)*/
