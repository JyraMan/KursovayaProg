/*
 * Declaration for bitwise IO
 *
 * vim: ts=4 sw=4 cindent
 */
#ifndef __BITIO__
#define __BITIO__

#include <stdio.h>
#include "testin"
#include "testout"

typedef struct {
	FILE* fp;
	unsigned char mask;
	int rack;
}Bitfile;


Bitfile* OpenBitFileInput(char* testin);
Bitfile* OpenBitFileOutput(char* testout);
void CloseBitFileInput(Bitfile* bf);
void CloseBitFileOutput(Bitfile* bf);
int BitInput(Bitfile* bf);
unsigned long BitsInput(Bitfile* bf, int count);
void BitOutput(Bitfile* bf, int bit);
void BitsOutput(Bitfile* bf, unsigned long code, int count);
#endif	// __BITIO__
