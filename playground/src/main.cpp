#include <fmt/base.h>
#include <fmt/format.h>
#include <tree_sitter/api.h>
#include <tree_sitter/tree-sitter-cmake.h>

#include <string>
#include <string_view>

std::string format_as(const TSLogType log_type)
{
  return log_type == TSLogType::TSLogTypeParse ? "Parse" : "Lex";
}

int main()
{
  TSParser* const parser{ ts_parser_new() };

  ts_parser_set_language(parser, tree_sitter_cmake());
  ts_parser_set_logger(
    parser, { .payload = nullptr, .log = [](void* const, const TSLogType log_type, const char* buffer) {
               fmt::println("[{}] {}", log_type, std::string_view(buffer));
             } });

  static constexpr std::string_view source_code{ "list(\"^=.*$\")" };

  TSTree* const tree = ts_parser_parse_string(parser, nullptr, source_code.data(), source_code.size());

  ts_tree_delete(tree);
  ts_parser_delete(parser);
}
