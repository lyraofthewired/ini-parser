#ifndef INIPARSE_H_
#define INIPARSE_H_

#include <stddef.h>

typedef struct ini_parser_s {
	unsigned char state;
	unsigned char cur_off;
	unsigned char data_off;
	char buffer[253];
} ini_parser;

typedef struct ini_callbacks_s {
	void* user;
	int (*on_section)(void* user, const char* section);
	int (*on_pair)(void* user, const char* key, const char* value);
} ini_callbacks;

int ini_parser_feed(ini_parser* parser, const ini_callbacks* cb, size_t count, const void* bytes);
int ini_parser_finish(ini_parser* parser, const ini_callbacks* cb);

#endif

