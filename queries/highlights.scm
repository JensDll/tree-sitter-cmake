[
  (block)
  (endblock)
] @keyword

[
  (if)
  (elseif)
  (else)
  (endif)
] @keyword.conditional

[
  (function)
  (endfunction)
  (macro)
  (endmacro)
] @keyword.function

[
  (foreach)
  (endforeach)
  (while)
  (endwhile)
] @keyword.repeat

(comment) @comment @spell

[
  (normal_variable)
  (env_variable)
  (cache_variable)
] @punctuation.special

(variable_content) @variable

(
  (variable_content) @variable.builtin
  (#match? @variable.builtin "^[A-Z_][A-Z0-9_]*$"))

[
  "("
  ")"
] @punctuation.bracket

(escape_sequence) @string.escape
(quoted_continuation) @string.escape

(argument_list
  (quoted_argument) @string)

(argument_list
  (bracket_argument) @string)

(
  (unquoted_argument) @boolean
  (#match? @boolean "^([oO][nN]|[yY][eE][sS]|[tT][rR][uU][eE]|[yY]|[oO][fF][fF]|[nN][oO]|[fF][aA][lL][sS][eE]|[nN]|[iI][gG][nN][oO][rR][eE]|[nN][oO][tT][fF][oO][uU][nN][dD]|.*-[nN][oO][tT][fF][oO][uU][nN][dD])$"))

(command
  (identifier) @function)

(command
  (identifier) @keyword.return
  (#match? @keyword.return "^[rR][eE][tT][uU][rR][nN]$"))

(command
  (identifier) @function
  (argument_list
    .
    (unquoted_argument) @keyword.operator
    (#any-of? @keyword.operator
     "FATAL_ERROR" "SEND_ERROR" "WARNING" "AUTHOR_WARNING" "DEPRECATION"
     "NOTICE" "STATUS" "VERBOSE" "DEBUG" "TRACE"
     "CHECK_START" "CHECK_PASS" "CHECK_FAIL" "CONFIGURE_LOG"))
  (#match? @function "^[mM][eE][sS][sS][aA][gG][eE]$"))

(command
  (identifier) @function
  (argument_list
    .
    [
      (unquoted_argument) @variable
      (bracket_argument
        (bracket_content) @variable)
      (quoted_argument
        (quoted_content) @variable)
    ])
  (#match? @function "^[sS][eE][tT]$"))

(if_command
  (if)
  (argument_list
    (unquoted_argument) @keyword.operator
    (#any-of? @keyword.operator
     "COMMAND" "POLICY" "TARGET" "TEST" "EXISTS" "IS_READABLE" "IS_WRITABLE" "IS_EXECUTABLE"
     "IS_DIRECTORY" "IS_SYMLINK" "IS_ABSOLUTE" "DEFINED" "EQUAL" "LESS" "LESS_EQUAL" "GREATER"
     "GREATER_EQUAL" "STREQUAL" "STRLESS" "STRLESS_EQUAL" "STRGREATER" "STRGREATER_EQUAL" "VERSION_EQUAL"
     "VERSION_LESS" "VERSION_LESS_EQUAL" "VERSION_GREATER" "VERSION_GREATER_EQUAL" "PATH_EQUAL" "IN_LIST"
     "IS_NEWER_THAN" "MATCHES" "NOT" "AND" "OR")))

(elseif_command
  (elseif)
  (argument_list
    (unquoted_argument) @keyword.operator
    (#any-of? @keyword.operator
     "COMMAND" "POLICY" "TARGET" "TEST" "EXISTS" "IS_READABLE" "IS_WRITABLE" "IS_EXECUTABLE"
     "IS_DIRECTORY" "IS_SYMLINK" "IS_ABSOLUTE" "DEFINED" "EQUAL" "LESS" "LESS_EQUAL" "GREATER"
     "GREATER_EQUAL" "STREQUAL" "STRLESS" "STRLESS_EQUAL" "STRGREATER" "STRGREATER_EQUAL" "VERSION_EQUAL"
     "VERSION_LESS" "VERSION_LESS_EQUAL" "VERSION_GREATER" "VERSION_GREATER_EQUAL" "PATH_EQUAL" "IN_LIST"
     "IS_NEWER_THAN" "MATCHES" "NOT" "AND" "OR")))

(function_command
  (function)
  (argument_list
    .
    [
      (unquoted_argument) @function
      (bracket_argument
        (bracket_content) @function)
      (quoted_argument
        (quoted_content) @function)
    ]
    [
      (unquoted_argument) @variable.parameter
      (bracket_argument
        (bracket_content) @variable.parameter)
      (quoted_argument
        (quoted_content) @variable.parameter)
    ]*))

(macro_command
  (macro)
  (argument_list
    .
    [
      (unquoted_argument) @function
      (bracket_argument
        (bracket_content) @function)
      (quoted_argument
        (quoted_content) @function)
    ]
    [
      (unquoted_argument) @variable.parameter
      (bracket_argument
        (bracket_content) @variable.parameter)
      (quoted_argument
        (quoted_content) @variable.parameter)
    ]*))
