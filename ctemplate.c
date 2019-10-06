#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define BUF_SIZE 1024
#define BUF_APPEND(buf,c) do{assert(buf##_n<BUF_SIZE); (buf)[buf##_n++] = (char)(c);}while(0)
#define error(errmsg, ...) do{ fprintf(stderr, "ERROR: " errmsg ,##__VA_ARGS__); exit(1); }while(0)

int match_double(int i, char c, FILE *f) {
    if(i == c) {
        i = fgetc(f);
        if(i == c) return 0;
        ungetc(i, f);
        return c;
    }
    return i;
}

int main(int argc, char **argv) {
    if(argc != 2) error("Expected one argument: ctemplate <template>");

    FILE *template = fopen(argv[1], "r");
    if(!template) error("Couldn't open template file \"%s\" for reading\n", argv[1]);

    char print_buf[BUF_SIZE];
    char select_buf[BUF_SIZE];
    size_t print_buf_n = 0, select_buf_n = 0;
    enum {START, SELECT} state = START;
    int c;

    while(c = fgetc(template), c != EOF) {
        switch(state) {
        case START:
            if((c = match_double(c, '{', template)) == 0) {
                state = SELECT;
                BUF_APPEND(print_buf, '\0');
                printf("print \"%s\"\n", print_buf);
                print_buf_n = 0;
                break;
            }

            BUF_APPEND(print_buf, c);
            break;

        case SELECT:
            if((c = match_double(c, '}', template)) == 0) {
                state = START;
                BUF_APPEND(select_buf, '\0');
                printf("select \"%s\"\n", select_buf);
                select_buf_n = 0;
                break;
            }

            BUF_APPEND(select_buf, c);
            break;

        default: assert(0); //Unexpected state value
        }
    }

    if(state != START) error("Unmatched {{");

    if(print_buf_n > 0){
        BUF_APPEND(print_buf, '\0');
        printf("print \"%s\"\n", print_buf);
    }

    return 0;
}
