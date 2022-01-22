# Project structure

Here is a quick description of the organisation of the repository:

* `doc`: contains some documentation material information & doxyfile configuration
* `.gitlab-ci.yml`: contains our Gitlab pipelines. We modified the given file to add a few pipelines to execute functional & unit tests and check for coding style
* `src`: contains the source code
  * `token.c`: contains the list of tokens used by the lexer
  * `lexer*.c`: contains files related to the lexer. The lexer is globally a recursive DFA.
  * `rl_*.c`: contains the rules for the parsing & the execution of the program. We decided to implement a recursive descent parser for this project.
  * `bi_*.c`: contains the source code of the builtins
  * `io` & `utils` : contains the given files by the acus
  * `include` : contains all our headers
  * `list.c`: contains a basic strings list implementation
  * `parser.c`: contains the root parser function
  * `symtab*.c`: contains our symbols table implementation
  * `symexp.c`: contains symbols expansion (variables, single & double quotes, escape characters, command substitution)
* `tests`: contains our testsuite
  * `units_tests`: contains some units tests. Because our apis were evolving quite quickly, we decided quickly that is was not relevant to add further tests.
  * `functional_tests`: contains around ~300 tests for our parser to check for regressions and do TDD
  * `check_coding_style`: a script that allows to check for coding style issue in our source code
