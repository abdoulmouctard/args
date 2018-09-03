#include <stdio.h>
#include "args.h"

int main(int argc, char const *argv[])
{

  option_t* options = parser(argc, argv);

  printf("Options : %p\n", options );

  printf("Hello world!! %d\n",argc);
  return 0;
}
