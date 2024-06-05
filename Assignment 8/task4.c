#include <stdio.h>

int count_digit(char* argv) {
    int c = 0;
    char* p_str = argv;

    while (*p_str != '\0') {
        if (*p_str >= '0' && *p_str <= '9') {
            c++;
        }
        p_str++;
    }

    return c;
}

int main(int argc, char** argv) {
    if(argc < 2){
        perror("Few Arguments\n");
        return 1;
    }
    printf("number of digits: %d\n", count_digit(argv[1]));
    return 0;
}
