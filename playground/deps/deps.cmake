cpmdeclarepackage(
  fmt
  NAME fmt
  VERSION 11.2.0
  URL https://github.com/fmtlib/fmt/archive/11.2.0.tar.gz
  URL_HASH SHA256=bc23066d87ab3168f27cef3e97d545fa63314f5c79df5ea444d41d56f962c6af
)

# cpmdeclarepackage(
#   tree_sitter
#   NAME tree_sitter
#   VERSION 0.25.8
#   URL https://github.com/tree-sitter/tree-sitter/archive/v0.25.8.tar.gz
#   URL_HASH SHA256=178b575244d967f4920a4642408dc4edf6de96948d37d7f06e5b78acee9c0b4e
#   DOWNLOAD_ONLY ON
# )

cpmdeclarepackage(
  tree_sitter
  NAME tree_sitter
  VERSION latest
  GIT_REPOSITORY https://github.com/tree-sitter/tree-sitter.git
  GIT_TAG 0bb43f7afb5f0d83579007037a13d1fede636dbd
  DOWNLOAD_ONLY ON
)
