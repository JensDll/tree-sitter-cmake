#include "tree_sitter/alloc.h"
#include "tree_sitter/parser.h"

#include <ctype.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

enum TokenType { UNQUOTED_TEXT, QUOTED_TEXT };

struct State
{
  int32_t last_unquoted_match;
  bool did_match_variable_in_quoted_text;
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

// static bool scan_bracket_content(TSLexer* const lexer)
// {
//   unsigned int open_count = 0;
//
//   while (lexer->lookahead == '=') {
//     ++open_count;
//     lexer->advance(lexer, false);
//   }
//
//   if (lexer->lookahead != '[') {
//     return false;
//   }
//
//   lexer->advance(lexer, false);
//
//   while (!lexer->eof(lexer)) {
//     const bool not_closing = lexer->lookahead != ']';
//
//     lexer->advance(lexer, false);
//
//     if (not_closing) {
//       continue;
//     }
//
//     unsigned int close_count = 0;
//
//     while (lexer->lookahead == '=') {
//       ++close_count;
//       lexer->advance(lexer, false);
//     }
//
//     if (lexer->lookahead == ']' && close_count == open_count) {
//       lexer->advance(lexer, false);
//       lexer->mark_end(lexer);
//       lexer->result_symbol = BRACKET_CONTENT;
//       return true;
//     }
//   }
//
//   return false;
// }

static bool scan_quoted_text(TSLexer* const lexer)
{
  unsigned int state;
  int lookahead = lexer->lookahead;
  bool result = false;

  if (lookahead == '"' || lookahead == '\\') {
    END_STATE();
  }

  if (lookahead == '$') {
    ADVANCE(MATCH_VARIABLE)
  }

  lexer->advance(lexer, false);
  lookahead = lexer->lookahead;

match_text:
  ACCEPT_TOKEN(QUOTED_TEXT);

  if (lexer->eof(lexer) || lookahead == '"' || lookahead == '\\') {
    END_STATE()
  }

  if (lookahead == '$') {
    ADVANCE(MATCH_VARIABLE)
  }

  lexer->advance(lexer, false);
  lookahead = lexer->lookahead;

  goto match_text;

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
    if (lookahead == '{') {
      END_STATE();
    }
    goto match_text;
  case MATCH_CA:
    if (lookahead == 'A') {
      ADVANCE(MATCH_CAC)
    }
    goto match_text;
  case MATCH_CAC:
    if (lookahead == 'C') {
      ADVANCE(MATCH_CACH);
    }
    goto match_text;
  case MATCH_CACH:
    if (lookahead == 'H') {
      ADVANCE(MATCH_CACHE);
    }
    goto match_text;
  case MATCH_CACHE:
    if (lookahead == 'E') {
      ADVANCE(MATCH_VARIABLE_OPEN);
    }
    goto match_text;
  case MATCH_EN:
    if (lookahead == 'N') {
      ADVANCE(MATCH_ENV)
    }
    goto match_text;
  case MATCH_ENV:
    if (lookahead == 'V') {
      ADVANCE(MATCH_VARIABLE_OPEN)
    }
    goto match_text;
  case MATCH_VARIABLE_OPEN:
    if (lookahead == '{') {
      END_STATE()
    }
    goto match_text;
  default:
    return false;
  }
}

static bool scan_unquoted_text(TSLexer* const lexer, State* const state)
{
  if (lexer->lookahead == state->last_unquoted_match) {
    return false;
  }

  for (; !lexer->eof(lexer); lexer->advance(lexer, false)) {
    if (isspace(lexer->lookahead)) {
      lexer->mark_end(lexer);
      lexer->result_symbol = UNQUOTED_TEXT;
      state->last_unquoted_match = lexer->lookahead;
      return true;
    }

    switch (lexer->lookahead) {
    case '(':
    case ')':
    case '\\':
      lexer->mark_end(lexer);
      lexer->result_symbol = UNQUOTED_TEXT;
      state->last_unquoted_match = lexer->lookahead;
      return true;
    case '$':
      lexer->mark_end(lexer);
      lexer->advance(lexer, false);
      if (lexer->lookahead == '{') {
        lexer->result_symbol = UNQUOTED_TEXT;
        state->last_unquoted_match = '$';
        return true;
      }
    }
  }

  return false;
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

bool tree_sitter_cmake_external_scanner_scan(void* const payload, TSLexer* const lexer, const bool* const valid_symbols)
{
  if (valid_symbols[UNQUOTED_TEXT]) {
    if (isspace(lexer->lookahead) || lexer->lookahead == '(' || lexer->lookahead == ')' || lexer->lookahead == '"') {
      return false;
    }
    return scan_unquoted_text(lexer, payload);
  }

  if (valid_symbols[QUOTED_TEXT]) {
    return scan_quoted_text(lexer);
  }

  return false;
}
