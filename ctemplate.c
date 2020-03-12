#include <assert.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUF_SIZE (1024*1024)
#define TEMPLATES_MAX 10
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
            "#define ROWS_MAX 1024\n"
            "#define BUF_SIZE 1024*1024\n"
            "#define error(errmsg, ...) do{ fprintf(stderr, \"ERROR: \" errmsg ,##__VA_ARGS__); exit(1); }while(0)\n"
            "\n"
            "sqlite3 *db;\n"
            "\n"
            "int val_ok(char *val){\n"
            "   return val && strlen(val) > 0 && strcmp(val, \"0\") != 0 && strcmp(val, \"false\") != 0;\n"
            "}\n"
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
            "       else if(*s == ' ')"
            "           putchar('+');\n"
            "       else printf(\"%%%hhx\", *s);\n"
            "   }\n"
            "}\n"
            "\n"
            "void print_esc_space(char *s){\n"
            "   assert(s);\n"
            "   for(;*s;s++){"
            "       if(*s != ' ')\n"
            "           putchar(*s);\n"
            "       else putchar('_');\n"
            "   }\n"
            "}\n"
            "\n"
            "sqlite3_stmt *res = NULL;\n"
            "\n"
            "void done(void){\n"
            "   if(res){\n"
            "       sqlite3_finalize(res);\n"
            "       res = NULL;\n"
            "   }\n"
            "}\n"
            "\n"
            "char *select(char *sel, char *tbl, char *where, char *final){\n"
            "   if(!res) {"
            "       char sql[SQL_MAX];\n"
            "       int got = snprintf(sql, SQL_MAX, \"SELECT %s\", sel);\n"
            "       if(tbl)\n"
            "           got += snprintf(&sql[got], SQL_MAX, \" FROM %s\", tbl);\n"
            "       if(where)\n"
            "           got += snprintf(&sql[got], SQL_MAX, \" WHERE %s\", where);\n"
            "       if(final)\n"
            "           got += snprintf(&sql[got], SQL_MAX, \" %s\", final);\n"
            "\n"
            "       int rc = sqlite3_prepare_v2(db, sql, -1, &res, 0);\n"
            "       if (rc != SQLITE_OK) \n"
            "           error(\"Failed to execute statement: %s\\nstatement: '%s'\\n\", sqlite3_errmsg(db), sql);\n"
            "   }\n"
            "\n"
            "   int step = sqlite3_step(res);\n"
            "   if (step != SQLITE_ROW) {done(); return NULL;}\n"
            "\n"
            "   return (char *)sqlite3_column_text(res, 0);\n"
            "}\n"
            "\n"
            "int main(int argc, char **argv){\n"
            "   char *err_msg = 0;\n"
            "\n"
            "   char *where = NULL;\n"
            "   char *tbl = NULL;\n"
            "   char *rows[ROWS_MAX];\n"
            "   size_t rows_n = 0;\n"
            "   FILE *file;\n"
            "   int got;\n"
            "   char buf[BUF_SIZE];\n"
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
            "   char *val;\n"
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
            case '\t': fputs("\\t", stdout); break;
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

char *strip_space(char *s) {
    if(!s) return s;
    while(isspace(*s)) s++;
    char *ret = s;
    while(*s) s++;
    while(s>ret && isspace(*(s-1))) {
        s--;
        *s = 0;
    }
    return ret;
}

void parse_sql(char *s, char **select, char **tbl, char **where, char **final) {
    *select = 0;
    *tbl = 0;
    *where = 0;
    *final = 0;

    do{
        *select = s;
        char *w = strsep(&s, ";");
        if(w == NULL) break;

        *where = s;
        w = strsep(&s, ";");
        if(w == NULL) break;

        *tbl = s;
        w = strsep(&s, ";");
        if(w == NULL) break;

        *final = s;
    }while(0);

    *select = strip_space(*select);
    *tbl = strip_space(*tbl);
    *where = strip_space(*where);
    *final = strip_space(*final);

    if(*select && !**select) *select = NULL;
    if(*tbl && !**tbl) *tbl = NULL;
    if(*where && !**where) *where = NULL;
    if(*final && !**final) *final = NULL;

}

int main(int argc, char **argv) {
    if(argc != 2) error("Expected one argument: ctemplate <template>");

    FILE *templates[TEMPLATES_MAX];
    int templates_i = 0;
    templates[templates_i] = fopen(argv[1], "r");
    if(!templates[templates_i]) error("Couldn't open template file \"%s\" for reading\n", argv[1]);

    char buf[BUF_SIZE];
    size_t buf_n = 0;
    enum {START, SELECT, SELECT_TYPE, SELECT_END} state = START;
    enum {UNESC, URLESC, SPACEESC, HTMLESC, ASSERT, NOTASSERT, ENDBLOCK, FOREACH, FORELSE, EXECUTE, INCLUDE, SUBTEMPLATE} type;
    int c;

    print_preamble();

    for(;;) {
        c = fgetc(templates[templates_i]);
        if(c == EOF) {
            if(templates_i > 0) { fclose(templates[templates_i--]); continue;}
            else break;
        }


        switch(state) {
        case SELECT_END:
            state = START;
            if(c == '\n') break;
            //fallthrough

        case START:
            if((c = match_double(c, '{', templates[templates_i])) == 0) {
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
                case '_': type = SPACEESC; continue;
                case '?': type = ASSERT; continue;
                case '^': type = NOTASSERT; continue;
                case '/': type = ENDBLOCK; continue;
                case '#': type = FOREACH; continue;
                case '~': type = FORELSE; continue;
                case '!': type = EXECUTE; continue;
                case '<': type = INCLUDE; continue;
                case '>': type = SUBTEMPLATE; continue;
                default: type = HTMLESC;
            }
            //fallthrough

        case SELECT:
            if((c = match_double(c, '}', templates[templates_i])) == 0) {
                state = SELECT_END;
                BUF_APPEND(buf, '\0');


                if(type != ENDBLOCK && type != FORELSE && type != SUBTEMPLATE) {
                    char *select, *where, *tbl, *final;
                    parse_sql(buf, &select, &tbl, &where, &final);
                    printf("//GET %s, %s, %s, %s\n",select, tbl, where, final);
                    printf("    val = select(\"");
                    if(type!=FOREACH) print_cstr_esc(select);
                    else printf("rowid");
                    printf("\"");

                    if (tbl) printf(", \"%s\"", tbl);
                    else printf(", tbl");

                    if (where) printf(", \"%s\"", where);
                    else {
                        if(tbl) printf(", NULL");
                        else printf(", where");
                    }

                    if (final) printf(", \"%s\");", final);
                    else printf(", NULL);");
                }

                switch(type) {
                    case HTMLESC: puts("    if(val_ok(val)) print_esc_html(val); done();"); break;
                    case URLESC: puts("    if(val_ok(val)) print_esc_url(val); done();"); break;
                    case SPACEESC: puts("    if(val_ok(val)) print_esc_space(val); done();"); break;
                    case UNESC: puts("    if(val_ok(val)) fputs(val, stdout); done();"); break;
                    case ASSERT: puts("    if(val_ok(val)){done();"); break;
                    case NOTASSERT: puts("    if(!val_ok(val)){done();"); break;
                    case ENDBLOCK: puts("    }done();"); break;
                    case FOREACH:
                        puts("    rows_n = 0;\n"
                             "    while(val) {\n"
                             "        assert(rows_n < ROWS_MAX);\n"
                             "        rows[rows_n++] = strdup(val);\n"
                             "        val = select(NULL, NULL, NULL, NULL);\n"
                             "    }\n"
                             "    done();\n"
                             "    for(int i = 0; i < rows_n; i++){\n"
                             "        char where_buf[BUF_SIZE];\n"
                             "        snprintf(where_buf, BUF_SIZE, \"rowid=%s\", rows[i]);\n"
                             "        char *where = where_buf;\n"
                                );
                        break;
                    case FORELSE:
                        puts("   }if(rows_n == 0){");
                        break;
                    case EXECUTE:
                        puts("  fflush(stdout);system(val); done();");
                        break;
                    case INCLUDE:
                        puts("  file = fopen(val, \"r\"); done();\n"
                             "  if(!file) error(\"Couldn't open file %s\", val);\n"
                             "  while((got = fread(buf, 1, BUF_SIZE, file)))\n"
                             "      while((got -= fwrite(buf, 1, got, stdout)));\n"
                             );
                        break;
                    case SUBTEMPLATE:
                        templates_i++;
                        if(templates_i >= TEMPLATES_MAX) error("Max template inclusion depth exceeded: %i", TEMPLATES_MAX);
                        templates[templates_i] = fopen(buf, "r");
                        if(templates[templates_i] == NULL) error("Couldn't open subtemplate '%s'", buf);
                        break;

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
