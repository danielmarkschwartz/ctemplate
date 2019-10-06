#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUF_SIZE 1024
#define BUF_APPEND(buf,c) do{assert(buf##_n<BUF_SIZE); (buf)[buf##_n++] = (c);}while(0)
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

char fmt[BUF_SIZE], *fmt_param[BUF_SIZE];
size_t fmt_n = 0, fmt_param_n = 0;

void fmt_append_str(char *s);

void fmt_append(char c) {
    if(c == '\n') fmt_append_str("\\n");
    else BUF_APPEND(fmt,c);
}

void fmt_append_str(char *s) {
    assert(s);
    while(*s) fmt_append(*(s++));
}

void fmt_append_param(char *s) {
    assert(s);
    BUF_APPEND(fmt_param, s);
}

void fmt_dump(void) {
    if(fmt_param_n > 0) {
        printf("vals = select(\"");
        for(size_t i = 0; i < fmt_param_n; i++) {
            if(i > 0) printf(", ");
            printf("%s", fmt_param[i]);
        }
        printf("\");\n");
    }
    printf("printf(\"%.*s\"", (int)fmt_n, fmt);
    for(size_t i = 0; i < fmt_param_n; i++)
        printf(", vals[%lu]", i);
    printf(");\n");
}

int main(int argc, char **argv) {
    if(argc != 2) error("Expected one argument: ctemplate <template>");

    FILE *template = fopen(argv[1], "r");
    if(!template) error("Couldn't open template file \"%s\" for reading\n", argv[1]);

    char select_buf[BUF_SIZE];
    size_t select_buf_n = 0;
    enum {START, SELECT} state = START;
    int c;

    while(c = fgetc(template), c != EOF) {
        switch(state) {
        case START:
            if((c = match_double(c, '{', template)) == 0) {
                state = SELECT;
                break;
            }

            fmt_append(c);
            break;

        case SELECT:
            if((c = match_double(c, '}', template)) == 0) {
                state = START;
                BUF_APPEND(select_buf, '\0');
                fmt_append_param(strdup(select_buf));
                select_buf_n = 0;
                fmt_append_str("%s");
                break;
            }

            BUF_APPEND(select_buf, c);
            break;

        default: assert(0); //Unexpected state value
        }
    }

    if(state != START) error("Unmatched {{");

    fmt_dump();

    return 0;
}
