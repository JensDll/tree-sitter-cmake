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
  inline: ($) => [$.variable],
  rules: {
    source_file: ($) => repeat1(choice(/\s/, $.normal_command)),

    normal_command: ($) =>
      seq($.identifier, "(", optional($.argument_list), ")"),

    argument_list: ($) => repeat1($._argument),

    _argument: ($) =>
      choice(
        /\s/,
        $.comment,
        $.unquoted_argument,
        $.quoted_argument,
        $.bracket_arguemnt,
        seq("(", optional($.argument_list), ")"),
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

    comment: ($) => choice(seq("#", choice($.bracket_arguemnt, /[^\n]*/))),
  },
});
