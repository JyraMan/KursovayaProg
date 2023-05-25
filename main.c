
/*
 * Definition for LZW coding
 *
 * vim: ts=4 sw=4 cindent nowrap
 */
#include <stdlib.h>
#include <stdio.h>
#include "bitio.h"
#define MAX_CODE 65535

struct {
	int suffix;// суффикс относится к P, родитель относится к C
	int parent, firstchild, nextsibling; // Последние две переменные используются для выходных символов один за другим
} dictionary[MAX_CODE + 1];
int next_code;// указывает на самое большое кодовое слово в словаре Следующего кода, например, самый большой код в словаре - 255, точка следующего_кода до 256
int d_stack[MAX_CODE]; // кодовое слово соответствует строке, которая в основном используется для вывода символов в строке в порядке, которая расположена в перевернутом порядке

#define input(f) ((int)BitsInput( f, 16)) 
#define output(f, x) BitsOutput( f, (unsigned long)(x), 16) 

int DecodeString(int start, int code);
void InitDictionary(void);

// Печать из 256, кодовое наблюдение, полученное алгоритмом LZW.
void PrintDictionary(void) {
	int n;
	int count;
	for (n = 256; n < next_code; n++) {
		count = DecodeString(0, n);
		printf("%4d->", n);
		while (0 < count--) printf("%c", (char)(d_stack[count]));
		printf("\n");
	}
}

// Поместите символы в символы, соответствующие строке кода, один за другим в массиве D_STACK, что удобно для вывода строки
// Когда начало = 0, код = 258,
// Предположим, что кодовое слово 258 соответствует ABA, P = AB, C = A и сначала хранить суффикс, то есть A a in d_stack [0], затем p = a, c = b и сохранение b в D_Stack [1], наконец, сохранить A в D_STACK [2], чтобы облегчить вывод ABA
// Длина строки, соответствующая этому кодовому слову, возвращает
int DecodeString(int start, int code) {
	int count;
	count = start;
	while (0 <= code) {
		d_stack[count] = dictionary[code].suffix; // суффикс записан в d_stack
		code = dictionary[code].parent; // код для поиска префиксов в порядке
		count++; // считать
	}
	return count;
}

// Первоначальная таблица химического кода - код ASCII.
void InitDictionary(void) {
	int i;

	for (i = 0; i < 256; i++) {
		dictionary[i].suffix = i;
		dictionary[i].parent = -1;
		dictionary[i].firstchild = -1;
		dictionary[i].nextsibling = i + 1;
	}
	dictionary[255].nextsibling = -1;
	next_code = 256;
}
/*
 * Input: string represented by string_code in dictionary,
 * Output: the index of character+string in the dictionary
 * 		index = -1 if not found
 */
int InDictionary(int character, int string_code) {
	int sibling;
	if (0 > string_code) return character;
	sibling = dictionary[string_code].firstchild;
	while (-1 < sibling) {
		if (character == dictionary[sibling].suffix) return sibling;
		sibling = dictionary[sibling].nextsibling;
	}
	return -1;
}

void AddToDictionary(int character, int string_code) {
	int firstsibling, nextsibling;
	if (0 > string_code) return;
	dictionary[next_code].suffix = character;
	dictionary[next_code].parent = string_code;
	dictionary[next_code].nextsibling = -1;
	dictionary[next_code].firstchild = -1;
	firstsibling = dictionary[string_code].firstchild;
	if (-1 < firstsibling) {	// the parent has child
		nextsibling = firstsibling;
		while (-1 < dictionary[nextsibling].nextsibling)
			nextsibling = dictionary[nextsibling].nextsibling;
		dictionary[nextsibling].nextsibling = next_code;
	}
	else {// no child before, modify it to be the first
		dictionary[string_code].firstchild = next_code;
	}
	next_code++;
}

void LZWEncode(FILE* fp, BITFILE* bf) {
	int character;
	int string_code;
	int index;
	unsigned long file_length;

	fseek(fp, 0, SEEK_END);
	file_length = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	BitsOutput(bf, file_length, 4 * 8);
	InitDictionary();// Словарь инициализации
	string_code = -1;// P
	while (EOF != (character = fgetc(fp))) {
		index = InDictionary(character, string_code); // Найти кодирование P+C в словаре. Если вы найдете его, верните код в индекс, иначе верните -1 в индекс
		if (0 <= index) {	// string_code относится к P, символ относится к C, P+C в словаре 
			string_code = index; // P <- P+C
		}
		else {	// p+c нет в словаре
			output(bf, string_code); // Выход P с потоком символов P
			if (MAX_CODE > next_code) {	// free space in dictionary
				// add string+character to dictionary
				AddToDictionary(character, string_code); // Написать P+C в словарь
			}
			string_code = character; // P <- C
		}
	}
	output(bf, string_code);
}


void LZWDecode(BITFILE* bf, FILE* fp) {
	int character;
	int new_code, last_code;
	int phrase_length;
	unsigned long file_length;

	file_length = BitsInput(bf, 4 * 8);
	if (-1 == file_length) file_length = 0;
	/*Нужно заполнить*/
	InitDictionary(); // Словарь инициализации
	last_code = -1; // last_code относится к PW
	while (0 < file_length) {
		new_code = input(bf); // вводится двоичные файлы, New_Code относится к CW
		if (new_code >= next_code) { // Если CW нет в словаре, напишите первый символ PW в C, PW до P и вывод P+C
			d_stack[0] = character; // Первые хранилища символа PW в выводимой строке D_STACK
			phrase_length = DecodeString(1, last_code); // Запуск PW с позиции 1 и хранить его в D_STACK, потому что место 0 записано в первый символ PW
		}
		else { // Если CW находится в словаре, храните CW непосредственно в вывод D_STACK
			phrase_length = DecodeString(0, new_code);
		}
		character = d_stack[phrase_length - 1]; // Первый символ текущего CW хранится по характеру, потому что D_STACK перевернулся, как первый персонаж следующего цикла PW
		while (0 < phrase_length) {
			phrase_length--;
			fputc(d_stack[phrase_length], fp);// вывод строки, соответствующей текущему кодовому слову
			file_length--;
		}
		if (MAX_CODE > next_code) {// p+c Напишите словарь
			AddToDictionary(character, last_code);
		}
		last_code = new_code;// pW <- cW
	}
}



int main(int argc, char** argv) {
	FILE* fp;
	BITFILE* bf;

	if (4 > argc) {
		fprintf(stdout, "usage: \n%s <o> <ifile> <ofile>\n", argv[0]);
		fprintf(stdout, "\t<o>: E or D reffers encode or decode\n");
		fprintf(stdout, "\t<ifile>: input file name\n");
		fprintf(stdout, "\t<ofile>: output file name\n");
		return -1;
	}
	if ('E' == argv[1][0]) { // do encoding
		fp = fopen(argv[2], "rb");
		bf = OpenBitFileOutput(argv[3]);
		if (NULL != fp && NULL != bf) {
			LZWEncode(fp, bf);
			fclose(fp);
			CloseBitFileOutput(bf);
			fprintf(stdout, "encoding done\n");
		}
	}
	else if ('D' == argv[1][0]) {	// do decoding
		bf = OpenBitFileInput(argv[2]);
		fp = fopen(argv[3], "wb");
		if (NULL != fp && NULL != bf) {
			LZWDecode(bf, fp);
			fclose(fp);
			CloseBitFileInput(bf);
			fprintf(stdout, "decoding done\n");
		}
	}
	else {	// otherwise
		fprintf(stderr, "not supported operation\n");
	}
	PrintDictionary();
	return 0;
}
