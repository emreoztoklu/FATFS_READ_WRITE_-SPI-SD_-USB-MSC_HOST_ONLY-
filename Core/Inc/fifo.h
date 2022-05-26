#ifndef _FIFO_H
#define _FIFO_H

#include <stdint.h>
#include <stdbool.h>

typedef uint8_t FIFO_VAR;

typedef struct {
  FIFO_VAR      *Buffer;        // FIFO buffer
  FIFO_VAR      *pHead;         // Kuyruk baþý
  FIFO_VAR      *PTail;         // Kuyruk sonu
  unsigned      Length;        // FIFO buffer uzunluðu
} FIFO;

void FIFO_Init(FIFO *pFifo, FIFO_VAR *Buffer, unsigned Length);
_Bool FIFO_SetData(FIFO *pFifo, FIFO_VAR data);
FIFO_VAR FIFO_GetData(FIFO *pFifo);
int FIFO_IsEmpty(FIFO *pFifo);

#endif