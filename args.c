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

struct dictionnary
{
  char* name;
  type_t type;
  action_t action;
  struct dictionnary* next;
};

struct option
{
  char* name;
  type_t type;
  action_t action;
  struct option* parent;
  struct option* next;
  union{
    int int_value;
    float float_value;
    char* string_value;
  }value;
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
  ret[n] = '\0';
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

option_t* init_void_option(option_t* option,const char* key_word, void (* f)())
{
  option_t* opt = partial_init(option,key_word);
  opt->action.option_void = f;
  opt->type = VOID;
  return opt;
}

option_t* init_int_option(option_t* option,const char* key_word, void (* f)(int),const char* value)
{
  option_t* opt = partial_init(option,key_word);
  opt->action.option_int = f;
  opt->type = INT;
  opt->value.int_value = atoi(value);
  return opt;
}

option_t* init_float_option(option_t* option,const char* key_word, void (* f)(float),const char* value)
{
  option_t* opt = partial_init(option,key_word);
  opt->action.option_float = f;
  opt->type = FLOAT;
  opt->value.float_value = (float)atof(value);
  return opt;
}

option_t* init_string_option(option_t* option,const char* key_word, void (* f)(const char *),const char* value)
{
  option_t* opt = partial_init(option,key_word);
  opt->action.option_string = f;
  opt->type = STRING;
  opt->value.string_value = str_copy(value);
  return opt;
}

bool is_letter(char c)
{
  return  (c >= 97 && c <= 122) || (c >= 65 && c <= 90);
}

bool is_single_arg(const char*param)
{
  return (param != NULL && strlen(param)==2&& param[0]=='-' && is_letter(param[1]));
}

bool is_many_sort_args(const char* param)
{
  if (param == NULL || strlen(param)<=2 || param[0] !='-') return false;
  for (int i = 1; i < strlen(param); i++) {
    if (!is_letter(param[i])) {
      return false;
    }
  }
  return true;
}


bool is_long_arg_without_value(char* param)
{
  if(param == NULL || strlen(param) < 3 || param[0] != '-' || param[1] != '-') return false;
  for (int i = 2; i < strlen(param); i++) {
    if (!is_letter(param[i])) {
      return false;
    }
  }
  return true;
}


bool is_long_arg_with_value(char* param)
{
  if (param == NULL || strlen(param) <= 4) return false; // --x=y
  char* tmp = strstr(param, "=");
  return (param[0]=='-' && param[1]=='-' && is_letter(param[2])) &&
         (tmp != NULL && (tmp-param > 1));
}

dictionnary_t* matched(const char* key_word, dictionnary_t* dico)
{
  if(dico == NULL) return NULL;
  int i=0;
  char* pattern;

  if (is_single_arg(key_word)) {
    pattern = str_copy(key_word);
  }else{
    // add others ...
    return NULL;
  }

  dictionnary_t* tmp = dico;
  dictionnary_t* ret = NULL;

  while (tmp != NULL) {
    if (strcmp(tmp->name, pattern) == 0) {
      ret = tmp;
      break;
    }
    tmp = tmp->next;
  }

  return ret;
}


option_t* parser(int argc, const char* argv[], dictionnary_t* dico)
{
  if (argc <= 1 || dico == NULL) return NULL;
  option_t* options = NULL;
  dictionnary_t* d = NULL;

  for (int i = 1; i < argc; i++) {
    if ((d = matched(argv[i], dico)) != NULL) {
      if (d->type == VOID) {
        options = init_void_option(options,d->name, d->action.option_void);
      }else if ( i+1<argc) {
        switch (d->type) {
          case INT:
            options = init_int_option(options,d->name, d->action.option_void,argv[i+1]);
          break;
          case FLOAT:
            options = init_float_option(options,d->name, d->action.option_float,argv[i+1]);
          break;
          case STRING:
            options = init_string_option(options,d->name, d->action.option_string,argv[i+1]);
          break;
        }
        i++;
      }
    }else{
      printf("Something went wrong %d\n",i);
    }
  }

  return options;
}


dictionnary_t* base_register(dictionnary_t* parent, const char* pattern)
{
  dictionnary_t* child = NULL;
  if (parent == NULL) {
    parent = malloc(sizeof(dictionnary_t));
    child = parent;
  }else{
    dictionnary_t* tmp =parent;
    while (tmp->next != NULL) {
      tmp = tmp->next;
    }
    tmp->next = malloc(sizeof(dictionnary_t));
    child = tmp->next;
  }

  child->name = str_copy(pattern);
}

dictionnary_t* register_void(dictionnary_t* parent, const char* pattern, void (*function)() )
{
  dictionnary_t* child = base_register(parent, pattern);
  child->action.option_void = function;
  child->type = VOID;

  if (parent == NULL) return child;
  return parent;
}
dictionnary_t* register_int(dictionnary_t* parent, const char* pattern, void (*function)(int) )
{
  dictionnary_t* child = base_register(parent, pattern);
  child->action.option_int = function;
  child->type = INT;

  if (parent == NULL) return child;
  return parent;
}
dictionnary_t* register_float(dictionnary_t* parent, const char* pattern, void (*function)(float) )
{
  dictionnary_t* child = base_register(parent, pattern);
  child->action.option_float = function;
  child->type = FLOAT;

  if (parent == NULL) return child;
  return parent;
}
dictionnary_t* register_string(dictionnary_t* parent, const char* pattern, void (*function)(const char*) )
{
  dictionnary_t* child = base_register(parent, pattern);
  child->action.option_string = function;
  child->type = STRING;

  if (parent == NULL) return child;
  return parent;
}



void free_option(option_t* option)
{
  if (option == NULL) return;
  free(option->name);
  if (option->type == STRING) {
    free(option->value.string_value);
  }
  return free_option(option->next);
}


void free_options(option_t* option)
{
  if (option == NULL) return;
  return free_options(get_first(option));
}
void free_dictionnary(dictionnary_t* dico)
{
  free(dico->name);
  return free_dictionnary(dico->next);
}


void execute(option_t* options)
{
  if (options == NULL) return;
  options = get_first(options);
  option_t* tmp = options;
  while (tmp != NULL) {
    switch (tmp->type) {
      case VOID:
        tmp->action.option_void();
        break;
      case INT:
        tmp->action.option_int(tmp->value.int_value);
        break;
      case FLOAT:
        tmp->action.option_float(tmp->value.float_value);
        break;
      case STRING:
        tmp->action.option_string(tmp->value.string_value);
        break;
    }
    tmp = tmp->next;
  }
}
