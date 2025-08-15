const zero_or_more_horizontal_whitespace = /[^\S\r\n]*/;

export default grammar({
  name: "cmake",
  extras: () => [],
  externals: (
    $,
  ) => [
    $.unquoted_text,
    $.quoted_text,
    $.variable_text,
    $._normal_variable_open,
    $._env_variable_open,
    $._cache_variable_open,
    $._variable_close,
    $.escape_sequence,
    $.quoted_continuation,
    $.bracket_open,
    $.bracket_content,
    $.bracket_close,
    $.error_sentinel,
  ],
  inline: ($) => [$.variable, $.arguments],
  rules: {
    source_file: ($) => repeat($._statement),

    body: ($) => repeat1($._statement),

    _statement: ($) =>
      choice(
        /\s/,
        $.if,
        $.foreach,
        $.while,
        $.function,
        $.macro,
        $.block,
        $.command,
        $.comment,
      ),

    if: ($) =>
      seq(
        $.if_command,
        optional($.body),
        repeat(seq($.elseif_command, optional($.body))),
        optional(seq($.else_command, optional($.body))),
        $.endif_command,
      ),
    if_command: ($) =>
      seq(/if/i, zero_or_more_horizontal_whitespace, $.arguments),
    elseif_command: ($) =>
      seq(/elseif/i, zero_or_more_horizontal_whitespace, $.arguments),
    else_command: ($) =>
      seq(/else/i, zero_or_more_horizontal_whitespace, $.arguments),
    endif_command: ($) =>
      seq(/endif/i, zero_or_more_horizontal_whitespace, $.arguments),

    foreach: ($) =>
      seq($.foreach_command, optional($.body), $.endforeach_command),
    foreach_command: ($) =>
      seq(/foreach/i, zero_or_more_horizontal_whitespace, $.arguments),
    endforeach_command: ($) =>
      seq(/endforeach/i, zero_or_more_horizontal_whitespace, $.arguments),

    while: ($) => seq($.while_command, optional($.body), $.endwhile_command),
    while_command: ($) =>
      seq(/while/i, zero_or_more_horizontal_whitespace, $.arguments),
    endwhile_command: ($) =>
      seq(/endwhile/i, zero_or_more_horizontal_whitespace, $.arguments),

    function: ($) =>
      seq($.function_command, optional($.body), $.endfunction_command),
    function_command: ($) =>
      seq(/function/i, zero_or_more_horizontal_whitespace, $.arguments),
    endfunction_command: ($) =>
      seq(/endfunction/i, zero_or_more_horizontal_whitespace, $.arguments),

    macro: ($) => seq($.macro_command, optional($.body), $.endmacro_command),
    macro_command: ($) =>
      seq(/macro/i, zero_or_more_horizontal_whitespace, $.arguments),
    endmacro_command: ($) =>
      seq(/endmacro/i, zero_or_more_horizontal_whitespace, $.arguments),

    block: ($) => seq($.block_command, optional($.body), $.endblock_command),
    block_command: ($) =>
      seq(/block/i, zero_or_more_horizontal_whitespace, $.arguments),
    endblock_command: ($) =>
      seq(/endblock/i, zero_or_more_horizontal_whitespace, $.arguments),

    command: ($) =>
      seq($.identifier, zero_or_more_horizontal_whitespace, $.arguments),

    arguments: ($) => seq("(", optional($.argument_list), ")"),
    argument_list: ($) => repeat1($._argument),
    _argument: ($) =>
      choice(
        /\s/,
        $.comment,
        $.unquoted_argument,
        $.quoted_argument,
        $.bracket_arguemnt,
        $.arguments,
      ),

    unquoted_argument: ($) =>
      prec.right(
        repeat1(
          choice(alias($.unquoted_text, $.text), $.variable, $.escape_sequence),
        ),
      ),

    quoted_argument: ($) =>
      seq(
        '"',
        repeat(choice(
          alias($.quoted_text, $.text),
          $.variable,
          $.escape_sequence,
          $.quoted_continuation,
        )),
        '"',
      ),

    bracket_arguemnt: ($) =>
      seq($.bracket_open, optional($.bracket_content), $.bracket_close),

    variable: ($) =>
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
        choice(alias($.variable_text, $.text), $.escape_sequence, $.variable),
      ),

    identifier: () => /[A-Za-z_][A-Za-z0-9_]*/,

    comment: ($) => choice(seq("#", choice($.bracket_arguemnt, /[^\r\n]*/))),
  },
});
