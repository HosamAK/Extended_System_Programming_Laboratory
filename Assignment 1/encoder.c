#include <stdio.h>

int main(int argc, char *argv[]){
    int debug = 0;//0 - debug mode off , 1 - debug mode on
    int oper = 0;//0 - no operation, 1 - add, -1 - substraction
    char *ckey;
    int index = 0;
    FILE *infile = stdin;
    FILE *outfile = stdout; 
    


    for(int i=1; i<argc; i++){
        if(strcmp(argv[i], "-D" )== 0)
            debug = 0;
            
        if(debug)
            fprintf(stderr, "Incorrect Argument: %s\n", argv[i]);
        
        if(strcmp(argv[i], "+D") == 0)
            debug = 1;

        if(strncmp(argv[i], "-e", 2) == 0){
            ckey = argv[i]+2;
            oper = -1;
        }
        
        if(strncmp(argv[i], "+e", 2) == 0){
            ckey = argv[i]+2;
            oper = 1;
        }

        if(strncmp(argv[i], "-i", 2) == 0){
            infile = fopen(argv[i]+2, "r");
            if(infile == NULL){
                fprintf(stderr, "Failed to open the file: %s", argv[i]+2);
                return 1;
            }
        }

        if(strncmp(argv[i], "-o", 2) == 0){
            outfile = fopen(argv[i]+2, "w");
        }
    }
    if(oper || oper == -1){
    int c = fgetc(infile);
    while(c != EOF){

    //encoding number
        if(c >= '0' && c <= '9'){
            c = c +  oper * (ckey[index] - '0');
            if (c > '9') {
                c -= 10;
            } else if (c < '0') {
                c += 10;
            }
        } else if(c >= 'A' && c <= 'Z'){ //encode capital letter
            c = c +  oper * (ckey[index] - '0');
            if(c > 'Z')
                c -= 26;
            else if(c < 'A')
                c+= 26;
        } else if(c >= 'a' && c <= 'z'){// encode small letter
            c = c +  oper * (ckey[index] - '0');
            if(c > 'z')
                c -= 26;
            else if(c < 'a')
                c += 26;
        }
        index++;
        if(ckey[index] == '\0')
                index = 0;
        fputc(c, outfile);
        c = fgetc(infile);

    }
    }
    else{
         int c = fgetc(infile);
    while(c != EOF){
        fputc(c, outfile);
        c = fgetc(infile);
    }

    }
    fclose(infile);
    
    return 0;
}