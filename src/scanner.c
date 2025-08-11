#include "tree_sitter/alloc.h"
#include "tree_sitter/parser.h"

#include <ctype.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

enum TokenType {
  UNQUOTED_TEXT,
  QUOTED_TEXT,
  VARIABLE_OPEN,
  NORMAL_VARIABLE,
  ENV_VARIABLE,
  CACHE_VARIABLE,
  VARIABLE_CLOSE,
  ESCAPE_SEQUENCE,
  ERROR_SENTINEL
};

enum StateMachine {
  MATCH_TEXT,
  MATCH_VARIABLE,
  MATCH_CA,
  MATCH_CAC,
  MATCH_CACH,
  MATCH_CACHE,
  MATCH_EN,
  MATCH_ENV,
  MATCH_VARIABLE_OPEN,
  MATCH_NORMAL_VARIABLE,
  MATCH_CACHE_VARIABLE,
  MATCH_ENV_VARIABLE,
  MATCH_VARIABLE_CLOSE,
  ACCEPT_VARIABLE_OPEN
};

typedef enum StateMachine StateMachine;

struct State
{
  unsigned int next_state;
};

typedef struct State State;

#define STATE_SIZE sizeof(State)

#define ADVANCE(state_value) \
  {                          \
    state = state_value;     \
    goto next_state;         \
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
  if (length) {
    memcpy(payload, buffer, STATE_SIZE);
  } else {
    memset(payload, 0, STATE_SIZE);
  }
}

static bool is_variable(TSLexer* const lexer)
{
  StateMachine state;

  if (lexer->lookahead != '$') {
    return false;
  }

  if (lexer->lookahead == 'C') {
    ADVANCE(MATCH_CA)
  }

  if (lexer->lookahead == 'E') {
    ADVANCE(MATCH_EN)
  }

  lexer->advance(lexer, false);
  return lexer->lookahead == '{';

next_state:
  lexer->advance(lexer, false);

  switch (state) {
  case MATCH_CA:
    if (lexer->lookahead == 'A') {
      ADVANCE(MATCH_CAC)
    }
    return false;
  case MATCH_CAC:
    if (lexer->lookahead == 'C') {
      ADVANCE(MATCH_CACH);
    }
    return false;
  case MATCH_CACH:
    if (lexer->lookahead == 'H') {
      ADVANCE(MATCH_CACHE);
    }
    return false;
  case MATCH_CACHE:
    if (lexer->lookahead == 'E') {
      ADVANCE(MATCH_VARIABLE_OPEN);
    }
    return false;
  case MATCH_EN:
    if (lexer->lookahead == 'N') {
      ADVANCE(MATCH_ENV)
    }
    return false;
  case MATCH_ENV:
    if (lexer->lookahead == 'V') {
      ADVANCE(MATCH_VARIABLE_OPEN)
    }
    return false;
  case MATCH_VARIABLE_OPEN:
    return lexer->lookahead == '{';
  default:
    return false;
  }
}

static bool scan_unqouted_text(TSLexer* const lexer, State* const payload)
{
  lexer->result_symbol = UNQUOTED_TEXT;

  while (!lexer->eof(lexer)) {
    lexer->mark_end(lexer);

    if (is_variable(lexer)) {
      payload->next_state = MATCH_TEXT;
      return true;
    }

    if (isspace(lexer->lookahead) || lexer->lookahead == '(' || lexer->lookahead == ')') {
      return true;
    }

    lexer->advance(lexer, false);
  }

  return false;
}

static bool scan_unquoted_argument(TSLexer* const lexer, State* const payload)
{
  StateMachine state = payload->next_state;
  goto start;

next_state:
  lexer->advance(lexer, false);

start:
  switch (state) {
  case MATCH_TEXT:
    if (lexer->lookahead == '$') {
      ADVANCE(MATCH_VARIABLE);
    }
    if (isspace(lexer->lookahead) || lexer->lookahead == '"' || lexer->lookahead == '(' || lexer->lookahead == ')') {
      return false;
    }
    lexer->advance(lexer, false);
    return scan_unqouted_text(lexer, payload);
  case MATCH_VARIABLE:
    if (lexer->lookahead == 'C') {
      ADVANCE(MATCH_CA)
    }
    if (lexer->lookahead == 'E') {
      ADVANCE(MATCH_EN)
    }
    payload->next_state = MATCH_NORMAL_VARIABLE;
    __attribute__((fallthrough));
  case MATCH_VARIABLE_OPEN:
    if (lexer->lookahead == '{') {
      lexer->advance(lexer, false);
      lexer->result_symbol = VARIABLE_OPEN;
      return true;
    }
    return scan_unqouted_text(lexer, payload);
  case MATCH_CA:
    if (lexer->lookahead == 'A') {
      ADVANCE(MATCH_CAC)
    }
    return scan_unqouted_text(lexer, payload);
  case MATCH_CAC:
    if (lexer->lookahead == 'C') {
      ADVANCE(MATCH_CACH);
    }
    return scan_unqouted_text(lexer, payload);
  case MATCH_CACH:
    if (lexer->lookahead == 'H') {
      ADVANCE(MATCH_CACHE);
    }
    return scan_unqouted_text(lexer, payload);
  case MATCH_CACHE:
    if (lexer->lookahead == 'E') {
      payload->next_state = MATCH_CACHE_VARIABLE;
      ADVANCE(MATCH_VARIABLE_OPEN);
    }
    return scan_unqouted_text(lexer, payload);
  case MATCH_EN:
    if (lexer->lookahead == 'N') {
      ADVANCE(MATCH_ENV)
    }
    return scan_unqouted_text(lexer, payload);
  case MATCH_ENV:
    if (lexer->lookahead == 'V') {
      payload->next_state = MATCH_ENV_VARIABLE;
      ADVANCE(MATCH_VARIABLE_OPEN)
    }
    return scan_unqouted_text(lexer, payload);
  case MATCH_NORMAL_VARIABLE:
    if (lexer->lookahead == '}') {
      return false;
    }

    while (!lexer->eof(lexer)) {
      lexer->advance(lexer, false);
      if (lexer->lookahead == '}') {
        lexer->result_symbol = NORMAL_VARIABLE;
        return true;
      }
    }

    return false;
  default:
    return false;
  }
}

bool tree_sitter_cmake_external_scanner_scan(void* const payload, TSLexer* const lexer, const bool* const valid_symbols)
{
  State* const state = payload;

  if (valid_symbols[VARIABLE_CLOSE]) {
    state->next_state = MATCH_TEXT;
    lexer->advance(lexer, false);
    lexer->result_symbol = VARIABLE_CLOSE;
    return true;
  }

  if (valid_symbols[UNQUOTED_TEXT] || valid_symbols[NORMAL_VARIABLE]) {
    return scan_unquoted_argument(lexer, state);
  }

  return false;
}
