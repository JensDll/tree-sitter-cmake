const zero_or_more_horizontal_whitespace = /[^\S\r\n]*/;

export default grammar({
  name: "cmake",
  extras: () => [],
  externals: ($) => [
    $.unquoted_text,
    $.quoted_text,
    $.variable_text,
    $._normal_variable_open,
    $._env_variable_open,
    $._cache_variable_open,
    $._variable_close,
    $._escape_start,
    $._escape_character,
    $.quoted_continuation,
    $.bracket_open,
    $.bracket_content,
    $.bracket_close,
    $._comment_start,
    $._line_comment,
    $.error_sentinel,
  ],
  inline: ($) => [$._variable, $._arguments],
  word: ($) => $.identifier,
  conflicts: ($) => [[$.comment]],
  rules: {
    source_file: ($) => repeat($._statement),

    body: ($) => repeat1($._statement),

    _statement: ($) =>
      choice(
        /\s/,
        $.if_statement,
        $.foreach_statement,
        $.while_statement,
        $.function_statement,
        $.macro_statement,
        $.block_statement,
        $.command,
        $.comment,
      ),

    if_statement: ($) =>
      seq(
        $.if_command,
        optional($.body),
        repeat(seq($.elseif_command, optional($.body))),
        optional(seq($.else_command, optional($.body))),
        $.endif_command,
      ),
    if_command: ($) =>
      seq($.if, zero_or_more_horizontal_whitespace, $._arguments),
    elseif_command: ($) =>
      seq($.elseif, zero_or_more_horizontal_whitespace, $._arguments),
    else_command: ($) =>
      seq($.else, zero_or_more_horizontal_whitespace, $._arguments),
    endif_command: ($) =>
      seq($.endif, zero_or_more_horizontal_whitespace, $._arguments),

    foreach_statement: ($) =>
      seq($.foreach_command, optional($.body), $.endforeach_command),
    foreach_command: ($) =>
      seq($.foreach, zero_or_more_horizontal_whitespace, $._arguments),
    endforeach_command: ($) =>
      seq($.endforeach, zero_or_more_horizontal_whitespace, $._arguments),

    while_statement: ($) =>
      seq($.while_command, optional($.body), $.endwhile_command),
    while_command: ($) =>
      seq($.while, zero_or_more_horizontal_whitespace, $._arguments),
    endwhile_command: ($) =>
      seq($.endwhile, zero_or_more_horizontal_whitespace, $._arguments),

    function_statement: ($) =>
      seq($.function_command, optional($.body), $.endfunction_command),
    function_command: ($) =>
      seq($.function, zero_or_more_horizontal_whitespace, $._arguments),
    endfunction_command: ($) =>
      seq($.endfunction, zero_or_more_horizontal_whitespace, $._arguments),

    macro_statement: ($) =>
      seq($.macro_command, optional($.body), $.endmacro_command),
    macro_command: ($) =>
      seq($.macro, zero_or_more_horizontal_whitespace, $._arguments),
    endmacro_command: ($) =>
      seq($.endmacro, zero_or_more_horizontal_whitespace, $._arguments),

    block_statement: ($) =>
      seq($.block_command, optional($.body), $.endblock_command),
    block_command: ($) =>
      seq($.block, zero_or_more_horizontal_whitespace, $._arguments),
    endblock_command: ($) =>
      seq($.endblock, zero_or_more_horizontal_whitespace, $._arguments),

    command: ($) =>
      seq($.identifier, zero_or_more_horizontal_whitespace, $._arguments),

    _arguments: ($) => seq("(", optional($.argument_list), ")"),
    argument_list: ($) => repeat1($._argument),

    _argument: ($) =>
      choice(
        /\s/,
        $.comment,
        $.unquoted_argument,
        $.quoted_argument,
        $.bracket_argument,
        $._arguments,
      ),

    comment: ($) =>
      seq(
        $._comment_start,
        choice($.bracket_argument, optional($._line_comment)),
      ),

    unquoted_argument: ($) =>
      prec.right(
        repeat1(
          choice(
            alias($.unquoted_text, $.text),
            $._variable,
            $.escape_sequence,
          ),
        ),
      ),

    quoted_argument: ($) => seq('"', optional($.quoted_content), '"'),
    quoted_content: ($) =>
      repeat1(
        choice(
          alias($.quoted_text, $.text),
          $._variable,
          $.escape_sequence,
          $.quoted_continuation,
        ),
      ),

    bracket_argument: ($) =>
      seq($.bracket_open, optional($.bracket_content), $.bracket_close),

    _variable: ($) =>
      choice($.normal_variable, $.env_variable, $.cache_variable),

    normal_variable: ($) =>
      seq(
        $._normal_variable_open,
        optional($.variable_content),
        $._variable_close,
      ),
    env_variable: ($) =>
      seq(
        $._env_variable_open,
        optional($.variable_content),
        $._variable_close,
      ),
    cache_variable: ($) =>
      seq(
        $._cache_variable_open,
        optional($.variable_content),
        $._variable_close,
      ),

    variable_content: ($) =>
      repeat1(
        choice(alias($.variable_text, $.text), $.escape_sequence, $._variable),
      ),

    if: () => /if/i,
    elseif: () => /elseif/i,
    else: () => /else/i,
    endif: () => /endif/i,

    foreach: () => /foreach/i,
    endforeach: () => /endforeach/i,

    while: () => /while/i,
    endwhile: () => /endwhile/i,

    function: () => /function/i,
    endfunction: () => /endfunction/i,

    macro: () => /macro/i,
    endmacro: () => /endmacro/i,

    block: () => /block/i,
    endblock: () => /endblock/i,

    identifier: () => /[A-Za-z_][A-Za-z0-9_]*/,

    escape_sequence: ($) => seq($._escape_start, $._escape_character),
  },
});
