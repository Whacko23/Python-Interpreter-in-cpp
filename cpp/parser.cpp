
#ifndef PARSER_CPP
#define PARSER_CPP

#include "parser.h"

// #define DEEBUG

#ifdef DEEBUG
int grammar_tracker = 1;
#endif

vector<double> int_vector;
vector<string> string_vector;
map<string, vector<string>> funct_args;
map<string, astptr> funct_definitions;
map<string, vector<astptr>> vector_with_id;
astptr parseetree;
int lineInsideFunction = 1,
    function_def_line_start = 0;

astptr newnode(nodetype n, string s, astptr first, astptr second, astptr third)
{
    astnode *mynode = new astnode();

    mynode->asttype = n;
    mynode->astdata = s;
    mynode->p1 = first;
    mynode->p2 = second;
    mynode->p3 = third;
    return mynode;
}

/*
<statements> ::= <statement> | <statements>
*/
astptr statements()
{
#ifdef DEEBUG
    cout << grammar_tracker << " ---inside  stmts --- line # " << linenumber << endl;
    grammar_tracker++;
#endif

    astptr pfirst = statement();
#ifdef DEEBUG
    int i = 1;
#endif

    while (true)
    {
#ifdef DEEBUG

        if (i == 40)
            break;
        i++;
        cout << "currenttoken";
        print_current_lextoken(currenttoken);
        cout << endl;
#endif

        if (currenttoken == eofsym)
        {
#ifdef DEEBUG
            cout << grammar_tracker << " ---exit  stmts loop--- line # " << linenumber << endl;
            grammar_tracker++;
#endif

            break;
        }
        if (currenttoken == newlinesym)
        {
            // linenumber++;
            pfirst = newnode(n_newline, "", pfirst, NULL, NULL);
            currenttoken = lexer();
        }

        pfirst = newnode(n_statements, "", pfirst, statement(), NULL);

#ifdef DEEBUG
        cout << grammar_tracker << " ---inside  stmts loop--- line # " << linenumber << endl;
        grammar_tracker++;
#endif
    }

    return pfirst;
};

/*
statement: compound_stmt  | simple_stmts
*/
astptr statement()
{
#ifdef DEEBUG
    cout << grammar_tracker << " ---inside  stmt--- line # " << linenumber << endl;
    grammar_tracker++;
#endif

    astptr pfirst = simple_stmt();
    if (pfirst->asttype == n_empty)
    {
        pfirst = compound_stmt();
    }
    return newnode(n_statement, to_string(linenumber), pfirst, NULL, NULL);
}

/*
<simple statement> : := <assignment> | <function call> | <printstatement> | <returnstatement> | empty
*/
astptr simple_stmt()
{
// TODO Log("inside statement function: Parser")
#ifdef DEEBUG
    cout << grammar_tracker << " ---inside  simple stmts--- line # " << linenumber << endl;
    grammar_tracker++;
#endif
    astptr pfirst = NULL;
    if (currenttoken == commentsym)
    {
        currenttoken = cleanLexer();

        while (true)
        {
#ifdef DEEBUG
            cout << grammar_tracker << " ---inside  simple stmts loop--- line # " << linenumber << endl;
            grammar_tracker++;
#endif

            if (currenttoken == newlinesym)
            {
                // linenumber++;
                pfirst = newnode(n_newline, "", pfirst, NULL, NULL);
                // currenttoken = lexer();

#ifdef DEEBUG
                cout << grammar_tracker << " ---exit simple stmts loop by newline ---- line # " << linenumber << endl;
                grammar_tracker++;
#endif

                break;
            }
            else if (currenttoken == eofsym)
            {
#ifdef DEEBUG
                cout << grammar_tracker << " ---exit simple stmts loop by eof ---- line # " << linenumber << endl;
                grammar_tracker++;
#endif
                break;
            }
            currenttoken = cleanLexer();
        }
    }
    if (currenttoken == identifiersym)
    {
        // Lookahead without consuming token used here
        if (vec[lexer_vectorindex].tokentype == openbracketsym)
        {
            return funct_call();
        }
        else
            return assignment();
    }
    else if (currenttoken == printsym)
        return printstatement();
    else if (currenttoken == returnsym)
        return returnstatement();
    else
    {
        pfirst = newnode(n_empty, "", NULL, NULL, NULL);
    }
    return pfirst;
}

/*
compound_stmt:
    | function_def
    | if_stmt
    | while_stmt
*/
astptr compound_stmt()
{
#ifdef DEEBUG
    cout << grammar_tracker << " ---inside  compound stmts --- line # " << linenumber << endl;
    grammar_tracker++;
#endif

    if (currenttoken == ifsym)
        return ifstatement();
    else if (currenttoken == defsym)
        return funct();
    else if (currenttoken == whilesym)
        return whilestatement();
    else
        return newnode(n_empty, "", NULL, NULL, NULL);
}

/*
<while> -> "while" <binary expression> ":" <block statement> ["else" <block statement>]
*/
astptr whilestatement()
{
#ifdef DEEBUG
    cout << grammar_tracker << " ---inside  while stmt --- line # " << linenumber << endl;
    grammar_tracker++;
#endif

    astptr pbexp = NULL;
    astptr pwhile = NULL;
    // int currentline;

    if (currenttoken != whilesym)
    {
        // TODO Error Expected while
    }
    else
    {
        currenttoken = cleanLexer();
        pbexp = booleanexpression();
        if (currenttoken == colonsym)
        {
            // currentline = linenumber;
            currenttoken = lexer();
            pwhile = newnode(n_while, "while", pbexp, blockstatement(), NULL);
        }
        else
        {
            // TODO Error expected ":"
        }
    }
    return pwhile;
};

/*
block statement: block <statement>
*/
astptr blockstatement()
{
#ifdef DEEBUG
    cout << grammar_tracker << " ---inside  block stmt --- line # " << linenumber << endl;
    grammar_tracker++;
    cout << "         currenttoken inside block stmt = ";
    print_current_lextoken(currenttoken);
#endif

    astptr pfirst;
    int current_indent = 0;

    if (currenttoken != whitespacesym)
    {

        return newnode(n_empty, "", NULL, NULL, NULL);
    }

    while (true)
    {
#ifdef DEEBUG
        cout << grammar_tracker << " ---inside  block stmt loop--- line # " << linenumber << endl;
        grammar_tracker++;
#endif

        if (currenttoken != whitespacesym)
        {
#ifdef DEEBUG
            cout << grammar_tracker << " ---exit  block stmt loop not whitespace--- line # " << linenumber << endl;
            grammar_tracker++;
#endif

            break;
        }
        currenttoken = lexer();
        current_indent++;
    }

    pfirst = statement();

    return newnode(n_block_stmt, to_string(current_indent), pfirst, NULL, NULL);
};

/*
<blockstatements> ::= <block statement> | <block statements>
*/
astptr blockstatements()
{
#ifdef DEEBUG
    cout << grammar_tracker << " ---inside  block stmts --- line # " << linenumber << endl;
    grammar_tracker++;
#endif

    astptr pfirst = blockstatement();

    if (pfirst->asttype == n_empty)
        return newnode(n_empty, "", NULL, NULL, NULL);
    string firstdata = pfirst->astdata;
    string seconddata = "";
    // int findent = stoi(firstdata);
    // int sindent = 0;
    astptr psecond;

    int i = 1;

    while (true)
    {

        if (i == 100)
            break;
        i++;

#ifdef DEEBUG

        cout
            << grammar_tracker << " ---inside  block stmts loop--- line # " << linenumber << endl;
        cout << " . currenttoken ";
        print_current_lextoken(currenttoken);
        cout << endl;

        //
        cout << " previous token = ";
        print_current_lextoken(vec[lexer_vectorindex - 2].tokentype);
        cout << " current token = ";
        print_current_lextoken(vec[lexer_vectorindex - 1].tokentype);
        cout << " next token = ";
        print_current_lextoken(vec[lexer_vectorindex].tokentype);
        cout << " seond next token = ";
        print_current_lextoken(vec[lexer_vectorindex + 1].tokentype);
        //
        grammar_tracker++;
#endif

        if (currenttoken == eofsym)
        {
#ifdef DEEBUG
            cout << grammar_tracker << " ---exit block stmt loop by eof ---- line # " << linenumber << endl;
            grammar_tracker++;
#endif
            break;
        }
        if (currenttoken == elsesym)
        {
#ifdef DEEBUG
            cout << grammar_tracker << " ---exit block stmt loop by else ---- line # " << linenumber << endl;
            grammar_tracker++;
#endif
            break;
        }
        if (currenttoken == newlinesym)
        {
            // linenumber++;
            pfirst = newnode(n_newline, "", pfirst, NULL, NULL);
            currenttoken = lexer();
            if (currenttoken != whitespacesym)
            {

#ifdef DEEBUG
                cout << grammar_tracker << " ---exit block stmts loop by newline & not whitespace---- line # " << linenumber << endl;
                grammar_tracker++;
#endif

                break;
            }
            psecond = blockstatement();

            seconddata = psecond->astdata;
            if (psecond->asttype == n_block_stmt || psecond->asttype == n_block_stmts)
            {
                // sindent = stoi(seconddata);
            }
            pfirst = newnode(n_block_stmts, seconddata, pfirst, psecond, NULL);
        }
    }
    return pfirst;
};

/*
<return> -> 'return' <expression>
*/
astptr returnstatement()
{
#ifdef DEEBUG
    cout << grammar_tracker << " ---inside  returnstatement()--- line # " << linenumber << endl;
    grammar_tracker++;
#endif
    astptr pfirst = NULL;
    currenttoken = cleanLexer();

    /*
    This statement useful for return 2+3
    However, to keep the interpreter for n_return simple,
    factor() is used instead of expression

    pfirst = expression();

    */
    pfirst = expression();

    return newnode(n_return, "", pfirst, NULL, NULL);
};

// NOTE the previous function (assignment) has already checked for [] brackets
astptr list()
{
#ifdef DEEBUG
    cout << grammar_tracker << " ---inside  list()--- line # " << linenumber << endl;
    grammar_tracker++;
#endif
    astptr pfirst = NULL;
    string tempid;
    bool idInsideList = false;
    int tempint;
    vector<astptr> temp_v;

    if (currenttoken == opensquaresym)
    {
        currenttoken = cleanLexer();

        /*
        Performing lookahead without consuming tokens
        to see if the list contains identifiers
        eg list = [2,x,y,x+1,3,6]

        if yes then idInsideList is set to true

        */
        tempint = lexer_vectorindex - 1;

        while (!idInsideList)
        {

            if (vec[tempint].tokentype == identifiersym)
            {
                idInsideList = true;
            }
            else if (vec[tempint].tokentype == closesquaresym)
            {
                break;
            }
            else if (vec[tempint].tokentype == newlinesym)
            {
                break;
            }
            else if (vec[tempint].tokentype == eofsym)
            {
                break;
            }
            tempint++;
        }

        if (idInsideList)
        {
            while (true)
            {
                if (currenttoken == newlinesym)
                {
                    // TODO Error ] not found
                    break;
                }
                else if (currenttoken == closesquaresym)
                {
                    // TODO
                    break;
                }
                else if (currenttoken == eofsym)
                {
                    // TODO Error
                    break;
                }
                else if (currenttoken == intsym)
                {
                    pfirst = newnode(n_integer, to_string(intvalue), NULL, NULL, NULL);
                    temp_v.push_back(pfirst);
                }
                else if (currenttoken == identifiersym)
                {
                    pfirst = newnode(n_id, identifier, NULL, NULL, NULL);
                    temp_v.push_back(pfirst);
                }

                currenttoken = cleanLexer();
            }
            currenttoken = cleanLexer();

            tempid = add_idVector(temp_v);
            pfirst = newnode(n_list_id, tempid, NULL, NULL, NULL);
            return pfirst;
        }

        if (currenttoken == intsym)
        {
            /*
            int tempvalue = intvalue;
            currenttoken = cleanLexer();
            if(currenttoken==closesquaresym){
                pfirst = newnode(n_listindex,to_string(tempvalue), NULL, NULL, NULL);
                currenttoken = cleanLexer();
                return pfirst;
            }
            */

            while (true)
            {
                if (currenttoken != intsym)
                {
                    // TODO error type mismatch
                }
                int_vector.push_back(intvalue);
                currenttoken = cleanLexer();

                if (currenttoken == eofsym)
                {
#ifdef DEEBUG
                    cout << grammar_tracker << " ---exit list  loop by eof ---- line # " << linenumber << endl;
                    grammar_tracker++;
#endif

                    break;
                }
                if (currenttoken == newlinesym)
                {
#ifdef DEEBUG
                    cout << grammar_tracker << " ---exit list  loop by newline ---- line # " << linenumber << endl;
                    grammar_tracker++;
#endif
                    // TODO error
                    //  linenumber++;
                    pfirst = newnode(n_newline, "", pfirst, NULL, NULL);
                    currenttoken = lexer();
                    break;
                }

                if (currenttoken == closesquaresym)
                {
#ifdef DEEBUG
                    cout << grammar_tracker << " ---exit list  loop by closebracker ---- line # " << linenumber << endl;
                    grammar_tracker++;
#endif
                    currenttoken = cleanLexer();
                    break;
                }
                if (currenttoken == commasym)
                {
                    currenttoken = cleanLexer();
                }
                else
                {
                    cout << "List missing ']', line " << linenumber << endl;
                    errorMsg = "SyntaxError: invalid syntax";
                    exitProgram();
                    // Display error
                    break;
                }
            } //: Exit while
            tempid = add_vector(int_vector);
            // //cout
            // print_vector_int(int_vector);
            // //
            int_vector.clear();
            pfirst = newnode(n_list_int, tempid, NULL, NULL, NULL);
        }
        else if (currenttoken == doublequotesym || currenttoken == singlequotesym)
        {
            // currenttoken = cleanLexer();
        }
        else if (currenttoken == closesquaresym)
        {
            // The list is empty
            int_vector.clear();
            tempid = add_vector(int_vector);
            pfirst = newnode(n_list_int, tempid, NULL, NULL, NULL);
            currenttoken = cleanLexer();
        }
    }
    return pfirst;
};

/*
This function does not have a return type defined
<function> -> 'def' NAME '(' [params] ')' ':'  block
*/
astptr funct()
{
#ifdef DEEBUG
    cout << grammar_tracker << " ---inside  funct--- line # " << linenumber << endl;
    grammar_tracker++;
#endif
    function_def_line_start = linenumber;

    astptr pfirst = NULL;
    string funct_name, arg1, arg2;
    bool arg_found = false;

    string_vector.clear();

    // This skips 'def' that is stored in currenttoken
    currenttoken = cleanLexer();

    if (currenttoken != identifiersym)
    {
        // TODO throw error
    }
    else
    {
        funct_name = identifier;
        currenttoken = cleanLexer();
        if (currenttoken != openbracketsym)
        {
            // TODO error
        }
        else
        {
            currenttoken = cleanLexer();
            if (currenttoken == identifiersym)
            {
                // Function with arguments
                arg1 = identifier;
                currenttoken = cleanLexer();
                string_vector.push_back(arg1);
                arg_found = true;
                if (currenttoken == closebracketsym)
                {
                    // Function with one argument
                    currenttoken = cleanLexer();
                    add_funct_args(funct_name, string_vector);
                }
                else if (currenttoken == commasym)
                {
                    // Function with two argument
                    currenttoken = cleanLexer();
                    if (currenttoken != identifiersym)
                    {
                        // Todo throw error
                    }
                    arg2 = identifier;
                    string_vector.push_back(arg2);
                    currenttoken = cleanLexer();

                    if (currenttoken == closebracketsym)
                    {
                        currenttoken = cleanLexer();
                        add_funct_args(funct_name, string_vector);
                    }
                    else
                    {
                        // TODO throw error missing )
                    }
                }
                else
                {
                    // TODO throw error
                }
            }
            else if (currenttoken == closebracketsym)
            {
                currenttoken = cleanLexer();
                // This is a function with no parameter
            }
            else
            {
                // TODO Error missing )
            }

            if (currenttoken != colonsym)
            {
                // TODO Error missing :
            }
            else if (currenttoken == colonsym)
            {
                currenttoken = cleanLexer();
            }

            if (currenttoken == newlinesym)
            {
                currenttoken = lexer();
            }

            pfirst = blockstatements();

            if (arg_found)
            {
                pfirst = newnode(n_funct_arg, to_string(function_def_line_start), pfirst, NULL, NULL);
            }
            else
            {
                pfirst = newnode(n_funct, to_string(function_def_line_start), pfirst, NULL, NULL);

                // //
                // cout << "&&&&&&&&&&&&Pirnt parse tree inside funct"<< endl;
                // printParserTree(pfirst);
                // cout << "&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&"<< endl;

                // //
            }

            add_function_def(funct_name, pfirst);
        }
    }
    return newnode(n_funct_definiton, funct_name, NULL, NULL, NULL);
};

/*

// NOTE Might do it later to support multiple arguments.
// Right now, used if-else to support upto 2 function arguments.

astptr argumentlist()
{
    return newnode(n_empty, "", NULL, NULL, NULL);
};
*/

/*
<f call> -> <identifier> '(' [<factor> [','<factor>]] ')'
*/

// Function call is made to handle upto two arguments during calll
astptr funct_call()
{
#ifdef DEEBUG
    cout << grammar_tracker << " ---inside  funct call --- line # " << linenumber << endl;
    grammar_tracker++;
#endif

    string id = identifier;
    astptr arg1 = NULL, arg2 = NULL;

    // This consumes '('
    currenttoken = lexer();

    currenttoken = cleanLexer();

    if (currenttoken != closebracketsym)
    {
        arg1 = factor();
        if (currenttoken == commasym)
        {
            currenttoken = cleanLexer();
            arg2 = factor();
        }
    }

    if (currenttoken == closebracketsym)
    {
        currenttoken = cleanLexer();
    }
    else
    {
        // TODO missing ')' error
    }

    return newnode(n_fcall, id, arg1, arg2, NULL);
};

/*
<expr> -> [+ | -]<term> {(+ | -) <term>}
*/
astptr expression()
{

// TODO Log("inside expression function: Parser")
#ifdef DEEBUG
    cout << grammar_tracker << " ---inside  expression--- line # " << linenumber << endl;
    grammar_tracker++;
#endif

    astptr pfirst = NULL; /* first pointer in the expr chain */
    astptr term2 = NULL;
    int startingtoken;
    string tokendata;
    nodetype ntype;

    // cout << "Inside expression" << endl;

    startingtoken = plussym;
    if ((currenttoken == plussym) || (currenttoken == minussym))
    {
        startingtoken = currenttoken;
        currenttoken = cleanLexer();
    }

    pfirst = term();

    if (startingtoken == minussym)
        pfirst = newnode(n_uminus, "-", pfirst, NULL, NULL);
    while ((currenttoken == plussym) || (currenttoken == minussym))
    {
#ifdef DEEBUG
        cout << grammar_tracker << " ---inside  expression loop --- line # " << linenumber << endl;
        grammar_tracker++;
#endif

        if (currenttoken == plussym)
        {
            ntype = n_plus;
            tokendata = "+";
        }
        else
        {
            ntype = n_minus;
            tokendata = "-";
        };
        currenttoken = cleanLexer();
        term2 = term();
        pfirst = newnode(ntype, tokendata, pfirst, term2, NULL);
    }

    return pfirst;
}

/* term
<term> -> <factor> {(* | /) <factor>)
*/
astptr term()
{

#ifdef DEEBUG
    cout << grammar_tracker << " ---inside  term--- line # " << linenumber << endl;
    grammar_tracker++;
#endif

    astptr pfirst, factor2;
    nodetype ntype;
    string tokendata;
    pfirst = factor();

    // cout << "Inside term" << endl;

    while ((currenttoken == multiplysym) || (currenttoken == dividesym))
    {
#ifdef DEEBUG
        cout << grammar_tracker << " ---inside  term loop--- line # " << linenumber << endl;
        grammar_tracker++;
#endif

        if (currenttoken == multiplysym)
        {
            ntype = n_mul;
            tokendata = "*";
        }
        else
        {
            ntype = n_div;
            tokendata = "/";
        };
        currenttoken = cleanLexer();
        factor2 = factor();
        pfirst = newnode(ntype, tokendata, pfirst, factor2, NULL);
    }

    return pfirst;
}

/* factor
<factor> -> id  | arrayindex e.g. a[0] | integer | string | function call e.g. hello("world") | len(list) | ( <expr> )
*/
astptr factor()
{
#ifdef DEEBUG
    cout << grammar_tracker << " ---inside  factor --- line # " << linenumber << endl;
    grammar_tracker++;
#endif

    astptr pfirst = NULL;
    string temp;

    // cout << "Inside factor" << endl;
    if (currenttoken == identifiersym)
    {
        temp = identifier;
        // Lookahead without consuming token used here

        if ((vec[lexer_vectorindex].tokentype) == openbracketsym)
        {
            return funct_call();
        }

        currenttoken = cleanLexer();
        if (currenttoken == opensquaresym)
        {
            currenttoken = cleanLexer();
            if (currenttoken == identifiersym)
            {
                //"notice me senpai"
                // deals with identifier as an index eg example = examples[i]

                /*
                Note the first node p1 is pointing to an empty nodetype
                This is to differentiate between n_list index integer i.e. examples[1]
                with n_list index identifier i.e. examples[a]

                The compiler will check if p1 is pointing to NULL or not
                If p1 is pointing to NULL, then the given node is for integer
                else the given pointer is idetifier
                */
                pfirst = newnode(n_empty, "", NULL, NULL, NULL);
                // this saves the identifier
                pfirst = newnode(n_listindex_data, identifier, pfirst, NULL, NULL);
                pfirst = newnode(n_listindex, temp, pfirst, NULL, NULL);
                currenttoken = cleanLexer();
                if (currenttoken != closesquaresym)
                {
                    // TODO SyntaxError: invalid syntax missing ']'
                }
                else
                {
                    currenttoken = cleanLexer();
                }
            }
            else if (currenttoken == intsym)
            {
                pfirst = newnode(n_listindex_data, to_string(intvalue), NULL, NULL, NULL);
                pfirst = newnode(n_listindex, temp, pfirst, NULL, NULL);
                currenttoken = cleanLexer();
                if (currenttoken != closesquaresym)
                {
                    // TODO SyntaxError: invalid syntax missing ']'
                }
                else
                {
                    currenttoken = cleanLexer();
                }
            }
            else
            {
                // TODO error
            }
        }
        else
        {
            // No need to call for new token since it has already been called before if statement
            pfirst = newnode(n_id, identifier, NULL, NULL, NULL);
        }
    }
    else if (currenttoken == intsym)
    {
        pfirst = newnode(n_integer, to_string(intvalue), NULL, NULL, NULL);
        currenttoken = cleanLexer();
    }
    else if (currenttoken == doublequotesym || currenttoken == singlequotesym)
    {
        pfirst = newnode(n_string, identifier, NULL, NULL, NULL);
        currenttoken = cleanLexer();
    }
    else if (currenttoken == opensquaresym)
    {
#ifdef DEEBUG
        cout << grammar_tracker << " -3--list in factor --- line # " << linenumber << endl;
        grammar_tracker++;
#endif
        pfirst = list();
        pfirst = newnode(n_list_factor, "", pfirst, NULL, NULL);
        if (currenttoken != closesquaresym)
        {
            // TODO Expected ']'
        }
        else
        {
            currenttoken = cleanLexer();
        }
    }
    else if (currenttoken == lensym)
    {
        currenttoken = cleanLexer();
        if (currenttoken != openbracketsym)
        {
            // TODO error missing (
        }
        else
        {
            currenttoken = cleanLexer();
            if (currenttoken != identifiersym)
            {
                // TODO error need an id
            }
            else
            {
                pfirst = newnode(n_len, identifier, NULL, NULL, NULL);
                currenttoken = cleanLexer();
                if (currenttoken != closebracketsym)
                {
                    // TODO missing )
                }
                else
                {
                    currenttoken = cleanLexer();
                }
            }
        }
    }
    else
    {
        if (currenttoken == openbracketsym)
        {
            currenttoken = cleanLexer();
            pfirst = expression();
            if (currenttoken == closebracketsym)
            {
                currenttoken = cleanLexer();
            }
            else
            {
                // TODO - Missing ')'
            }
        }
        else
        {
            // TODO - error not an id, integer or (expression)
        }
    }
    return pfirst;
}

/* if statement
<ifstmt> -> if <boolexpr> colon <block statement> [else <statement>]
*/
astptr ifstatement()
{
#ifdef DEEBUG
    cout << grammar_tracker << " ---inside  if stmt --- line # " << linenumber << endl;
    grammar_tracker++;
#endif

    astptr pfirst = NULL, bexp = NULL, elsee = NULL;
    // int currentindent = 0;
    if (currenttoken != ifsym)
    {
        // TODO Errror not an if ststaement
    }
    else
    {
        currenttoken = cleanLexer();
        bexp = booleanexpression();
        if (currenttoken != colonsym)
        {
            // TODO Error expected ':' after if
        }
        else
        {
            currenttoken = lexer();
            if (currenttoken == newlinesym)
            {
                // linenumber++;
                pfirst = newnode(n_newline, "", pfirst, NULL, NULL);
                currenttoken = lexer();
            }
            pfirst = blockstatements();

            if (currenttoken == elsesym)
            {
                currenttoken = cleanLexer();
                if (currenttoken == colonsym)
                {
                    currenttoken = lexer();
                    if (currenttoken == newlinesym)
                    {
                        // linenumber++;
                        pfirst = newnode(n_newline, "", pfirst, NULL, NULL);
                        currenttoken = lexer();
                    }
                    elsee = blockstatements();
                    // currenttoken = cleanLexer();
                    pfirst = newnode(n_ifelse, "", bexp, pfirst, elsee);
                }
                else
                {
                    // TODO Missing colon
                }
            }
            else
            {
                pfirst = newnode(n_if, "", bexp, pfirst, NULL);
            }
        }
    }
    return pfirst;
}
/* assign statement
<assignment> -> identifier = {<experssion> | <list>}
*/
astptr assignment()
{
#ifdef DEEBUG
    cout << grammar_tracker << " ---inside  assignment --- line # " << linenumber << endl;
    grammar_tracker++;
#endif

    astptr pfirst = NULL, lis = NULL, exp = NULL, index_id = NULL, index_num = NULL;
    string id;

    id = identifier;
    currenttoken = cleanLexer();

    // This is for the list index e.g. a[0] in a[0] = 1
    if (currenttoken == opensquaresym)
    {
        currenttoken = cleanLexer();
        if (currenttoken != intsym)
        {
            // TODO TypeError: list indices must be integers or slices, not str
        }
        else
        {
            index_num = newnode(n_index_assign_index, to_string(intvalue), NULL, NULL, NULL);
            index_id = newnode(n_index_assign_id, id, NULL, NULL, NULL);
            currenttoken = cleanLexer();
            if (currenttoken != closesquaresym)
            {
                // TODO SyntaxError: invalid syntax missing ']'
            }
            else
            {
                currenttoken = cleanLexer();
            }
        }
        if (currenttoken != assignsym)
        {
            // TODO Expected assign symbol
        }
        else
        {
            currenttoken = cleanLexer();
            exp = expression();
            pfirst = newnode(n_index_assign_data, "", index_id, index_num, exp);
            return pfirst;
        }
    }
    /*
    id = identifier;
    currenttoken = cleanLexer();
    */
    if (currenttoken != assignsym)
    {
        // TODO Expected assign symbol
    }
    else
    {
        currenttoken = cleanLexer();
        if (currenttoken == opensquaresym)
        {
            // TODO check for list indexes as well
            lis = list();

            pfirst = newnode(n_assignment_list, id, lis, NULL, NULL);
            if (currenttoken != closesquaresym)
            {
                // TODO Expected ']'
            }
            else
            {
                currenttoken = cleanLexer();
            }

            if (currenttoken == plussym)
            {
                exp = expression();
                pfirst = newnode(n_plus, id, pfirst, exp, NULL);
            }
        }
        else
        {
            exp = expression();
            pfirst = newnode(n_assignment_int, id, exp, NULL, NULL);
        }
    }
    return pfirst;
};

/* print statement
<print> -> print({<expression> | <list>) //TODO List
*/
astptr printstatement()
{
#ifdef DEEBUG
    cout << grammar_tracker << " ---inside  print stmt --- line # " << linenumber << endl;
    grammar_tracker++;
#endif

    astptr pfirst = NULL, expr = NULL;
    if (currenttoken != printsym)
    {
        // TODO Expected print
    }
    else
    {
        currenttoken = cleanLexer();
        if (currenttoken != openbracketsym)
        {
            // TODO Expected '('
        }
        else
        {
            currenttoken = cleanLexer();
            expr = expression();
            if (currenttoken == doublequotesym || currenttoken == singlequotesym)
                currenttoken = cleanLexer();
            pfirst = newnode(n_print, "print", expr, NULL, NULL);

            while (currenttoken == commasym)
            {

#ifdef DEEBUG
                cout << " ---inside  print stmt loop --- line # " << linenumber << endl;
#endif

                currenttoken = cleanLexer();
                expr = expression();
                if (currenttoken == doublequotesym || currenttoken == singlequotesym)
                    currenttoken = cleanLexer();
                pfirst = newnode(n_prints, "", pfirst, expr, NULL);
            }

            if (currenttoken != closebracketsym)
            {
                // TODO Expected ')'
            }
            else
            {
                currenttoken = cleanLexer();
            }
        }
    }
    return pfirst;
};

/* boolean expression
<boolean expression> -> <expresion> <boolean operation> <expression>
*/
astptr booleanexpression()
{
    bool brackettracker = false;
#ifdef DEEBUG
    cout << grammar_tracker << " ---inside  bool exp --- line # " << linenumber << endl;
    grammar_tracker++;
#endif

    astptr exp1 = NULL, exp2 = NULL, bexp = NULL;
    if (currenttoken == openbracketsym)
    {
        brackettracker = true;
        currenttoken = cleanLexer();
    }
    exp1 = expression();
    exp2 = booleanoperation();
    bexp = expression();
    if (brackettracker)
    {
        if (currenttoken == closebracketsym)
        {
            currenttoken = cleanLexer();
        }
        else
        {
            // TODO "missing )"
        }
    }
    return newnode(n_booleanexp, "", exp1, bexp, exp2);
};

/* boolean operation
<boolean expression> -> { == | != | >= | <= | > | <}
*/

astptr booleanoperation()
{
#ifdef DEEBUG
    cout << grammar_tracker << " ---inside  bool op --- line # " << linenumber << endl;
    grammar_tracker++;
#endif

    switch (currenttoken)
    {
    case equalsym:
        currenttoken = cleanLexer();
        return newnode(n_eq, "==", NULL, NULL, NULL);
        break;
    case notequalsym:
        currenttoken = cleanLexer();
        return newnode(n_ne, "!=", NULL, NULL, NULL);
        break;
    case greaterorequalsym:
        currenttoken = cleanLexer();
        return newnode(n_ge, ">=", NULL, NULL, NULL);
        break;
    case greaterthansym:
        currenttoken = cleanLexer();
        return newnode(n_gt, ">", NULL, NULL, NULL);
        break;
    case lessthansym:
        currenttoken = cleanLexer();
        return newnode(n_lt, "<", NULL, NULL, NULL);
        break;
    case lessorequalsym:
        currenttoken = cleanLexer();
        return newnode(n_le, "<=", NULL, NULL, NULL);
        break;
    default:
        return newnode(n_error, " ", NULL, NULL, NULL);
        break;
    }
}

astptr parser()
{
#ifdef DEEBUG
    cout << grammar_tracker << " ---inside  parser() --- line # " << linenumber << endl;
    grammar_tracker++;
#endif

    return statements();
};

void printParserTree(astptr head)
{
    string current;
    astptr left, right, mid;

    switch (head->asttype)
    {
    case n_id:
    case n_integer:
    case n_string:
    case n_eq:
    case n_ne:
    case n_gt:
    case n_lt:
    case n_le:
    case n_ge:
    case n_list_int:
    case n_list_id:
    case n_index_assign_index:
    case n_index_assign_id:
    case n_error:
    case n_empty:
    case n_def:
    case n_len:
    case n_funct_definiton:
        cout << "token type = ";
        print_current_parsetoken(head->asttype);
        cout << " " << head->astdata << " ";
        cout << "*leaf* " << endl;
        break;
    case n_listindex_data:
        cout << "token type = ";
        print_current_parsetoken(head->asttype);

        if (head->p1 != NULL)
        {
        }
        else
        {
        }
        cout << " " << head->astdata << " ";
        cout << "*leaf* " << endl;
        break;
    case n_plus:
    case n_minus:
    case n_div:
    case n_mul:
    case n_statements:
    case n_while:
    case n_prints:
    case n_if:
        cout << "token type = ";
        print_current_parsetoken(head->asttype);

        cout << " " << head->astdata << " ";
        left = head->p1;
        right = head->p2;
        cout << " left " << endl;
        printParserTree(left);
        cout << " right " << endl;
        printParserTree(right);
        break;
    case n_fcall:
        cout << "token type = ";
        print_current_parsetoken(head->asttype);

        cout << " " << head->astdata << " ";
        left = head->p1;
        right = head->p2;
        if (left != NULL)
        {
            cout << " left " << endl;
            printParserTree(left);
        }
        if (right != NULL)
        {
            cout << " right " << endl;
            printParserTree(right);
        }

        break;
    case n_block_stmts:
        cout << "token type = ";
        print_current_parsetoken(head->asttype);
        left = head->p1;
        right = head->p2;
        cout << " left " << endl;
        printParserTree(left);
        cout << " right " << endl;
        printParserTree(right);
        break;
    case n_statement:
    case n_block_stmt:
    case n_simple_stmt:
    case n_return:
    case n_list_factor:
        cout << "token type = ";
        print_current_parsetoken(head->asttype);

        left = head->p1;
        cout << " down " << endl;
        printParserTree(left);
        break;
    case n_ifelse:
    case n_booleanexp:
    case n_index_assign_data:
        left = head->p1;
        mid = head->p2;
        right = head->p3;
        cout << " left " << endl;
        printParserTree(left);
        cout << " mid " << endl;
        printParserTree(mid);
        cout << " right " << endl;
        printParserTree(right);
        break;
    case n_assignment_list:
    case n_assignment_int:
    case n_funct:
    case n_funct_arg:

        cout << "token type = ";
        print_current_parsetoken(head->asttype);

        cout << " " << head->astdata << " = ";
        left = head->p1;
        cout << " down " << endl;
        printParserTree(left);
        break;
    case n_print:
    case n_listindex:
    case n_uminus:
        cout << "token type = ";
        print_current_parsetoken(head->asttype);
        left = head->p1;
        cout << head->astdata << " ";
        cout << " down" << endl;
        printParserTree(left);
        break;
    case n_newline:
        left = head->p1;
        printParserTree(left);
        break;
    }
}

void freeMemory(astptr head)
{
    astptr left, right, mid;

    switch (head->asttype)
    {
    case n_def:
    case n_id:
    case n_integer:
    case n_empty:
    case n_error:
    case n_string:
    case n_len:
    case n_eq:
    case n_ne:
    case n_gt:
    case n_lt:
    case n_le:
    case n_ge:
    case n_list_int:
    case n_list_id:
    case n_index_assign_index:
    case n_index_assign_id:
    case n_funct_definiton:
        delete head;
        break;
    case n_listindex_data:
        if (head->p1 != NULL)
        {
            left = head->p1;
            delete left;
        }
        delete head;
        break;
    case n_plus:
    case n_minus:
    case n_div:
    case n_mul:
    case n_statements:
    case n_while:
    case n_prints:
    case n_block_stmts:
    case n_if:
        left = head->p1;
        right = head->p2;
        delete head;
        freeMemory(left);
        freeMemory(right);
        break;
    case n_fcall:
        left = head->p1;
        right = head->p2;
        delete head;
        if (left != NULL)
            freeMemory(left);
        if (right != NULL)
            freeMemory(right);
        break;
    case n_statement:
    case n_block_stmt:
    case n_simple_stmt:
    case n_newline:
    case n_listindex:
    case n_assignment_list:
    case n_assignment_int:
    case n_print:
    case n_uminus:
    case n_funct:
    case n_funct_arg:
    case n_return:
    case n_list_factor:
        left = head->p1;
        delete head;
        freeMemory(left);
        break;
    case n_ifelse:
    case n_booleanexp:
    case n_index_assign_data:
        left = head->p1;
        mid = head->p2;
        right = head->p3;
        delete head;
        freeMemory(left);
        freeMemory(mid);
        freeMemory(right);
        break;
    }
}

void freeFunctionMemory()
{
    map<string, astptr>::iterator it;

    for (it = funct_definitions.begin(); it != funct_definitions.end(); it++)
    {
        freeMemory(it->second);
    }
}

void print_current_lextoken(lextokens t)
{
    switch (t)
    {
    case intsym:
        cout << "Integer token = " << intvalue << endl;
        break;
    case whitespacesym:
        cout << "Whitespace token " << endl;
        break;
    case printsym:
        cout << "Print token " << endl;
        break;
    case blocksym:
        cout << "Block token " << endl;
    case identifiersym:
        cout << "Identifier token = " << identifier << endl;
        break;
    case whilesym:
        cout << "While token " << endl;
        break;
    case eofsym:
        cout << "EOF token " << endl;
        break;
    case ifsym:
        cout << "IF token " << endl;
        break;
    case elsesym:
        cout << "Else token " << endl;
        break;
    case elseifsym:
        cout << "Elseif token " << endl;
        break;
    case defsym:
        cout << "Def token " << endl;
        break;
    case returnsym:
        cout << "REturn token " << endl;
        break;
    case semicolonsym:
        cout << "semicolon token " << endl;
        break;
    case commasym:
        cout << "comma token " << endl;
        break;
    case assignsym:
        cout << "assign token " << endl;
        break;
    case errorsym:
        cout << "Error " << identifier << " is unrecognized on line " << linenumber << " at character " << tracker << endl;
        break;
    case dividesym:
        cout << "divide token " << endl;
        break;
    case openbracketsym:
        cout << "OpenBracker token " << endl;
        break;
    case closebracketsym:
        cout << "CloseBracker token " << endl;
        break;
    case plussym:
        cout << "Plus token " << endl;
        break;
    case minussym:
        cout << "Minus token " << endl;
        break;
    case multiplysym:
        cout << "multiply token " << endl;
        break;
    case equalsym:
        cout << "Euqal token " << endl;
        break;
    case leftanklesym:
        cout << "Left ankle token " << endl;
        break;
    case rightanklesym:
        cout << "Right ankle token " << endl;
        break;
    case colonsym:
        cout << "Colon token " << endl;
        break;
    case commentsym:
        cout << "Comment token " << endl;
        break;
    case singlequotesym:
        cout << "Single quote token = " << identifier << endl;
        break;
    case doublequotesym:
        cout << "Double quote token = " << identifier << endl;
        break;
    case opensquaresym:
        cout << "Open sq bracket token " << endl;
        break;
    case closesquaresym:
        cout << "Close sq bracket token " << endl;
        break;
    case shebangsym:
        cout << "Shebang token " << endl;
        break;
    case notequalsym:
        cout << "Not equal token " << endl;
        break;
    case greaterorequalsym:
        cout << "Greater than or equal token " << endl;
        break;
    case lessorequalsym:
        cout << "Less than or equal token " << endl;
        break;
    case lessthansym:
        cout << "Less than token " << endl;
        break;
    case greaterthansym:
        cout << "Greater than token " << endl;
        break;
    case newlinesym:
        cout << "  New line " << endl;
        break;
    case lensym:
        cout << "  len token " << endl;
        break;
    case fullstopsym:
        cout << "  fullstop token " << endl;
        break;
    }
};

void print_current_parsetoken(nodetype n)
{
    switch (n)
    {
    case n_statements:
        cout << "n_statements" << endl;
        break;
    case n_statement:
        cout << "n_statement" << endl;
        break;
    case n_assignment_list:
        cout << "n_assignment_list" << endl;
        break;
    case n_assignment_int:
        cout << "n_assignment_int" << endl;
        break;
    case n_simple_stmt:
        cout << "n_simple_stmt" << endl;
        break;
    case n_booleanexp:
        cout << "n_booleanexp" << endl;
        break;
    case n_print:
        cout << "n_print" << endl;
        break;
    case n_prints:
        cout << "n_prints" << endl;
        break;
    case n_newline:
        cout << "n_newline" << endl;
        break;
    case n_uminus:
        cout << "n_uminus" << endl;
        break;
    case n_plus:
        cout << "n_plus" << endl;
        break;
    case n_minus:
        cout << "n_minus" << endl;
        break;
    case n_mul:
        cout << "n_mul" << endl;
        break;
    case n_div:
        cout << "n_div" << endl;
        break;
    case n_eq:
        cout << "n_eq" << endl;
        break;
    case n_ne:
        cout << "n_ne" << endl;
        break;
    case n_lt:
        cout << "n_lt" << endl;
        break;
    case n_le:
        cout << "n_le" << endl;
        break;
    case n_gt:
        cout << "n_gt" << endl;
        break;
    case n_ge:
        cout << "n_ge" << endl;
        break;
    case n_integer:
        cout << "n_integer" << endl;
        break;
    case n_string:
        cout << "n_string" << endl;
        break;
    case n_list_int:
        cout << "n_list_int" << endl;
        break;
    case n_listindex:
        cout << "n_listindex" << endl;
        break;
    case n_id:
        cout << "n_id" << endl;
        break;
    case n_while:
        cout << "n_while" << endl;
        break;
    case n_if:
        cout << "n_if" << endl;
        break;
    case n_ifelse:
        cout << "n_ifelse" << endl;
        break;
    case n_error:
        cout << "n_error" << endl;
        break;
    case n_empty:
        cout << "n_empty" << endl;
        break;
    case n_def:
        cout << "n_def" << endl;
        break;
    case n_block_stmt:
        cout << "n_block_stmt" << endl;
        break;
    case n_block_stmts:
        cout << "n_block_stmts" << endl;
        break;
    case n_index_assign_data:
        cout << "n_index_assign_data" << endl;
        break;
    case n_index_assign_id:
        cout << "n_index_assign_id" << endl;
        break;
    case n_index_assign_index:
        cout << "n_index_assign_index" << endl;
        break;
    case n_return:
        cout << "n_return" << endl;
        break;
    case n_fcall:
        cout << "n_fcall" << endl;
        break;
    case n_funct:
        cout << "n_funct" << endl;
        break;
    case n_funct_arg:
        cout << "n_funct_arg" << endl;
        break;
    case n_funct_definiton:
        cout << "n_funct_definiton" << endl;
        break;
    case n_listindex_data:
        cout << "n_listindex_data" << endl;
        break;
    case n_len:
        cout << "n_len" << endl;
        break;
    case n_list_id:
        cout << "n_list_id" << endl;
        break;
    case n_list_factor:
        cout << "n_list_factor" << endl;
        break;
    }
};

/*_________Interpreter functions______*/

int vector_index = 0;
int function_arg_index = 0;
map<string, double> int_indefiers;
map<string, string> string_identifiers;
map<string, vector<double>> vector_identifiers;
bool notfound = false;
vector<double> current_vec_int;
vector<string> current_vec_str;

string add_vector(vector<double> v)
{
    vector_index++;
    // vector_identifiers.insert({to_string(vector_index),v});
    vector_identifiers[to_string(vector_index)] = v;
    return to_string(vector_index);
}

string add_idVector(vector<astptr> v)
{
    vector_index++;
    // vector_identifiers.insert({to_string(vector_index),v});
    vector_with_id[to_string(vector_index)] = v;
    return to_string(vector_index);
};

void add_funct_args(string fname, vector<string> v)
{
    // funct_args.insert({fname,v});
    funct_args[fname] = v;
};

vector<string> get_funct_args(string fname)
{
    map<string, vector<string>>::iterator search = funct_args.find(fname);
    vector<string> v;
    if (search == funct_args.end())
    {
        // TODO not found
        notfound = true;
    }
    else
    {
        notfound = false;
        v = search->second;
    }
    return v;
}

astptr get_funct_head(string s)
{
    map<string, astptr>::iterator search = funct_definitions.find(s);
    astptr v = NULL;
    if (search == funct_definitions.end())
    {
        // TODO not found
        notfound = true;
    }
    else
    {
        notfound = false;
        v = search->second;
    }
    return v;
}

void add_function_def(string n, astptr p)
{
    funct_definitions[n] = p;
    // funct_definitions.insert({n,p});
};

vector<double> get_vector_int(string s)
{
    map<string, vector<double>>::iterator search = vector_identifiers.find(s);
    vector<double> v;
    if (search == vector_identifiers.end())
    {
        // TODO not found
        notfound = true;
    }
    else
    {
        notfound = false;
        v = search->second;
    }

    return v;
}

vector<astptr> get_vector_id(string s)
{
    map<string, vector<astptr>>::iterator search = vector_with_id.find(s);
    vector<astptr> v;
    if (search == vector_with_id.end())
    {
        // TODO not found
        notfound = true;
    }
    else
    {
        notfound = false;
        v = search->second;
    }

    return v;
};

void modify_vector_int(string s, vector<double> v)
{
    map<string, vector<double>>::iterator search = vector_identifiers.find(s);
    if (search == vector_identifiers.end())
    {
        // TODO not found
        cout << "not found" << endl;
    }
    else
    {
        search->second = v;
    }
};

void print_vector_int()
{
    cout << "[";
    for (int i = 0; i < (int)current_vec_int.size(); i++)
    {
        cout << current_vec_int[i];
        if (i != (int)current_vec_int.size() - 1)
        {
            cout << ", ";
        }
    }
    cout << "]";
};

void print_vector_int(vector<double> v)
{
    cout << "[";
    for (auto itr : v)
        cout << itr << " ";
};

void exitProgram()
{
    cout << "Traceback (most recent call last):" << endl;
    cout << " line " << linenumber << ", in "
         << "<module>" << endl;
    if (inside_funct)
    {
        cout << " line " << lineInsideFunction + function_def_line_start << ", in " << curr_fname << endl;
    }
    cout << errorMsg << endl;
    freeMemory(parseetree);
    freeFunctionMemory();
    exit(-1);
};

#endif