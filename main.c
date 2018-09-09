#include <stdio.h>
#include "args.h"

dictionnary_t* DICTIONNARY = NULL;
dictionnary_t* dictionnary_generator();

void f_void();
void f_int(int p);
void f_float(float p);
void f_string(const char* p);

int main(int argc, char const *argv[])
{
  dictionnary_generator();
  option_t* options = parser(argc, argv, DICTIONNARY);
  execute(options);
  return 0;
}


dictionnary_t* dictionnary_generator()
{
  DICTIONNARY = register_void(DICTIONNARY, "-a", f_void);
  DICTIONNARY = register_int(DICTIONNARY, "-b", f_int);
  DICTIONNARY = register_float(DICTIONNARY, "-c", f_float);
  DICTIONNARY = register_string(DICTIONNARY, "-d", f_string);

  return DICTIONNARY;
}



void f_void()
{
  printf("Hello i'am a void function \n");
}

void f_int(int p)
{
  printf("Hello i'am a int function, and my param = |%d| \n",p);
}

void f_float(float p)
{
  printf("Hello i'am a float function, and my param = |%f| \n",p);
}

void f_string(const char* p)
{
  printf("Hello i'am a string function, and my param = |%s| \n",p);

}
