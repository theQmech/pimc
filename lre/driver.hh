#ifndef LRE_DRIVER_HH
#define LRE_DRIVER_HH

#include <string>
#include <map>
#include "parser.hh"

// Tell Flex the lexer's prototype ...
# define YY_DECL \
  yy::lre_parser::symbol_type yylex (lre_driver& driver)
// ... and declare it for the parser's sake.
YY_DECL;

enum class DS {BDD, CLAUSES};

class lre_driver{
public:
  lre_driver();
  virtual ~lre_driver ();

  std::map<std::string, int> variables;
  int result;
  int err = 0;
  ast_node *root;
  DS DS_used;

  // Handling the scanner.
  void scan_begin ();
  void scan_end ();
  bool trace_scanning;

  // Run the parser on file F.
  // Return 0 on success.
  int parse (const std::string& f);
  // The name of the file being parsed.
  // Used later to pass the file name to the location tracker.
  std::string file;
  // Whether parser traces should be generated.
  bool trace_parsing;

  // Error handling.
  void error (const yy::location& l, const std::string& m);
  void error (const std::string& m);
};

#endif // ! LRE_DRIVER_HH
