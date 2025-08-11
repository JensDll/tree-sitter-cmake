export default grammar({
  name: "cmake",
  extras: () => [],
  externals: (
    $,
  ) => [
    $.unquoted_text,
    $.quoted_text,
    $._variable_open,
    $.normal_variable,
    $.env_variable,
    $.cache_variable,
    $._variable_close,
    $.escape_sequence,
  ],
  rules: {
    source_file: ($) => repeat1(choice(/\s/, $.normal_command)),

    normal_command: ($) =>
      seq($.identifier, "(", optional($.argument_list), ")"),

    argument_list: ($) => repeat1($._argument),

    _argument: ($) =>
      choice(
        /\s/,
        $.unquoted_argument,
        // $.quoted_argument,
        seq("(", optional($.argument_list), ")"),
      ),

    unquoted_argument: ($) =>
      prec.right(
        repeat1(choice($.unquoted_text, $.variable, $.escape_sequence)),
      ),

    quoted_argument: ($) =>
      seq(
        '"',
        repeat(choice(
          $.quoted_text,
          $.variable,
          $.escape_sequence,
          $.quoted_continuation,
        )),
        '"',
      ),

    quoted_continuation: () => "\\\n",

    variable: ($) =>
      seq(
        $._variable_open,
        choice($.normal_variable, $.env_variable, $.cache_variable),
        $._variable_close,
      ),

    identifier: () => /[A-Za-z_][A-Za-z0-9_]*/,
  },
});
