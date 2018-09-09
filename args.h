#include <stdlib.h>
#include <stdio.h>

typedef struct option option_t;
typedef struct dictionnary dictionnary_t;


option_t* parser(int argc, const char* argv[], dictionnary_t* dico);

dictionnary_t* register_void(dictionnary_t* parent, const char* pattern, void (*function)() );
dictionnary_t* register_int(dictionnary_t* parent, const char* pattern, void (*function)(int) );
dictionnary_t* register_float(dictionnary_t* parent, const char* pattern, void (*function)(float) );
dictionnary_t* register_string(dictionnary_t* parent, const char* pattern, void (*function)(const char*) );

void execute(option_t* options);

void free_options(option_t*);
void free_dictionnary(dictionnary_t*);
