/*
 * Copyright (C) Rida Bazzi, 2017
 *
 * Do not share this file with anyone
 *
 * Do not post this file or derivatives of 
 * of this file online
 *
 */
#include <iostream>
#include <cstdlib>
#include <map>
#include<cstring>
#include<string>
#include "parser.h"

using namespace std;

struct Valid{
    string name;
    int declared;
    int used;
    Valid(string aName, int aDeclared, int aUsed){
        name = aName;
        declared = aDeclared;
        used = aUsed;
    }
    Valid(){
        name = "Default";
        declared = 0;
        used = 0;
    }
};

vector<Valid> validTokens;

struct Declaration{
    string name;
    string type;
    int line;
    string typeName;
    Declaration(string aName, string aType, int aLine, string aTypeName){
        name = aName;
        type = aType;
        line = aLine;
        typeName = aTypeName;
    }
    Declaration(){
        name = "Default";
        type = "Default";
        line = 0;
        typeName = "Default";
    }
};

struct Scope{
    Scope* parent;
    map<string, Declaration> declarations;
};
   
Scope* currentScope;

Declaration* lookup_in_local_scope(Scope* scope, string name){
    if(scope == NULL){
        return NULL;
    }
    else if(scope->declarations.count(name) != 0){
        return &(scope->declarations[name]);
    }
    return NULL;
}

Declaration* lookup(Scope* scope, string name){
    if(scope != NULL){
        if(lookup_in_local_scope(scope, name) != NULL){
            return lookup_in_local_scope(scope, name);
        }
        else{
            return lookup(scope->parent, name);
        }
    }
    else{
        return NULL;
    }     
}

void variableExists(string id_name){
    Declaration* currentDec = lookup(currentScope, id_name);
    if(currentDec != NULL){
        if(currentDec->type.compare("TYPE") == 0){
            cout << "ERROR CODE 1.3 " << id_name << endl;
            exit(1);
        }
    }  
    else{
        cout << "ERROR CODE 2.4 " << id_name << endl;
        exit(1);
    }
}

void Parser::syntax_error()
{
    cout << "Syntax Error\n";
    exit(1);
}

// this function gets a token and checks if it is
// of the expected type. If it is, the token is
// returned, otherwise, synatx_error() is generated
// this function is particularly useful to match
// terminals in a right hand side of a rule.
// Written by Mohsen Zohrevandi
Token Parser::expect(TokenType expected_type)
{
    Token t = lexer.GetToken();
    if (t.token_type != expected_type)
        syntax_error();
    return t;
}

// this function simply checks the next token without
// consuming the input
// Written by Mohsen Zohrevandi
Token Parser::peek()
{
    Token t = lexer.GetToken();
    lexer.UngetToken(t);
    return t;
}

// Parsing

void Parser::parse_program()
{
    // program -> scope
    
    parse_scope();
}

void Parser::parse_scope()
{
    // scope -> { scope_list }
    
    expect(LBRACE);
    
    Scope* parent = currentScope;
    currentScope = new Scope;
    currentScope->parent = parent;
    
    parse_scope_list();
    expect(RBRACE);
    
    Scope *dummyScope = currentScope;
    currentScope = currentScope->parent;
    free(dummyScope);
}

void Parser::parse_scope_list()
{
    // scope_list -> stmt
    // scope_list -> scope
    // scope_list -> declaration
    // scope_list -> stmt scope_list
    // scope_list -> scope scope_list
    // scope_list -> declaration scope_list

    Token t = peek();
    //Parse for stmt
    if(t.token_type == ID || t.token_type == WHILE){
        parse_stmt();
        Token t2 = peek();
        if(t2.token_type == ID || t2.token_type == WHILE || t2.token_type == LBRACE || t2.token_type == TYPE || t2.token_type == VAR){
            parse_scope_list();
        }
    }
    //Parse for scope
    else if(t.token_type == LBRACE){
        parse_scope();
        Token t2 = peek();
        if(t2.token_type == ID || t2.token_type == WHILE || t2.token_type == LBRACE || t2.token_type == TYPE || t2.token_type == VAR){
            parse_scope_list();
        }
    }
    //Parse for declaration
    else if (t.token_type == TYPE || t.token_type == VAR){
        parse_declaration();
        Token t2 = peek();
        if(t2.token_type == ID || t2.token_type == WHILE || t2.token_type == LBRACE || t2.token_type == TYPE || t2.token_type == VAR){
            parse_scope_list();
        }
    }
    else{
        syntax_error();
    }
}

void Parser::parse_declaration()
{
    // declaration -> type_decl
    // declaration -> var_decl
    
    Token t = peek();
    if (t.token_type == TYPE)
    	parse_type_decl();
    else if (t.token_type == VAR)
    	parse_var_decl();
}

void Parser::parse_type_decl()
{
    // type_decl -> TYPE id_list COLON type_name SEMICOLON

    expect(TYPE);
    vector<Token> ids = parse_id_list();
    expect(COLON);
    string type_name = parse_type_name();
    expect(SEMICOLON);
    
    for (std::vector<Token>::iterator id = ids.begin(); id != ids.end(); ++ id) {
        Declaration dec = Declaration((*id).lexeme, "TYPE", (*id).line_no, type_name);

        for(std::map<string,Declaration>::iterator it=currentScope->declarations.begin(); it!=currentScope->declarations.end(); ++it){
            if(it->second.type.compare("TYPE") == 0 && it->first.compare(dec.name) == 0){
                cout << "ERROR CODE 1.1 " << dec.name << endl;
                exit(1);
            }
            if(it->second.type.compare("VAR") == 0 && it->first.compare(dec.name) == 0){
                cout << "ERROR CODE 2.2 " << dec.name << endl;
                exit(1);
            }
        }
        currentScope->declarations[dec.name] = dec;
    }
}

string Parser::parse_type_name()
{
    // type_name -> REAL
    // type_name -> INT
    // type_name -> BOOLEAN
    // type_name -> STRING
    // type_name -> LONG
    // type_name -> ID
    Token t = peek();
  
    if (t.token_type == REAL) {
        Token t = lexer.GetToken();
        return "REAL";
    } 
    else if (t.token_type == INT) {
        Token t = lexer.GetToken();
        return "INT";
    }
    else if (t.token_type == BOOLEAN) {
        Token t = lexer.GetToken();
        return "BOOLEAN";
    }
    else if (t.token_type == STRING) {
        Token t = lexer.GetToken();
        return "STRING";
    }
    else if (t.token_type == LONG) {
        Token t = lexer.GetToken();
        return "LONG";
    }
    else if(t.token_type == ID && t.lexeme == "ID"){
        Token t = lexer.GetToken();
        return "ID";
    }
    else if (t.token_type == ID) {
        Token t = lexer.GetToken();
        Declaration *currentDec = lookup(currentScope, t.lexeme);
        if(currentDec != NULL){
            if(currentDec->type.compare("VAR") == 0){
                cout << "ERROR CODE 2.3 " << t.lexeme <<  endl;
                exit(1);
            }
        }
        else{
            cout << "ERROR CODE 1.4 " << t.lexeme << endl;
            exit(1);
        }
        Valid currentValid = Valid(t.lexeme, t.line_no, currentDec->line);
        validTokens.push_back(currentValid);
        return currentDec->typeName;
    }
    else{
        syntax_error();
    }
}

void Parser::parse_var_decl()
{
    // var_decl -> VAR id_list COLON type_name SEMICOLON

    expect(VAR);
    vector<Token> ids = parse_id_list();
    expect(COLON);
    string type_name = parse_type_name();
    expect(SEMICOLON);
    
    for (std::vector<Token>::iterator id = ids.begin(); id != ids.end(); ++ id) {
        Declaration dec = Declaration((*id).lexeme, "VAR", (*id).line_no, type_name);
        for(std::map<string,Declaration>::iterator it=currentScope->declarations.begin(); it!=currentScope->declarations.end(); ++it){
            if(it->second.type.compare("TYPE") == 0 && it->first.compare(dec.name) == 0){
                cout << "ERROR CODE 1.2 " << dec.name << endl;
                exit(1);
            }
            if(it->second.type.compare("VAR") == 0 && it->first.compare(dec.name) == 0){
                cout << "ERROR CODE 2.1 " << dec.name << endl;
                exit(1);
            }
        }       
        currentScope->declarations[dec.name] = dec;
    }
}

vector<Token> Parser::parse_id_list()
{
    vector<Token> ids = vector<Token>();
    // id_list -> ID
    // id_list -> ID COMMA id_list

    Token id = expect(ID);
    ids.push_back(id);
   
    Token t = peek();
    if(t.token_type == COMMA){
        Token t = lexer.GetToken();
        vector<Token> other_ids = parse_id_list();
        for (std::vector<Token>::iterator id = other_ids.begin(); id != other_ids.end(); ++id) {
            ids.push_back(*id);
        }
        return ids;
    }  
    else if(t.token_type == COLON){
        return ids;
    }
    else{
        syntax_error();
    }
}

void Parser::parse_stmt_list()
{
    // stmt_list -> stmt
    // stmt_list -> stmt stmt_list
    
    parse_stmt();
    Token t = peek();
    if (t.token_type == WHILE || t.token_type == ID)
    {
        // stmt_list -> stmt stmt_list
        parse_stmt_list();
    }
    else if (t.token_type == RBRACE)
    {
        // stmt_list -> stmt
    }
    else
    {
        syntax_error();
    }
}

void Parser::parse_stmt()
{
    // stmt -> assign_stmt
    // stmt -> while_stmt
 
    Token t = peek();
    if(t.token_type == ID){
        parse_assign_stmt();
    }
    else if(t.token_type == WHILE){
       parse_while_stmt();
    }
    else{
        syntax_error();
    }
    
}

void Parser::parse_assign_stmt()
{
    // assign_stmt -> ID EQUAL expr SEMICOLON
    
    Token t = lexer.GetToken();
    if (t.token_type != ID){
        syntax_error();
    }
    
    variableExists(t.lexeme);   
    
    Declaration *currentDec = lookup(currentScope, t.lexeme);
    Valid currentValid = Valid(t.lexeme, t.line_no, currentDec->line);
    validTokens.push_back(currentValid);
    string mismatch = currentDec->typeName;
    
    expect(EQUAL);
    string mismatch2 = parse_expr();
    if(mismatch.compare(mismatch2) != 0){
        cout << "TYPE MISMATCH " << t.line_no << " C1" << endl;
        exit(1);
    }
    expect(SEMICOLON);
}

void Parser::parse_while_stmt()
{
   // while_stmt -> WHILE condition LBRACE stmt list RBRACE

    expect(WHILE);
    parse_condition();
    expect(LBRACE);
    parse_stmt_list();
    expect(RBRACE);
}

string Parser::parse_expr()
{
    // expr -> term 
    // expr -> term + expr

    string mismatch = parse_term();
    Token t = peek();
    if(t.token_type == PLUS){
        Token t = lexer.GetToken();
        string mismatch2 = parse_expr();
        if(mismatch.compare(mismatch2) != 0){
            cout << "TYPE MISMATCH " << t.line_no << " C2" << endl;
            exit(1);
        }
    }
    return mismatch;
}

string Parser::parse_term()
{
    // term -> factor
    // term -> factor MULT term

    string mismatch = parse_factor();
    Token t = peek();
    if(t.token_type == MULT){
        Token t = lexer.GetToken();
        string mismatch2 = parse_term();
        if(mismatch.compare(mismatch2) != 0){
            cout << "TYPE MISMATCH " << t.line_no << " C2" << endl;
            exit(1);
        }
    }
    return mismatch;
}

string Parser::parse_factor()
{
    // factor -> LPAREN expr RPAREN
    // factor -> NUM
    // factor -> REALNUM
    // factor -> ID

    Token t = peek();
    if(t.token_type == LPAREN){
        Token t = lexer.GetToken();
        string mismatch = parse_expr();
        expect(RPAREN);
        return mismatch;
    }
    else if(t.token_type == NUM || t.token_type == REALNUM || t.token_type == ID){
        return parse_primary();
    }
    else{
        syntax_error();
    }
}

void Parser::parse_condition()
{
    // condition -> ID
    // condition -> primary relop primary

    Token t = peek();
    if(t.token_type == ID){
        Token t = lexer.GetToken();
        variableExists(t.lexeme);
        Declaration* currentDec = lookup(currentScope, t.lexeme);
        Valid currentValid = Valid(t.lexeme, t.line_no, currentDec->line);
        validTokens.push_back(currentValid);
        Token t2 = peek();
        if(t2.token_type == GREATER || t2.token_type == GTEQ || t2.token_type == LESS || t2.token_type == NOTEQUAL || t2.token_type == LTEQ){
            parse_relop();
            string mismatch = parse_primary();
            if(currentDec->typeName.compare(mismatch) != 0){
                cout << "TYPE MISMATCH " << t.line_no << " C3" << endl;
                exit(1);
            }
        }
        else{
            if(t2.token_type != LBRACE){
                syntax_error();
            }
            if(currentDec->typeName.compare("BOOLEAN") != 0) {
                cout << "TYPE MISMATCH " << t.line_no << " C4" << endl;
                exit(1);
            }
            
        }
    }
    else if(t.token_type == NUM || t.token_type == REALNUM){
        string mismatch = parse_primary();
        parse_relop();
        string mismatch2 = parse_primary();
        if(mismatch.compare(mismatch2) != 0){
            cout << "TYPE MISMATCH " << t.line_no << " C3" << endl;
            exit(1);
        }
    }
    else{
        syntax_error();
    }
}

string Parser::parse_primary()
{
    // primary -> ID
    // primary -> NUM
    // primary -> REALNUM
    Token t = peek();
    if(t.token_type == ID){
        Token t = lexer.GetToken();
        variableExists(t.lexeme);
        Declaration *currentDec = lookup(currentScope, t.lexeme);
        Valid currentValid = Valid(t.lexeme, t.line_no, currentDec->line);
        validTokens.push_back(currentValid);
        return currentDec->typeName;
    }
    else if(t.token_type == NUM){
        Token t = lexer.GetToken();
        return "INT";
    }
    else if(t.token_type == REALNUM){
        Token t = lexer.GetToken();
        return "REAL";
    }
    else{
        syntax_error();
    }
}

void Parser::parse_relop()
{
    // relop -> GTEQ
    // relop -> LESS
    // relop -> NOTEQUAL
    // relop -> LTEQ

    Token t = peek();
    if(t.token_type == GREATER){
        Token t = lexer.GetToken();
    }
    else if(t.token_type == GTEQ){
        Token t = lexer.GetToken();
    }
    else if(t.token_type == LESS){
        Token t = lexer.GetToken();
    }
    else if(t.token_type == NOTEQUAL){
        Token t = lexer.GetToken();
    }
    else if(t.token_type == LTEQ){
        Token t = lexer.GetToken();
    }
    else{
        syntax_error();
    }
}

void Parser::ParseInput()
{
    parse_program();
    expect(END_OF_FILE);
}

void PrintSemantics(){
    for(vector <Valid> :: iterator i = validTokens.begin(); i != validTokens.end(); ++i){
        cout << i->name << " " << i->declared << " " << i->used << endl;
    }
}

int main()
{
    Parser parser;

    parser.ParseInput();
    
    PrintSemantics();
}

