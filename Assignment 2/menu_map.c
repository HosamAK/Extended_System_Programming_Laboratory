#include <stdlib.h>
#include <stdio.h>
#include <string.h>
 
char* map(char *array, int array_length, char (*f) (char)){
  char* mapped_array = (char*)(malloc(array_length*sizeof(char)));
  /* TODO: Complete during task 2.a */
  for(int i=0; i<array_length; i++)
    mapped_array[i] = f(array[i]);
  return mapped_array;
}

char my_get(char c){
  /* Ignores c, reads and returns a character from stdin using fgetc. */
  return fgetc(stdin);
}
char cprt(char c){
  /* If c is a number between 0x20 and 0x7E, cprt prints the character of ASCII value c followed by a new line. Otherwise, cprt prints the dot ('.') character. After printing, cprt returns the value of c unchanged. */
  if(0x20<=c && c<=0x7E)
    printf("%c\n", c);
  else
    printf(".\n");
  return c;
}
char encrypt(char c){
  /* Gets a char c and returns its encrypted form by adding 1 to its value. If c is not between 0x20 and 0x7E it is returned unchanged */
  if(0x20<=c && c<=0x7E)
    return c+1;
  return c;
}
char decrypt(char c){
  /* Gets a char c and returns its decrypted form by reducing 1 from its value. If c is not between 0x20 and 0x7E it is returned unchanged */ 
  if(0x20<=c && c<=0x7E)
    return c-1;
  return c;
}
char xprt(char c){
  /* xprt prints the value of c in a hexadecimal representation followed by a new line, and returns c unchanged. */
  if(0x20<=c && c<=0x7E)
    printf("%x\n", c);
  else
    printf(".\n");
  return c;
}

struct fun_desc {
char *name;
char (*fun)(char);
}; 

int main(int argc, char **argv){
  /* TODO: Test your code */
  /*
  int base_len = 5;
  char arr1[base_len];
  char* arr2 = map(arr1, base_len, my_get);
  char* arr3 = map(arr2, base_len, cprt);
  char* arr4 = map(arr3, base_len, xprt);
  char* arr5 = map(arr4, base_len, encrypt);
  char* arr6 = map(arr5, base_len, decrypt);
  free(arr2);
  free(arr3);
  free(arr4);
  free(arr5);
  free(arr6);
  */

  struct fun_desc menu[]= { {"Get String", my_get},
                            {"Print String", cprt},
                            {"Print Hex", xprt},
                            {"Encrypt", encrypt},
                            {"Decrypt", decrypt},
                            {NULL, NULL} };
  char *carray = (char*)(malloc(5));

  char tmp[20];
    printf("Please choose a function (ctrl^D for exit):\n");
    for(int i=0;i<5; i++){
      printf("%d)   %s\n", i, menu[i].name);
    }
  fgets(tmp, 20, stdin);
  //check is in bound
  if((tmp[0]-'0')>=0 && (tmp[0]-'0')<=4){
    printf("Within bounds\n");
  }
  else{
    printf("Not within bounds\n");
    exit(0);
  }

  while(tmp[0]!=EOF){
    int op=tmp[0]-'0';
    carray=map(carray,5,menu[op].fun);
    printf("Done.\n\n");
    printf("Please choose a function (ctrl^D for exit):\n");
    for(int i=0;i<5; i++){
      printf("%d)   %s\n", i, menu[i].name);
    }
    //check
    if(fgets(tmp, 20, stdin)==NULL)
    break;
    if((tmp[0]-'0')>=0 && (tmp[0]-'0')<=4){
    printf("Within bounds\n");
  }
  else{
    printf("Not within bounds\n");
    exit(0);
  }
  }

  free(carray);
  return 0;
}

