
#ifndef LEXER_H
#define LEXER_H

#include <string>
using namespace std;

//Setting max identifier length to 35 although python allows for infinite length for identifiers
//NOTE The max identifier length can be set to unlimited. Max length is enforced on lexer.cpp -> lexer() -> if (isalpha(ch))

#define MAXIDENTIFIERLENGTH 35

typedef enum{
    whitespacesym,
    blocksym,
    intsym,
    printsym,
    ifsym,
    elsesym,
    elseifsym,
    defsym,
    returnsym,
    whilesym,
    semicolonsym,
    commasym,
    opensquaresym,
    closesquaresym,
    openbracketsym,
    closebracketsym,
    dividesym,
    multiplysym,
    plussym,
    minussym,
    assignsym,
    equalsym,
    identifiersym,
    errorsym,
    eofsym,
    shebangsym,
    commentsym,
    leftanklesym,
    rightanklesym,
    colonsym,
    notequalsym,
    greaterthansym,
    lessthansym,
    greaterorequalsym,
    lessorequalsym,
    singlequotesym,
    doublequotesym,
} lextokens;

extern lextokens lexer();
extern int tracker;
extern int linenumber;

//Variable to store the last identifier encountered by lexer. Error symbol also uses the same variable
extern string identifier;
extern char ch;
extern lextokens currenttoken;

//Variable to store the last integer value encountered by the lexer
extern int intvalue;
//
extern string lineInput;

extern char getch();

// This function returns tokens that is not whitespace or block symbol
extern lextokens cleanLexer();

#endif