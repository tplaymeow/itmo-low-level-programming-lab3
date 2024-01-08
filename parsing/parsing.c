#include "parsing.h"
#include "generated/sql_lexer.h"
#include "generated/sql_parser.h"

struct parsing_result parse(const char *input) {
  yy_scan_string(input);

  struct sql_statement statement;
  const int parser_result = yyparse(&statement);
  if (parser_result != 0) {
    yylex_destroy();
    return (struct parsing_result){.status = PARSING_STATUS_ERROR};
  }

  yylex_destroy();
  return (struct parsing_result){.status = PARSING_STATUS_OK,
                                 .value = statement};
}