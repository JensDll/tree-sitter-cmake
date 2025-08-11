export default grammar({
  name: "cmake",
  extras: () => [],
  externals: ($) => [$.unquoted_text, $.quoted_text],
  rules: {
    source_file: ($) => repeat(choice(/\s/, $.normal_command)),

    normal_command: ($) =>
      seq($.identifier, "(", optional($.argument_list), ")"),

    argument_list: ($) => repeat1($._argument),

    _argument: ($) =>
      choice(
        /\s/,
        $.unquoted_argument,
        $.quoted_argument,
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
      choice($.normal_variable, $.env_variable, $.cache_variable),
    normal_variable: ($) => seq("${", optional($.variable_content), "}"),
    env_variable: ($) => seq("$ENV{", optional($.variable_content), "}"),
    cache_variable: ($) => seq("$CACHE{", optional($.variable_content), "}"),
    variable_content: ($) =>
      repeat1(choice(/[a-zA-Z0-9/_.+-]/, $.escape_sequence, $.variable)),

    escape_sequence: () =>
      choice("\\t", "\\r", "\\n", "\\;", /\\[^A-Za-z0-9;]/),

    identifier: () => /[A-Za-z_][A-Za-z0-9_]*/,
  },
});
