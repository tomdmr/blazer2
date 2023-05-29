#include <stdint.h>
#include <inttypes.h>
uint64_t
hashl(unsigned char* str){
  uint64_t hash = 5831;
  int c;

  while(c = *str++){
    hash = ((hash<<5)+hash) ^ c;
  }
  return hash;
}


unsigned int
hashi(unsigned char* str){
  unsigned int hash = 5831;
  int c;

  while(c = *str++){
    hash = ((hash<<5)+hash) ^ c;
  }
  return hash;
}

#include <stdio.h>

int
main(int argc, char** argv){
  if(argc<2)
    return 0;
  printf("%016"PRIx64"\n", hashl(argv[1]));
  //fprintf(stderr, "<%s>\n", argv[1]);
  return 0;
}
