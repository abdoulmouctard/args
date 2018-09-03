#include "args.h"
#include <string.h>
#include <stdbool.h>

typedef enum
{
  VOID=0,
  INT,
  FLOAT,
  STRING
} type_t;

typedef union
{
  void (*option_void)();
  void (*option_int)(int);
  void (*option_float)(float);
  void (*option_string)(const char *);
}action_t;

typedef struct
{
  char* name;
  type_t type;
  action_t action;
}dictionnary_t;

struct option
{
  char* name;
  type_t type;
  // enum {VOID=0,INT,FLOAT,STRING}type;
  // union {
  //   void (*option_void)();
  //   void (*option_int)(int);
  //   void (*option_float)(float);
  //   void (*option_string)(const char *);
  // }action;
  action_t action;
  struct option* parent;
  struct option* next;
};

int str_count(const char * str,char c)
{
  return (str == NULL || str[0] == '\0')?0:(1+str_count(str+1, c));
}

char* str_copy(const char* word)
{
  if (word == NULL) return NULL;
  int n = strlen(word);
  char* ret = malloc(sizeof(char)*n+1);
  strcpy(ret, word);
  return ret;
}

option_t* get_last(option_t* list)
{
  if (list == NULL || list->next == NULL) return list;
  return get_last(list->next);
}

option_t* get_first(option_t* list)
{
  if (list == NULL || list->parent == NULL) return list;
  return get_first(list->parent);
}

option_t* partial_init(option_t*option,const char * key_word)
{
  option_t* opt;
  if ((opt = get_last(option)) == NULL){
    opt = malloc(sizeof(option_t));
    opt->parent = NULL;
  }else{
    opt->next = malloc(sizeof(option_t));
    opt->next->parent = opt;
  }
  opt->next = NULL;
  opt->name = str_copy(key_word);

  return opt;
}

void init_void_option(option_t* option,const char* key_word, void (* f)())
{
  option_t* opt = partial_init(option,key_word);
  option->action.option_void = f;
  option->type = VOID;
}

void init_int_option(option_t* option,const char* key_word, void (* f)(int))
{
  option_t* opt = partial_init(option,key_word);
  option->action.option_int = f;
  option->type = INT;
}

void init_float_option(option_t* option,const char* key_word, void (* f)(float))
{
  option_t* opt = partial_init(option,key_word);
  option->action.option_float = f;
  option->type = FLOAT;
}

void init_string_option(option_t* option,const char* key_word, void (* f)(const char *))
{
  option_t* opt = partial_init(option,key_word);
  option->action.option_string = f;
  option->type = STRING;
}

bool is_sort_arg(const char* param)
{
  if (param == NULL|| strlen(param) <= 1) return NULL;

  int i;
  bool error = false;
  for (i = 1; i < strlen(param) && !error; i++) {
    if ( !(param[i] >= 97 && param[i] <= 122) && !(param[i] >= 65 && param[i] <= 90) ) {
      error = true;
    }
  }
  return !error;
}


bool is_long_arg(char* param)
{
  if (param == NULL || strlen(param) <= 4) return false; // --x=y

  bool ret;
  char* tmp = strstr(param, "=");
  return (param[0]=='-' && param[1]=='-' && param[0] != '=') &&
         (tmp != NULL && (tmp-param > 1));
}


char** splite_sort_args(const char* keys)
{
  if (is_sort_arg(keys))return NULL;

  int n = strlen(keys);
  int i;
  char ** ret = malloc(sizeof(char)*n);
  for ( i = 0; i < n-1; i++) {
    ret[i] = malloc(sizeof(char)*2);
  }
  ret[n-1] = NULL;

  for (i = 0; i < strlen(keys)-1; i++) {
    ret[i][0] = keys[i+1];
    ret[i][1] = '\0';
  }
  return ret;
}


option_t* matched(char* key_word, dictionnary_t** dico)
{
  if(dico == NULL) return NULL;
  int i=0;
  while (dico[i] != NULL) {
    if (strcmp(dico[i]->name, key_word)) {
      return dico[i];
    }
    i++;
  }
  return NULL;
}




option_t* parser(int argc, const char* argv[])
{
  if (argc <= 1) return NULL;
  int i;
  bool has_errors = false;

  for ( i = 1; i < argc && !has_errors; i++) {
    if (!is_sort_arg(argv[i]) && !is_long_arg(argv[i])) {
      has_errors = true;
    }
  }
  printf("Nombre d'arguments  = %d \n", argc-1 );

  if (has_errors) return NULL;

  option_t* list = NULL;// = malloc(sizeof(option_t));
  for (i = 1; i < argc; i++) {
    if (is_sort_arg(argv[i])) {
      // PUSH
    }else if(is_long_arg(argv[i])) {
      // PUSH
    }
  }

  return list;
}
