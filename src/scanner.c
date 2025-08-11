#include "tree_sitter/alloc.h"
#include "tree_sitter/parser.h"

#include <ctype.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

enum TokenType { UNQUOTED_TEXT, QUOTED_TEXT };

struct State
{
  bool matching_unquoted;
  bool matching_quoted;
};

typedef struct State State;

#define STATE_SIZE sizeof(State)

#define MATCH_VARIABLE 0
#define MATCH_CA 1
#define MATCH_CAC 2
#define MATCH_CACH 3
#define MATCH_CACHE 4
#define MATCH_EN 5
#define MATCH_ENV 6
#define MATCH_VARIABLE_OPEN 7

static bool is_variable(TSLexer* const lexer)
{
  int lookahead = lexer->lookahead;

  if (lookahead != '$') {
    return false;
  }

  unsigned int state = MATCH_VARIABLE;

next_state:
  lexer->advance(lexer, false);
  lookahead = lexer->lookahead;

  switch (state) {
  case MATCH_VARIABLE:
    if (lookahead == 'C') {
      ADVANCE(MATCH_CA)
    }
    if (lookahead == 'E') {
      ADVANCE(MATCH_EN)
    }
    return lookahead == '{';
  case MATCH_CA:
    if (lookahead == 'A') {
      ADVANCE(MATCH_CAC)
    }
    return false;
  case MATCH_CAC:
    if (lookahead == 'C') {
      ADVANCE(MATCH_CACH);
    }
    return false;
  case MATCH_CACH:
    if (lookahead == 'H') {
      ADVANCE(MATCH_CACHE);
    }
    return false;
  case MATCH_CACHE:
    if (lookahead == 'E') {
      ADVANCE(MATCH_VARIABLE_OPEN);
    }
    return false;
  case MATCH_EN:
    if (lookahead == 'N') {
      ADVANCE(MATCH_ENV)
    }
    return false;
  case MATCH_ENV:
    if (lookahead == 'V') {
      ADVANCE(MATCH_VARIABLE_OPEN)
    }
    return false;
  case MATCH_VARIABLE_OPEN:
    return lookahead == '{';
  default:
    return false;
  }
}

static bool scan_quoted_text(TSLexer* const lexer, State* const state)
{
  if (lexer->lookahead == '"' || lexer->lookahead == '\\' || is_variable(lexer) || lexer->eof(lexer)) {
    return false;
  }

  lexer->mark_end(lexer);
  lexer->result_symbol = QUOTED_TEXT;

  while (lexer->lookahead != '"' && lexer->lookahead != '\\' && !is_variable(lexer) && !lexer->eof(lexer)) {
    lexer->advance(lexer, false);
    lexer->mark_end(lexer);
    lexer->result_symbol = QUOTED_TEXT;
  }

  return true;
}

static bool scan_unquoted_text(TSLexer* const lexer, State* const state)
{
  if (isspace(lexer->lookahead) || lexer->lookahead == '"' || lexer->lookahead == '\\' || lexer->lookahead == '('
      || lexer->lookahead == ')' || is_variable(lexer) || lexer->eof(lexer)) {
    return false;
  }

  lexer->mark_end(lexer);
  lexer->result_symbol = UNQUOTED_TEXT;

  while (!isspace(lexer->lookahead) && lexer->lookahead != '\\' && lexer->lookahead != '(' && lexer->lookahead != ')'
         && !is_variable(lexer) && !lexer->eof(lexer)) {
    lexer->advance(lexer, false);
    lexer->mark_end(lexer);
    lexer->result_symbol = UNQUOTED_TEXT;
  }

  return true;
}

void* tree_sitter_cmake_external_scanner_create(void)
{
  return ts_malloc(STATE_SIZE);
}

void tree_sitter_cmake_external_scanner_destroy(void* const payload)
{
  ts_free(payload);
}

unsigned int tree_sitter_cmake_external_scanner_serialize(const void* const payload, char* const buffer)
{
  memcpy(buffer, payload, STATE_SIZE);
  return STATE_SIZE;
}

void tree_sitter_cmake_external_scanner_deserialize(void* const payload,
  const char* const buffer,
  const unsigned length)
{
  memcpy(payload, buffer, length);
}

bool tree_sitter_cmake_external_scanner_scan(void* const payload, TSLexer* const lexer, const bool* const valid_symbols)
{
  State* state = payload;

  if (valid_symbols[UNQUOTED_TEXT]) {
    return scan_unquoted_text(lexer, state);
  }

  if (valid_symbols[QUOTED_TEXT]) {
    return scan_quoted_text(lexer, state);
  }

  return false;
}
