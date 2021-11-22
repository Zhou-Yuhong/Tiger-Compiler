**handle comment**

when reading  "/*" , change the state into COMMENT, and since  the comment can be nested, i use the variable comment_level to mark the level of comment, 

in COMMENT state, when reading "/*" ,i add the comment_level;when read the symbol of the end of comment, i reduce the comment_level,when the comment_level=0, quit the COMMENT state,and the comment is handled.

**handle string**

when reading quotation mark, i change the state into STR. in STR state, the key is to handle special token defined in tiger and add the characters to string , and when read annother quatation mark ,quit the STR state, and the string is handled. 

**error handling**

when the characters read doesn't matched any rule, report "illegal token"

**end-of-file handling**

the scanner handle it, so in tiger.lex, there is no need to handle eof

**other interesting features of your lexer**

i guess no

