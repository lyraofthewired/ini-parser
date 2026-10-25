#include <ctype.h>

#include "iniparse.h"

enum {
	/* Beginning of a line */
	S_BEGIN,
	/* Within a section */
	S_SECTION,
	/* Expecting the line to end */
	S_ENDLINE,
	/* Within a comment */
	S_COMMENT,
	/* Within a field name */
	S_FIELD,
	/* Expecting an equal sign */
	S_EQUALS,
	/* Expecting data to begin */
	S_DATA_BEGIN,
	/* Within data */
	S_DATA,
	/* Within data and whitespace */
	S_DATA_WS,
	/* Within data, but also within quotes */
	S_DATA_QUOT,
};

static int feed(ini_parser* p, const ini_callbacks* cb, char c);
static int isfield(int c);
static int iscombeg(int c);

int ini_parser_feed(ini_parser* parser, const ini_callbacks* cb, size_t count, const void* bytes) {
	const char* cdata = NULL;

	for (cdata = bytes; count--; ++cdata)
		if (feed(parser, cb, *cdata))
			return -1;

	return 0;
}

int ini_parser_finish(ini_parser* parser, const ini_callbacks* cb) {
	switch (parser->state) {
	case S_BEGIN: case S_ENDLINE: case S_COMMENT:
		return 0;
	case S_DATA_BEGIN: case S_DATA: case S_DATA_WS:
		if (parser->cur_off >= sizeof(parser->buffer))
			return -1;

		parser->buffer[parser->cur_off++] = '\0';
		if (!cb->on_pair || cb->on_pair(cb->user, parser->buffer, parser->buffer + parser->data_off))
			return -1;

		return 0;
	default:
		return -1;
	}

	return 0;
}

static int feed(ini_parser* p, const ini_callbacks* cb, char c) {
	switch (p->state) {
	case S_BEGIN:
		if (c == '[') {
			p->state = S_SECTION;
			p->cur_off = 0;
			return 0;
		} else if (isfield(c)) {
			p->state = S_FIELD;
			p->cur_off = 0;
			return feed(p, cb, c);
		} else if (iscombeg(c)) {
			p->state = S_COMMENT;
			return 0;
		} else if (c == '\n') {
			return 0;
		}

		return -1;
	case S_SECTION:
		if (c == ']') {
			if (p->cur_off >= sizeof(p->buffer))
				return -1;
			
			p->buffer[p->cur_off] = '\0';
			if (!cb->on_section || cb->on_section(cb->user, p->buffer))
				return -1;

			p->state = S_ENDLINE;
			return 0;
		} else if (isfield(c)) {
			if (p->cur_off >= sizeof(p->buffer))
				return -1;

			p->buffer[p->cur_off++] = c;
			return 0;
		}

		return -1;
	case S_ENDLINE:
		if (c == '\n') {
			p->state = S_BEGIN;
			return 0;
		} else if (iscombeg(c)) {
			p->state = S_COMMENT;
			return 0;
		} else if (isspace(c)) {
			return 0;
		}

		return -1;
	case S_COMMENT:
		if (c == '\n')
			p->state = S_BEGIN;

		return 0;
	case S_FIELD:
		if (isfield(c)) {
			if (p->cur_off >= sizeof(p->buffer))
				return -1;

			p->buffer[p->cur_off++] = c;
			return 0;
		} else if (c == '\n') {
			return -1;
		} else if (c == ' ' || c == '=') {
			if (p->cur_off >= sizeof(p->buffer))
				return -1;

			p->buffer[p->cur_off++] = '\0';
			p->data_off = p->cur_off;

			p->state = S_EQUALS;
			return feed(p, cb, c);
		}

		return -1;
	case S_EQUALS:
		if (c == '\n')
			return -1;
		else if (isspace(c))
			return 0;
		else if (c != '=')
			return -1;

		p->state = S_DATA_BEGIN;
		return 0;
	case S_DATA_BEGIN:
		if (c == '\n' || iscombeg(c)) {
			if (p->cur_off >= sizeof(p->buffer))
				return -1;

			p->buffer[p->cur_off++] = '\0';

			if (!cb->on_pair || cb->on_pair(cb->user, p->buffer, p->buffer + p->data_off))
				return -1;

			p->state = S_COMMENT;
			return feed(p, cb, c);
		} else if (isspace(c)) {
			return 0;
		}

		p->state = S_DATA;
		return feed(p, cb, c);
	case S_DATA:
		if (c == '\n' || iscombeg(c)) {
			if (p->cur_off >= sizeof(p->buffer))
				return -1;

			p->buffer[p->cur_off++] = '\0';

			if (!cb->on_pair || cb->on_pair(cb->user, p->buffer, p->buffer + p->data_off))
				return -1;

			p->state = S_COMMENT;
			return feed(p, cb, c);
		} else if (isspace(c)) {
			p->state = S_DATA_WS;
			return 0;
		}
		
		if (p->cur_off >= sizeof(p->buffer))
				return -1;

		p->buffer[p->cur_off++] = c;
		if (c == '"')
			p->state = S_DATA_QUOT;

		return 0;
	case S_DATA_WS:
		if (c == '\n' || iscombeg(c)) {
			p->state = S_DATA;
			return feed(p, cb, c);
		} else if (isspace(c)) {
			return 0;
		}

		if (p->cur_off >= sizeof(p->buffer))
			return -1;

		p->buffer[p->cur_off++] = ' ';

		p->state = S_DATA;
		return feed(p, cb, c);
	case S_DATA_QUOT:
		if (c == '\n')
			return -1;

		if (p->cur_off >= sizeof(p->buffer))
			return -1;

		p->buffer[p->cur_off++] = c;		
		if (c == '"')
			p->state = S_DATA;

		return 0;
	default:
		return -1;
	}
}

static int isfield(int c) {
	return isalnum(c) || c == '_' || c == '.';
}

static int iscombeg(int c) {
	return c == '#' || c == ';';
}

