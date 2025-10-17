/*
 * https://github.com/LordOfTrident/cargs
 *
 */

#ifndef CARGS_HEADER
#define CARGS_HEADER

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>

#endif

#ifdef __cplusplus
extern "C" {
#endif

char *CARGS_FORMAT_COMMAND = " -%s --%s [%s]\n";
char *CARGS_FORMAT_PARAM1 = "   --%s (%s)\n";
char *CARGS_FORMAT_PARAM2 = "   -%s --%s (%s)\n";
char *CARGS_LANGUAGE_DEFAULT = "default '%s'";
char *CARGS_LANGUAGE_REQUIRE = "force required";

#define CARGS_REQUIRE_INTEGER -9999999
#define CARGS_REQUIRE_NUMBER -9999999.9f
#define CARGS_REQUIRE_STRING NULL

enum {
	CARGS_OK = 0,
	CARGS_MISSING_COMMAND,
	CARGS_UNKNOWN_COMMAND,
	CARGS_MISSING_PARAM,
	CARGS_UNKNOWN_PARAM,
	CARGS_MISSING_VALUE,
	CARGS_INVALID_VALUE,
	CARGS_UNKNOWN_ERROR,
} cargs_code;

typedef enum {
	CARGS_BOOLEAN = 0,
	CARGS_INTEGER,
	CARGS_NUMBER,
	CARGS_STRING,
} cArgsType;

typedef struct _cargsParam {
	cArgsType type;
	const char *letter;
	const char *name;
	union {
		bool boolean;
		int integer;
		float number;
		char *string;
	};
	void *ptr;
	bool found;
	struct _cargsParam *next;
} cArgsParam;

typedef struct {
	const char *letter;
	const char *name;
	const char *describe;
	bool found;
	cArgsParam *params;
} cArgsCommand;

typedef struct {
	int          c;
	const char **v;
	char       **base;
	const char *path;
	char *extra;
} cArgs;

#define _CARGS_COMMAND_CAPACITY 128

cArgsCommand _cargs_list[_CARGS_COMMAND_CAPACITY];
size_t _cargs_count = 0;

bool _cargs_is_letter_tag(const char *arg) {
	if(strlen(arg) == 2 && arg[0] == '-' && arg[1] != '-') {
		return true;
	} else if (strlen(arg) >= 2 && arg[0] == '-' && arg[2] == '=') {
		return true;
	}
	return false;
}

bool _cargs_is_name_tag(const char *arg) {
	return strlen(arg) > 2 && arg[0] == '-' && arg[1] == '-';
}

bool _cargs_is_end_tag(const char *arg) {
	return strcmp(arg, "--") == 0;
}

bool _cargs_is_non_val(cArgsParam *param) {
	cArgsType t = param->type;
	void *p = param->ptr;
	if (t == CARGS_BOOLEAN) {
		return false;
	} else if (t == CARGS_INTEGER && *(int *)p == CARGS_REQUIRE_INTEGER) {
		return true;
	} else if (t == CARGS_NUMBER && *(float *)p == CARGS_REQUIRE_NUMBER) {
		return true;
	} else if (t == CARGS_STRING && *(char **)p == CARGS_REQUIRE_STRING) {
		return true;
	}
	return false;
}

const char *_cargs_get_tag_value(const char *tag) {
	bool isLetter = _cargs_is_letter_tag(tag);
	bool isName = _cargs_is_name_tag(tag);
	const char *_tag = tag + isName * 2 + isLetter * 1;
	return _tag;
}

cArgs cargs_new(int argc, const char **argv) {
	cArgs body;
	body.c = argc;
	body.v = argv;
	body.base = (char**)argv;
	body.path = argv[0];
	body.extra = NULL;
	return body;
}

bool *cargs_command(const char *letter, const char *name, const char *describe) {
	assert(letter != NULL);
	assert(name != NULL);
	assert(describe != NULL);
	assert(_cargs_count < _CARGS_COMMAND_CAPACITY);
	cArgsCommand *cmd = &_cargs_list[_cargs_count];
	cmd->letter = letter;
	cmd->name = name;
	cmd->describe = describe;
	cmd->found = false;
	cmd->params = NULL;
	_cargs_count++;
	return &cmd->found;
}

cArgsParam *_cargs_add_param(cArgsType type, const char *letter, const char *name, void *value) {
	assert(name != NULL);
	assert(value != NULL);
	assert(_cargs_count > 0);
	cArgsCommand *cmd = &_cargs_list[_cargs_count - 1];
	cArgsParam *param = malloc(sizeof(cArgsParam));
	param->type = type;
	param->letter = letter;
	param->name = name;
	param->boolean = 0;
	param->ptr = value;
	param->found = false;
	param->next = NULL;
	//
	if (type == CARGS_BOOLEAN) {
		param->boolean = *(bool *)value;
	} else if (type == CARGS_INTEGER) {
		param->integer = *(int *)value;
	} else if (type == CARGS_NUMBER) {
		param->integer = *(float *)value;
	} else if (type == CARGS_STRING) {
		param->string = *(char **)value;
	}
	//
	if (cmd->params == NULL) {
		cmd->params = param;
	} else {
		cArgsParam *_param = cmd->params;
		while (_param->next != NULL) _param = _param->next;
		_param->next = param;
	}
	//
	return param;
}

void cargs_boolean(const char *letter, const char *name, bool *value) {
	_cargs_add_param(CARGS_BOOLEAN, letter, name, value);
}

void cargs_integer(const char *letter, const char *name, int *value) {
	_cargs_add_param(CARGS_INTEGER, letter, name, value);
}

void cargs_number(const char *letter, const char *name, float *value) {
	_cargs_add_param(CARGS_NUMBER, letter, name, value);
}

void cargs_string(const char *letter, const char *name, char **value) {
	_cargs_add_param(CARGS_STRING, letter, name, value);
}

static cArgsCommand *_cargs_find_command(const char *text) {
	if (text == NULL) return NULL;
	for (size_t i = 0; i < _cargs_count; ++ i) {
		cArgsCommand *cmd = &_cargs_list[i];
		if (strlen(text) == 1) {
			if (cmd->letter != NULL && strcmp(cmd->letter, text) == 0) return cmd;
		} else {
			if (cmd->name != NULL && strcmp(cmd->name, text) == 0) return cmd;
		}
	}
	return NULL;
}

cArgsParam *_cargs_find_param(cArgsCommand *command, const char *text) {
	if (text == NULL) return NULL;
	cArgsParam *param = command->params;
	while (param != NULL) {
		if (strlen(text) == 1) {
			if (param->letter != NULL && param->letter && strcmp(param->letter, text) == 0) return param;
		} else {
			if (param->name != NULL && param->name && strcmp(param->name, text) == 0) return param;
		}
		param = param->next;
	}
	return NULL;
}

int _cargs_check_require(cArgs *body, cArgsCommand *command) {
	cArgsParam *param = command->params;
	while (param != NULL) {
		if (!param->found && _cargs_is_non_val(param)) {
			body->extra = param->name;
			return CARGS_MISSING_PARAM;
		}
		param = param->next;
	}
	return CARGS_OK;
}

static int _cargs_parse(cArgsParam *prm, const char *txt) {
	void *p = prm->ptr;
	switch (prm->type) {

	case CARGS_BOOLEAN: {
		if (strcmp(txt, "true") == 0 || strcmp(txt, "1") == 0)
			*(bool *)p = true;
		else if (strcmp(txt, "false") == 0 || strcmp(txt, "0") == 0)
			*(bool *)p = false;
		else
			return CARGS_INVALID_VALUE;
	} break;

	case CARGS_INTEGER: {
		char *ptr;
		int v = (int)strtol(txt, &ptr, 10);
		if (*ptr != '\0')
			return CARGS_MISSING_VALUE;

		*(int *)p = v;
	} break;

	case CARGS_NUMBER: {
		char *ptr;
		float v = (float)strtod(txt, &ptr);
		if (*ptr != '\0')
			return CARGS_MISSING_VALUE;

		*(float *)p = v;
	} break;

	case CARGS_STRING: {
		*(char **)p = (char*)malloc(strlen(txt) + 1);
		strcpy(*(char **)p, txt);
	} break;

	}
	prm->found = true;
	return CARGS_OK;
}

int cargs_run(cArgs *body) {
	//
	if (body->c < 2) return CARGS_MISSING_COMMAND;
	const char *path = body->v[0];
	const char *slct = body->v[1];
	if (!_cargs_is_letter_tag(slct)) return CARGS_MISSING_COMMAND;
	//
	const char *_slct = _cargs_get_tag_value(slct);
	body->extra = _slct;
	cArgsCommand *command = _cargs_find_command(_slct);
	if (!command) return CARGS_UNKNOWN_COMMAND;
	//
	command->found = true;
	cArgsParam *params = command->params;
	if (params == NULL) {
		return _cargs_check_require(body, command);
	}
	//
	for (int i = 2; i < body->c; ++ i) {
		const char *_arg = body->v[i];
		if (_cargs_is_end_tag(_arg)) {
			return CARGS_OK;
		} else if (!_cargs_is_letter_tag(_arg) && !_cargs_is_name_tag(_arg)) {
			continue;
		}

		bool isLetter = _cargs_is_letter_tag(_arg);
		bool isName = _cargs_is_name_tag(_arg);
		const char *arg = _cargs_get_tag_value(_arg);

		const char *tmp   = arg;
		size_t count = 0;
		while (*arg != '\0' && *arg != '=') {
			++ count;
			++ arg;
		}

		char *name = (char*)malloc(count + 1);
		if (name == NULL) return CARGS_UNKNOWN_ERROR;
		strncpy(name, tmp, count);
		name[count] = '\0';

		cArgsParam *param = _cargs_find_param(command, name);
		body->extra = param;
		if (param == NULL) return CARGS_UNKNOWN_PARAM;
		void *ptr = param->ptr;

		char *text = NULL;
		if (*arg == '=') {
			++ arg;
			tmp   = arg;
			count = 0;
			while (*arg != '\0') {
				++ count;
				++ arg;
			}
			text = (char*)malloc(count + 1);
			if (text == NULL) return CARGS_UNKNOWN_ERROR;
			strncpy(text, tmp, count);
			text[count] = '\0';
		}

		if (text != NULL) {
			int err = _cargs_parse(param, text);
			if (err != CARGS_OK) return err;
			free(text);
		} else if (param->type == CARGS_BOOLEAN) {
			*(bool *)ptr = true;
			int err = _cargs_parse(param, "true");
			if (err != CARGS_OK) return err;
		} else {
			++ i;
			if (i >= body->c) {
				return CARGS_MISSING_VALUE;
			}
			text = (char*)body->v[i];
			int err = _cargs_parse(param, text);
			if (err != CARGS_OK) return err;
		}

		free(name);
	}
	//
	return _cargs_check_require(body, command);
}

void cargs_print(FILE *file) {
	if (!file) file = stdout;
	for (size_t i = 0; i < _cargs_count; ++ i) {
		cArgsCommand *command = &_cargs_list[i];
		char *describe = command->describe != NULL ? command->describe : "?";
		fprintf(file, CARGS_FORMAT_COMMAND, command->letter, command->name, describe);
		cArgsParam *param = command->params;
		while (param != NULL)
		{
			char _describe[256];
			cArgsType type = param->type;
			void *ptr = param->ptr;
			if (_cargs_is_non_val(param)) {
				sprintf(_describe, "%s", CARGS_LANGUAGE_REQUIRE);
			} else {
				char _formatted[256];
				switch (type) {
					case CARGS_BOOLEAN: sprintf(_formatted, *(bool *)ptr ? "true" : "false"); break;
					case CARGS_INTEGER: sprintf(_formatted, "%i", *(int *)ptr); break;
					case CARGS_NUMBER:  sprintf(_formatted, "%f", *(float *)ptr); break;
					case CARGS_STRING:  sprintf(_formatted, "%s", *(char **)ptr); break;
				}
				sprintf(_describe, CARGS_LANGUAGE_DEFAULT, _formatted);
			}
			if (param->letter == NULL) {
				fprintf(file, CARGS_FORMAT_PARAM1, param->name, _describe);
			} else {
				fprintf(file, CARGS_FORMAT_PARAM2, param->letter, param->name, _describe);
			}
			param = param->next;
		}
	}
}

int _cargs_test_main(int argc, const char **argv) {
	cArgs args = cargs_new(argc, argv);
	const char *name = args.path;
	// 
	bool show = false;
	int count = 0;
	float value = 0.0f;
	char *path = NULL;
	//
	bool *help = cargs_command("h", "help", "supported command");
	//
	bool *work = cargs_command("w", "work", "work command");
	cargs_integer("c", "count", &count);
	cargs_number("v", "value", &value);
	cargs_string("p", "path", &path);
	//
	bool *other = cargs_command("o", "other", "other command");
	cargs_boolean("s", "show", &show);
	cargs_number("v", "value", &value);
	cargs_string("p", "path", &path);
	//
	int err = cargs_run(&args);
	if (err != CARGS_OK) {
		fprintf(stderr, " error code %d \n", err);
	}
	if (*help) {
		printf("use of %s:\nOptions:\n", name);
		cargs_print(stdout);
	}
	if(path != NULL && path[0] != '\0') {
		printf("path: %s \n", path);
	}
}

#ifdef __cplusplus
}
#endif
