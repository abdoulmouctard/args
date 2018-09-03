#include <stdlib.h>


typedef struct option option_t;


void init_void_option(option_t* option,const char* key_word, void (* f)());
void init_int_option(option_t* option,const char* key_word, void (* f)(int));
void init_float_option(option_t* option,const char* key_word, void (* f)(float));
void init_string_option(option_t* option,const char* key_word, void (* f)(const char *));
option_t* parser(int argc, const char* argv[]);
//
