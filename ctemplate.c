#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#define BUF_SIZE 1024
#define error(errmsg, ...) do{ fprintf(stderr, "ERROR: " errmsg ,##__VA_ARGS__); exit(1); }while(0)

int main(int argc, char **argv) {
    if(argc != 2) error("Expected one argument: ctemplate <template>");

    FILE *template = fopen(argv[1], "r");
    if(!template) error("Couldn't open template file \"%s\" for reading\n", argv[1]);

    char print_buf[BUF_SIZE];
    char select_buf[BUF_SIZE];
    size_t p_n = 0, s_n = 0;
    enum {START, SELECT} state = START;
    int c;

    while(c = fgetc(template), c != EOF) {
        switch(state) {
        case START:
            if(c == '{') {
                c = fgetc(template);
                if( c == '{') {
                    assert(p_n < BUF_SIZE);
                    print_buf[p_n] = '\0';
                    printf("print \"%s\"\n", print_buf);
                    p_n = 0;
                    state = SELECT;
                    break;
                }

                ungetc(c, template);
                c = '{';
            }

            assert(p_n < BUF_SIZE);
            print_buf[p_n++] = (char)c;
            break;

        case SELECT:
            if(c == '}') {
                c = fgetc(template);
                if( c == '}') {
                    state = START;
                    assert(s_n < BUF_SIZE);
                    select_buf[s_n] = '\0';
                    printf("select \"%s\"\n", select_buf);
                    s_n = 0;
                    break;
                }

                ungetc(c, template);
                c = '}';
            }

            assert(s_n < BUF_SIZE);
            select_buf[s_n++] = (char)c;
            break;

        default: assert(0); //Unexpected state value
        }
    }

    if(state != START) error("Unmatched {{");

    if(p_n > 0){
        assert(p_n < BUF_SIZE);
        print_buf[p_n] = '\0';
        printf("print \"%s\"\n", print_buf);
    }

    return 0;
}
