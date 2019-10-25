#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUF_SIZE 1024
#define BUF_APPEND(buf,c) do{assert(buf##_n<BUF_SIZE); (buf)[buf##_n++] = (c);}while(0)
#define error(errmsg, ...) do{ fprintf(stderr, "ERROR: " errmsg ,##__VA_ARGS__); exit(1); }while(0)

void print_preamble(void) {
    puts(
            "#include <assert.h>\n"
            "#include <ctype.h>\n"
            "#include <stdio.h>\n"
            "#include <stdlib.h>\n"
            "#include <strings.h>\n"
            "#include <sqlite3.h>\n"
            "\n"
            "#define SQL_MAX 1024\n"
            "#define error(errmsg, ...) do{ fprintf(stderr, \"ERROR: \" errmsg ,##__VA_ARGS__); exit(1); }while(0)\n"
            "\n"
            "sqlite3 *db;\n"
            "\n"
            "void print_esc_html(char *s){\n"
            "   assert(s);\n"
            "   for(;*s;s++){"
            "       switch(*s){\n"
            "           case '<': fputs(\"&lt;\", stdout); break;\n"
            "           case '>': fputs(\"&gt;\", stdout); break;\n"
            "           case '&': fputs(\"&amp;\", stdout); break;\n"
            "           default: putchar(*s);\n"
            "       }\n"
            "   }\n"
            "}\n"
            "\n"
            "void print_esc_url(char *s){\n"
            "   assert(s);\n"
            "   for(;*s;s++){"
            "       if(isalnum(*s) || *s == '-' || *s == '.' || *s == '_' || *s == '~')\n"
            "           putchar(*s);\n"
            "       else printf(\"%%%x\", *s);\n"
            "   }\n"
            "}\n"
            "\n"
            "char **select(char *sel, size_t n, char *tbl, char *where){\n"
            "   sqlite3_stmt *res;\n"
            "   char sql[SQL_MAX];\n"
            "   int got = snprintf(sql, SQL_MAX, \"SELECT %s\", sel);\n"
            "   if(tbl && strstr(sel, \"FROM\") == NULL)\n"
            "       got += snprintf(&sql[got], SQL_MAX, \" FROM %s\", tbl);\n"
            "   if(where && strstr(sel, \"WHERE\") == NULL)\n"
            "       got += snprintf(&sql[got], SQL_MAX, \" WHERE %s\", where);\n"
            "\n"
            "   int rc = sqlite3_prepare_v2(db, sql, -1, &res, 0);\n"
            "   if (rc != SQLITE_OK) \n"
            "       error(\"Failed to execute statement: %s\\nstatement: '%s'\\n\", sqlite3_errmsg(db), sql);\n"
            "\n"
            "   int step = sqlite3_step(res);\n"
            "   if (step != SQLITE_ROW) return NULL;\n"
            "\n"
            "   char **vals = malloc(sizeof(*vals) * n);\n"
            "   if(!vals) return NULL;\n"
            "\n"
            "   for(size_t i = 0; i < n; i++)\n"
            "       vals[i] = strdup((const char *)sqlite3_column_text(res, i));\n"
            "\n"
            "   sqlite3_finalize(res);\n"
            "   return vals;\n"
            "}\n"
            "\n"
            "int main(int argc, char **argv){\n"
            "   char *err_msg = 0;\n"
            "\n"
            "   char *where = NULL;\n"
            "   char *tbl = NULL;\n"
            "\n"
            "   for(int i = 1; i < argc; i++) {\n"
            "       if(strcmp(\"--where\", argv[i]) == 0) {\n"
            "           i++;\n"
            "           if(i >= argc) error(\"Expected argument after --where\");\n"
            "           where = argv[i];\n"
            "       }\n"
            "       else if(strcmp(\"--table\", argv[i]) == 0) {\n"
            "           i++;\n"
            "           if(i >= argc) error(\"Expected argument after --table\");\n"
            "           tbl = argv[i];\n"
            "       }\n"
            "   }\n"
            "\n"
            "   int rc = sqlite3_open(argv[1], &db);\n"
            "   if (rc != SQLITE_OK)\n"
            "       error(\"Cannot open database: %s\\n\", sqlite3_errmsg(db));\n"
            "\n"
            "   char **vals;\n"
          );
}

void print_final(void) {
    puts(
            "   sqlite3_close(db);\n"

            "   return 0;\n"
            "}\n"
          );
}

//Prints to std out escaping newline, backslash, and newlines
//suitable for inclusion in a C string
void print_cstr_esc(char *s) {
    assert(s);
    for(;*s;s++) {
        switch(*s) {
            case '\n': fputs("\\n", stdout); break;
            case '\\': fputs("\\\\", stdout); break;
            case '"': fputs("\\\"", stdout); break;
            default: putchar(*s);
        }
    }
}

int match_double(int i, char c, FILE *f) {
    if(i == c) {
        i = fgetc(f);
        if(i == c) return 0;
        ungetc(i, f);
        return c;
    }
    return i;
}

void parse_sql(char *s, char *tbl, char *where, size_t n) {
    char *w;
    for(;;) {
        w = strsep(&s, " ");
        if(w == NULL) break;

        if(strcmp("FROM", w) == 0) {
            w = strsep(&s, " ");
            snprintf(tbl, n, "\"%s\"", w);
        } else
        if(strcmp("WHERE", w) == 0) {
            w = strsep(&s, " ");
            snprintf(where, n, "\"%s\"", w);
        } else {
        }

    }
}

int main(int argc, char **argv) {
    if(argc != 2) error("Expected one argument: ctemplate <template>");

    FILE *template = fopen(argv[1], "r");
    if(!template) error("Couldn't open template file \"%s\" for reading\n", argv[1]);

    char buf[BUF_SIZE];
    size_t buf_n = 0;
    enum {START, SELECT, SELECT_TYPE, SELECT_END} state = START;
    enum {UNESC, URLESC, HTMLESC, ASSERT, NOTASSERT, ENDBLOCK} type;
    int c;

    print_preamble();

    while(c = fgetc(template), c != EOF) {
        switch(state) {
        case SELECT_END:
            if(c == '\n') break;
            state = START;
            //fallthrough

        case START:
            if((c = match_double(c, '{', template)) == 0) {
                state = SELECT_TYPE;
                BUF_APPEND(buf, '\0');
                fputs("    fputs(\"", stdout);
                print_cstr_esc(buf);
                fputs("\", stdout);\n", stdout);
                buf_n = 0;
                break;
            }

            BUF_APPEND(buf, c);
            break;

        case SELECT_TYPE:
            state = SELECT;
            switch(c) {
                case '&': type = UNESC; continue;
                case '%': type = URLESC; continue;
                case '?': type = ASSERT; continue;
                case '^': type = NOTASSERT; continue;
                case '/': type = ENDBLOCK; continue;
                default: type = HTMLESC;
            }
            //fallthrough

        case SELECT:
            if((c = match_double(c, '}', template)) == 0) {
                state = SELECT_END;
                BUF_APPEND(buf, '\0');

                char where[BUF_SIZE] = {0}, tbl[BUF_SIZE] = {0};
                parse_sql(buf, tbl, where, BUF_SIZE);

                if(type != ENDBLOCK) {
                    printf("    vals = select(\"");
                    print_cstr_esc(buf);
                    printf("\", 1, %s, %s);\n",
                            tbl[0]?tbl:"tbl",
                            where[0]?where:"where");
                }

                switch(type) {
                    case HTMLESC: puts("    print_esc_html(vals[0]);"); break;
                    case URLESC: puts("    print_esc_url(vals[0]);"); break;
                    case UNESC: puts("    fputs(vals[0], stdout);"); break;
                    case ASSERT: puts("    if(vals && vals[0] && strlen(vals[0]) > 0 && strcmp(vals[0], \"0\") != 0){"); break;
                    case NOTASSERT: puts("    if(!(vals && vals[0] && strlen(vals[0]) > 0 && strcmp(vals[0], \"0\")) != 0){"); break;
                    case ENDBLOCK: puts("    }"); break;
                    default: assert(0); //Unknown tag type
                }

                buf_n = 0;
                break;
            }

            BUF_APPEND(buf, c);
            break;

        default: assert(0); //Unexpected state value
        }
    }

    if(state != START && state != SELECT_END) error("Unmatched {{");
    if(buf_n > 0) {
        BUF_APPEND(buf, '\0');
        fputs("    fputs(\"", stdout);
        print_cstr_esc(buf);
        fputs("\", stdout);\n", stdout);
    }

    print_final();

    return 0;
}
