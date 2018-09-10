#include "args.h"
#include <string.h>
#include <stdbool.h>
#include <fcntl.h>
#include <unistd.h>


#define COLOR_NC "\e[0m" // No Color
#define COLOR_RED "\e[1;31m"
#define COLOR_WHITE "\e[1;37m"
#define COLOR_YELLOW "\e[1;33m"
#define COLOR_LIGHT_CYAN "\e[1;36m"

// #define COLOR_GREEN "\e[0;32m"
// #define COLOR_BLACK "\e[0;30m"
// #define COLOR_BLUE "\e[0;34m"
// #define COLOR_LIGHT_BLUE "\e[1;34m"
// #define COLOR_LIGHT_GREEN "\e[1;32m"
// #define COLOR_CYAN "\e[0;36m"
// #define COLOR_LIGHT_RED "\e[1;31m"
// #define COLOR_PURPLE "\e[0;35m"
// #define COLOR_LIGHT_PURPLE "\e[1;35m"
// #define COLOR_BROWN "\e[0;33m"
// #define COLOR_GRAY "\e[0;30m"
// #define COLOR_LIGHT_GRAY "\e[0;37m"

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
  // ADD HERE YOUR CUSTOM FUNTIONS
}action_t;

struct dictionnary
{
  char* name;
  type_t type;
  action_t action;
  enum{SORT=0,LONG}nature;
  bool used;
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


typedef enum{
  INFO = 1,
  WARNING,
  ERROR
}message_t;


void alert(const char* message, message_t type)
{
  if (message == NULL) return;
  int file_desc = STDIN_FILENO;
  char buffer[2048] = {'\0'};
  if (type == INFO) {
    int n = sprintf(buffer, "\n%s  %s!!-INFORMATION-!! %s=>%s %s \n",COLOR_NC,COLOR_LIGHT_CYAN, COLOR_WHITE, COLOR_NC, message);
    write(file_desc, buffer, n+1);
  }else if (type == WARNING) {
    int n = sprintf(buffer, "\n%s  %s!!-WARNING-!! %s=>%s %s \n",COLOR_NC,COLOR_YELLOW, COLOR_WHITE, COLOR_NC, message);
    write(file_desc, buffer, n+1);
  }else if (type == ERROR) {
    int n = sprintf(buffer, "\n%s  %s!!-ERROR-!! %s=>%s %s \n",COLOR_NC,COLOR_RED, COLOR_WHITE, COLOR_NC, message);
    write(file_desc, buffer, n+1);
  }
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
  char* name = str_copy(key_word);
  option_t* ret = NULL;

  ret = malloc(sizeof(option_t));
  if ((opt = get_last(option)) == NULL){
    ret->parent = NULL;
  }else{
    opt->next = ret;
    ret->parent = opt;
  }

  ret->next = NULL;
  ret->name = name;

  return ret;
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


bool is_long_arg_without_value(const char* param)
{
  if(param == NULL || strlen(param) < 3 || param[0] != '-' || param[1] != '-') return false;
  for (int i = 2; i < strlen(param); i++) {
    if (!is_letter(param[i])) {
      return false;
    }
  }
  return true;
}


bool is_long_arg_with_value(const char* param)
{
  if (param == NULL || strlen(param) <= 4) return false; // --x=y
  char* tmp = strstr(param, "=");
  return (param[0]=='-' && param[1]=='-' && is_letter(param[2])) &&
         (tmp != NULL && (tmp-param > 1));
}


char* trim_pattern(const char* pattern)
{
  if (pattern == NULL || strlen(pattern) == 0) return NULL;
  char* ret;
  if (is_single_arg(pattern)) return str_copy(pattern+1);
  else if (is_long_arg_without_value(pattern)) return str_copy(pattern+2);

  // is_long_arg_with_value
  ret = strpbrk(pattern, "=");
  ret[0] = '\0';
  char* tmp = str_copy(pattern);
  ret[0] = '=';
  return tmp;
}

dictionnary_t* matched(const char* key_word, dictionnary_t* dico)
{
  if(dico == NULL) return NULL;
  int i=0;
  char* pattern = trim_pattern(key_word);
  dictionnary_t* tmp = dico;
  dictionnary_t* ret = NULL;

  while (tmp != NULL) {
    if (strcmp(tmp->name, pattern) == 0) {
      ret = tmp;
      break;
    }
    tmp = tmp->next;
  }
  free(pattern);

  if (ret != NULL && ret->used) return NULL;
  return ret;
}


option_t* init_option(option_t* options, dictionnary_t* dico,const char* argv)
{
  dico->used = true;
  if (dico->type == VOID) {
    options = init_void_option(options,dico->name, dico->action.option_void);
  }else {
    if (argv != NULL){
      if (dico->type == INT) {
        options = init_int_option(options,dico->name, dico->action.option_void,argv);
      }else if (dico->type == FLOAT) {
        options = init_float_option(options,dico->name, dico->action.option_float,argv);
      }else if (dico->type == STRING) {
        options = init_string_option(options,dico->name, dico->action.option_string,argv);
      }else{/* ADD HERE YOUR TYPES HANDLERS */}
    }else{
      char t[100]={'\0'}; snprintf(t, 100, "There is no value for the argument %s ", dico->name);
      alert(t, WARNING);
    }
  }
  return options;
}

option_t* parser(int argc, const char* argv[], dictionnary_t* dico)
{
  if (argc <= 1 || dico == NULL) return NULL;
  option_t* options = NULL;
  dictionnary_t* d = NULL;
  char** tmp = NULL;
  bool errors = false;
  for (int i = 1; i < argc && !errors; i++) {
    if (is_single_arg(argv[i])) {
      if ((d = matched(argv[i], dico)) != NULL && d->nature == SORT) {
        options = init_option(options, d, (i+1<argc)?argv[i+1]:NULL);
        if (d->type != VOID) i++;
      }else{
        errors = true;
        char t[100]={'\0'}; snprintf(t, 100, "Invalid option <<%s>> ", argv[i]);
        alert(t, WARNING);

      }
    }else if (is_many_sort_args(argv[i])) {
      char* tmp = malloc(sizeof(char)*3);
      tmp[0] = '-';
      tmp[2] = '\0';
      for (int j = 1; j < strlen(argv[i]) && !errors; j++) {
        tmp[1] = (char)argv[i][j];
        if ((d = matched(tmp, dico)) != NULL && d->type == VOID && d->nature == SORT) {
          options = init_void_option(options,d->name, d->action.option_void);
        }else{
          errors = true;
          char t[100]={'\0'}; snprintf(t, 100, "The option <<%s>> is not valid", tmp);
          alert(t, WARNING);
        }
      }
      free(tmp);
    }else if (is_long_arg_without_value(argv[i])) {
      if ( (d = matched((argv[i]), dico)) != NULL && d->nature == LONG) {
        options = init_void_option(options,d->name, d->action.option_void);
      }else{
        errors = true;
        char t[100]={'\0'}; snprintf(t, 100, "The option <<%s>> is not valid", argv[i]);
        alert(t, WARNING);
      }
    }else if (is_long_arg_with_value(argv[i])) {
      if ((d = matched(argv[i]+2, dico)) != NULL && d->type != VOID) {
        options = init_option(options,d,(strpbrk(argv[i], "=")+1));
      }else{
        errors = true;
        char t[100]={'\0'}; snprintf(t, 100, "The option <<%s>> is not valid", argv[i]);
        alert(t, WARNING);
      }
    }else{
      char t[100]={'\0'}; snprintf(t, 100, "I don't recognized this value <<%s>>", argv[i]);
      alert(t, WARNING);
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

  if (is_single_arg(pattern)) {
    child->nature = SORT;
  }else if (is_long_arg_without_value(pattern)) {
    child->nature = LONG;
  }else{
    free_dictionnary(child);
    return NULL;
  }

  child->name = trim_pattern(pattern);
  child->used = false;

  return child;
}

dictionnary_t* register_void(dictionnary_t* parent, const char* pattern, void (*function)())
{
  dictionnary_t* child = base_register(parent, pattern);
  child->action.option_void = function;
  child->type = VOID;
  if (parent == NULL) return child;
  return parent;
}

dictionnary_t* register_int(dictionnary_t* parent, const char* pattern, void (*function)(int))
{
  dictionnary_t* child = base_register(parent, pattern);
  child->action.option_int = function;
  child->type = INT;

  if (parent == NULL) return child;
  return parent;
}

dictionnary_t* register_float(dictionnary_t* parent, const char* pattern, void (*function)(float))
{
  dictionnary_t* child = base_register(parent, pattern);
  child->action.option_float = function;
  child->type = FLOAT;

  if (parent == NULL) return child;
  return parent;
}

dictionnary_t* register_string(dictionnary_t* parent, const char* pattern, void (*function)(const char*))
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
  return free_option(get_first(option));
}
void free_dictionnary(dictionnary_t* dico)
{
  if (dico == NULL) return;
  free(dico->name);
  return free_dictionnary(dico->next);
}


void execute(option_t* options)
{
  if (options == NULL) return;

  option_t* tmp = get_first(options);
  while (tmp != NULL) {
    if (tmp->type == VOID) { tmp->action.option_void();}
    else if (tmp->type == INT) { tmp->action.option_int(tmp->value.int_value);}
    else if (tmp->type == FLOAT) { tmp->action.option_float(tmp->value.float_value);}
    else if (tmp->type == STRING) { tmp->action.option_string(tmp->value.string_value);}
    else {/* ERROR HERE, IF YUO ADD OTHER TYPE */}
    tmp = tmp->next;
  }
}
