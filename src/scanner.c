#include "tree_sitter/alloc.h"
#include "tree_sitter/parser.h"

#include <assert.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

typedef enum TokenType {
  UNQUOTED_TEXT,
  QUOTED_TEXT,
  VARIABLE_TEXT,
  NORMAL_VARIABLE_OPEN,
  ENV_VARIABLE_OPEN,
  CACHE_VARIABLE_OPEN,
  VARIABLE_CLOSE,
  ESCAPE_SEQUENCE,
  QUOTED_CONTINUATION,
  BRACKET_OPEN,
  BRACKET_CONTENT,
  BRACKET_CLOSE,
  ERROR_SENTINEL
} TokenType;

typedef struct State
{
  unsigned short open_count;
  bool unquoted_argument_active;
} State;

#define STATE_SIZE sizeof(State)

#define NEXT_STATE(state) \
  next_state = state;     \
  goto next_state;

#if __GNUC__ || __clang__
#define ATTRIBUTE_UNUSED __attribute__((unused))
#define ATTRIBUTE_FALLTROUGH __attribute__((fallthrough));
#else
#define ATTRIBUTE_UNUSED /**/
#define ATTRIBUTE_FALLTROUGH /**/
#endif

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
  const unsigned int length)
{
  if (length) {
    memcpy(payload, buffer, STATE_SIZE);
  } else {
    memset(payload, 0, STATE_SIZE);
  }
}

static bool is_bracket_open(TSLexer* const lexer, unsigned int* const open_count)
{
  *open_count = 0;

  if (lexer->lookahead != '[') {
    return false;
  }

  lexer->advance(lexer, false);

  while (lexer->lookahead == '=') {
    lexer->advance(lexer, false);
    ++*open_count;
  }

  return lexer->lookahead == '[';
}

static bool is_bracket_close(TSLexer* const lexer, const unsigned int open_count)
{
  if (lexer->lookahead != ']') {
    return false;
  }

  lexer->advance(lexer, false);

  unsigned int close_count = 0;

  while (lexer->lookahead == '=') {
    lexer->advance(lexer, false);
    ++close_count;
  }

  return lexer->lookahead == ']' && open_count == close_count;
}

static bool is_variable(TSLexer* const lexer, TSSymbol* const open_type)
{
  enum StateMachine {
    MATCH_VARIABLE,
    MATCH_VARIABLE_OPEN,
    MATCH_CA,
    MATCH_CAC,
    MATCH_CACH,
    MATCH_CACHE,
    MATCH_EN,
    MATCH_ENV,
  };

  *open_type = NORMAL_VARIABLE_OPEN;
  enum StateMachine next_state = MATCH_VARIABLE;

  if (lexer->lookahead != '$') {
    return false;
  }

next_state:
  lexer->advance(lexer, false);

  switch (next_state) {
  case MATCH_VARIABLE:
    if (lexer->lookahead == 'E') {
      NEXT_STATE(MATCH_EN)
    }
    if (lexer->lookahead == 'C') {
      NEXT_STATE(MATCH_CA)
    }
    ATTRIBUTE_FALLTROUGH
  case MATCH_VARIABLE_OPEN:
    return lexer->lookahead == '{';
  case MATCH_CA:
    *open_type = CACHE_VARIABLE_OPEN;
    if (lexer->lookahead == 'A') {
      NEXT_STATE(MATCH_CAC)
    }
    return false;
  case MATCH_CAC:
    if (lexer->lookahead == 'C') {
      NEXT_STATE(MATCH_CACH);
    }
    return false;
  case MATCH_CACH:
    if (lexer->lookahead == 'H') {
      NEXT_STATE(MATCH_CACHE);
    }
    return false;
  case MATCH_CACHE:
    if (lexer->lookahead == 'E') {
      NEXT_STATE(MATCH_VARIABLE_OPEN);
    }
    return false;
  case MATCH_EN:
    *open_type = ENV_VARIABLE_OPEN;
    if (lexer->lookahead == 'N') {
      NEXT_STATE(MATCH_ENV)
    }
    return false;
  case MATCH_ENV:
    if (lexer->lookahead == 'V') {
      NEXT_STATE(MATCH_VARIABLE_OPEN)
    }
    return false;
  default:
    return false;
  }
}

static bool is_quoted_text(TSLexer* const lexer)
{
  TSSymbol open_type = 0;
  return lexer->lookahead != '\\' && lexer->lookahead != '"' && !is_variable(lexer, &open_type);
}

static bool is_argument_separator(const TSLexer* const lexer)
{
  return isspace(lexer->lookahead) || lexer->lookahead == '(' || lexer->lookahead == ')' || lexer->lookahead == '#';
}

static bool is_unqouted_text(TSLexer* const lexer)
{
  TSSymbol open_type = 0;
  return !is_argument_separator(lexer) && lexer->lookahead != '\\' && !is_variable(lexer, &open_type);
}

static bool is_escape_character(const TSLexer* const lexer)
{
  return lexer->lookahead == 't' || lexer->lookahead == 'r' || lexer->lookahead == 'n' || lexer->lookahead == ';'
         || !isalnum(lexer->lookahead);
}

static bool is_variable_text(const TSLexer* const lexer)
{
  return isalnum(lexer->lookahead) || lexer->lookahead == '/' || lexer->lookahead == '_' || lexer->lookahead == '.'
         || lexer->lookahead == '+' || lexer->lookahead == '-';
}

static bool scan_bracket_content(TSLexer* const lexer, State* const state)
{
  lexer->result_symbol = BRACKET_CLOSE;

  if (is_bracket_close(lexer, state->open_count)) {
    lexer->advance(lexer, true);
    return true;
  }

  lexer->result_symbol = BRACKET_CONTENT;
  lexer->mark_end(lexer);

  while (!lexer->eof(lexer) && !is_bracket_close(lexer, state->open_count)) {
    lexer->advance(lexer, false);
    lexer->mark_end(lexer);
  }

  return !lexer->eof(lexer);
}

static bool scan_variable_content(TSLexer* const lexer, State* const state)
{
  if (lexer->lookahead == '}') {
    lexer->result_symbol = VARIABLE_CLOSE;
    lexer->advance(lexer, false);
    state->unquoted_argument_active = state->unquoted_argument_active && !is_argument_separator(lexer);
    return true;
  }

  if (lexer->lookahead == '\\') {
    lexer->advance(lexer, false);

    if (is_escape_character(lexer)) {
      lexer->advance(lexer, false);
      lexer->result_symbol = ESCAPE_SEQUENCE;
      return true;
    }

    return false;
  }

  TSSymbol open_type = 0;
  if (is_variable(lexer, &open_type)) {
    lexer->advance(lexer, false);
    lexer->result_symbol = open_type;
    return true;
  }

  while (!lexer->eof(lexer) && is_variable_text(lexer)) {
    lexer->advance(lexer, false);
  }

  lexer->result_symbol = VARIABLE_TEXT;

  return !lexer->eof(lexer);
}

static bool scan_bracket_open(TSLexer* const lexer, State* const state)
{
  unsigned int open_count = 0;

  if (is_bracket_open(lexer, &open_count)) {
    lexer->advance(lexer, false);
    lexer->result_symbol = BRACKET_OPEN;
    state->open_count = (unsigned short)open_count;
    return true;
  }

  return false;
}

static bool scan_unquoted_argument(TSLexer* const lexer, State* const state)
{
  if (is_argument_separator(lexer)) {
    return false;
  }

  if (!state->unquoted_argument_active) {
    if (lexer->lookahead == '"') {
      return false;
    }

    if (scan_bracket_open(lexer, state)) {
      return true;
    }
  }

  state->unquoted_argument_active = true;

  if (lexer->lookahead == '\\') {
    lexer->advance(lexer, false);

    if (is_escape_character(lexer)) {
      lexer->advance(lexer, false);
      lexer->result_symbol = ESCAPE_SEQUENCE;
      state->unquoted_argument_active = !is_argument_separator(lexer);
      return true;
    }

    return false;
  }

  TSSymbol open_type = 0;
  if (is_variable(lexer, &open_type)) {
    lexer->advance(lexer, false);
    lexer->result_symbol = open_type;
    return true;
  }

  while (!lexer->eof(lexer) && is_unqouted_text(lexer)) {
    lexer->advance(lexer, false);
    lexer->mark_end(lexer);
  }

  lexer->result_symbol = UNQUOTED_TEXT;
  state->unquoted_argument_active = false;

  return !lexer->eof(lexer);
}

static bool scan_quoted_argument(TSLexer* const lexer)
{
  if (lexer->lookahead == '"') {
    return false;
  }

  if (lexer->lookahead == '\\') {
    lexer->advance(lexer, false);

    if (lexer->lookahead == '\n') {
      lexer->result_symbol = QUOTED_CONTINUATION;
      lexer->advance(lexer, false);
      return true;
    }

    if (is_escape_character(lexer)) {
      lexer->advance(lexer, false);
      lexer->result_symbol = ESCAPE_SEQUENCE;
      return true;
    }

    return false;
  }

  TSSymbol open_type = 0;
  if (is_variable(lexer, &open_type)) {
    lexer->advance(lexer, false);
    lexer->result_symbol = open_type;
    return true;
  }

  while (!lexer->eof(lexer) && is_quoted_text(lexer)) {
    lexer->advance(lexer, false);
    lexer->mark_end(lexer);
  }

  lexer->result_symbol = QUOTED_TEXT;

  return !lexer->eof(lexer);
}

bool tree_sitter_cmake_external_scanner_scan(void* const payload, TSLexer* const lexer, const bool* const valid_symbols)
{
  if (valid_symbols[ERROR_SENTINEL]) {
    return false;
  }

  if (valid_symbols[UNQUOTED_TEXT]) {
    return scan_unquoted_argument(lexer, payload);
  }

  if (valid_symbols[QUOTED_TEXT]) {
    return scan_quoted_argument(lexer);
  }

  if (valid_symbols[BRACKET_CLOSE]) {
    return scan_bracket_content(lexer, payload);
  }

  if (valid_symbols[VARIABLE_TEXT]) {
    return scan_variable_content(lexer, payload);
  }

  assert(valid_symbols[BRACKET_OPEN]);

  return scan_bracket_open(lexer, payload);
}
