#include <fifo.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>


void FIFO_Init(FIFO *pFifo, FIFO_VAR *Buffer, unsigned Length)
{
  pFifo->Buffer = Buffer;
  pFifo->Length = Length;
  pFifo->pHead = Buffer;
  pFifo->PTail = Buffer;
}

// Birinci parametrede belirtilen kuyru�un sonuna
// ikinci parametredeki de�eri ekler
// Geri d�n�� true ise ba�ar�l�
// false ise ba�ar�s�z (buffer full)
_Bool FIFO_SetData(FIFO *pFifo, FIFO_VAR data)
{
  // ptr kopya g�sterici (pTail i�in)
  FIFO_VAR *ptr = pFifo->PTail;
  
  // ptr ring buffer kural�na uygun art�r�l�yor
  if (++ptr == pFifo->Buffer + pFifo->Length)
    ptr = pFifo->Buffer;
  
  // E�er pTail art�r�ld�ktan sonra pHead'e e�it oluyorsa
  // buffer full demektir, ekleme yapamay�z
  if (ptr == pFifo->pHead) {
    // Beep();
    return false;
  }
  
  *pFifo->PTail = data;
  pFifo->PTail = ptr;
  
  return true;
}

// FIFO kuyruk bo�sa true d�ner
int FIFO_IsEmpty(FIFO *pFifo)
{
  return (pFifo->pHead == pFifo->PTail);
}

// FIFO buffer'dan veri �eker (kuyru�un ba��ndan)
// E�er buffer bo�sa bloke bekler
FIFO_VAR FIFO_GetData(FIFO *pFifo)
{
  FIFO_VAR data;
  
  // Kuyruk bo� oldu�u m�ddet�e bekle
  while (FIFO_IsEmpty(pFifo)) ;
  
  data = *pFifo->pHead;
  
  // dairesel buffer kural�na uygun olarak
  // pHead g�stericisini art�raca��z
  if (++pFifo->pHead == pFifo->Buffer + pFifo->Length)
    pFifo->pHead = pFifo->Buffer;
  
  return data;
}