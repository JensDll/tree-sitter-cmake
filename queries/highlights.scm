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

(comment) @comment

[
  (normal_variable)
  (env_variable)
  (cache_variable)
] @punctuation.special

(variable_content) @variable

[
  "("
  ")"
] @punctuation.bracket

(escape_sequence) @string.escape

(argument_list
  (quoted_argument) @string)

(argument_list
  (bracket_argument) @string)

((unquoted_argument) @boolean
  (#match? @boolean "^(1|[oO][nN]|[yY][eE][sS]|[tT][rR][uU][eE]|[yY]|0|[oO][fF][fF]|[nN][oO]|[fF][aA][lL][sS][eE]|[nN]|[iI][gG][nN][oO][rR][eE]|[nN][oO][tT][fF][oO][uU][nN][dD]|.*-[nN][oO][tT][fF][oO][uU][nN][dD])$"))

(command
  (identifier) @function)
