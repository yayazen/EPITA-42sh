## 42SH LL Grammar

###Tokens

```
%token  IF    THEN    ELSE    ELIF    FI    DO    DONE   ...
       'if'  'then'  'else'  'elif'  'fi'  'do'  'done'  ...
```

These tokens are only recognised when being the first token
of a command. Otherwise, these types as WORD tokens.

###Rules

```
input:       list
         |   list EOF
         |   '\n'
         |   EOF

list:        and_or ((';'|'&') and_or)* [';'|'&']

and_or:      pipeline (('&&'|'||') ('\n')* pipeline)*

pipeline:    ['!'] command ('|' ('\n')* command)*

command:     simple_command
         |   shell_command (redirection)*
         |   funcdec (redirection)*

simple_command: (prefix)+
         |   (prefix)* (element)+

shell_command: '{' compound_list '}'
         |   '(' compound_list ')'
         |   rule_for
         |   rule_while
         |   rule_until
         |   rule_case
         |   rule_if

funcdec:     ['function'] WORD '(' ')' ('\n')* shell_command

redirection: [IONUMBER] '>' WORD
         |   [IONUMBER] '<' WORD
         |   [IONUMBER] '>>' WORD
         |   [IONUMBER] '>&' WORD
         |   [IONUMBER] '<&' WORD
         |   [IONUMBER] '>|' WORD
         |   [IONUMBER] '<>' WORD

prefix:     ASSIGNEMENT_WORD
         |   redirection

element:     WORD
         |   redirection

compound_list:
    ('\n')* and_or ((';'|'&'|'\n') ('\n')* and_or)* [('&'|';'|'\n') ('\n')*]

rule_for:
    For WORD ([';']|[('\n')* 'in' (WORD)* (';'|'\n')]) ('\n')* do_group

rule_while: While compound_list do_group

rule_until: Until compound_list do_group

rule_case:  Case WORD ('\n')* 'in' ('\n')* [case_clause] Esac

rule_if:    If compound_list Then compound_list [else_clause] Fi

else_clause:    Else compound_list
        |   Elif compound_list Then compound_list [else_clause]

do_group: Do compound_list Done

case_clause: case_item (';;' ('\n')* case_item)* [;;] ('\n')*

case_item:  ['('] WORD ('|' WORD)* ')' ('\n')* [ compound_list ]
```
