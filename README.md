# ctemplate

A templating system that compile mustache style template files to executables via C.

## Basic Usage

Ctemplate coverts mustache like templates in to C source for an executable that
applies that template to an input database. Unlike mustache, which operates on
abstract maps and lists, ctemplate operates using SQL on a relational database.
Table selection and where clause elements can be passed through the command
line.

```sh
> ctemplate mytemplate.mustache
> cc -o mytemplate mytemplate.mustache.c
> mytemplate input.db --where 'id = 1' --table users
Hello Bob (user id 1)
> mytemplate input.db --where 'id = 2' --table users
Hello Sam (user id 2)
```

## Templating

Template tags are surrounded by `{{` `}}`. Available invocations include:

- `sql_query` --- runs given sql query, with 'SELECT' prepended. If a table is
  included on the command line, you do not need to include a FROM clause. Any
  WHERE clause in AND'ed with any command line `--where` arguments, unless
  `--table` is specified and SQL TABLE clause differs from `--table` value. This
  allows the invoker to effectively filter input data.
- `{{sql_query}}` --- inserts the HTML escaped output of the SQL query, or
  nothing if NULL. If more than one row is returned, the compiled template will
  abort with error.
- `{{&sql_query}}` --- inserts the raw un-escaped output of the SQL query, or
  nothing if NULL. If more than one row is returned, the compiled template will
  abort with error.
- `{{%sql_query}}` --- inserts url escaped output of the SQL query, or nothing
  if NULL. If more than one row is returned, the compiled template will abort
      with error.
- `{{?sql_query}} ... {{/}}` --- Inserts code between tags iff SQL query returns
  a non-NULL, non-zero, non-empty string, or not FALSE value.
- `{{^sql_query}} ... {{/}}` --- Inserts code between tags iff SQL query returns
  a NULL, zero, empty string, FALSE value.
- `{{#where_clause}} ... {{~}} ... {{/}}` --- Inserts code once for each row of
  data returned by where clause. FROM clause may be provided, or is otherwise
  inferred from command line. LIMIT, etc clauses may also be used. Command line
  `--where` clauses are AND'ed to produce final query, if tables match.
  Sql_query tags found inside of this block run using this block's WHERE, TABLE,
  etc parameters. Any code after the optional {{^}} tag is only run if there are
  no rows in the resulting query.
- `{{!command sql_query ...}}` --- runs a system command with 0 or more sql
  query arguments. These arguments are replaced with the results the sql query
  values of this row.
- `{{<sql_query}}` --- includes given local file, copies text directly without
  any escaping performed
