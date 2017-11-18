#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <string.h>
#include "compiler.h"

void Parser::syntax_error()
{
    cout << "Syntax Error\n";
    exit(1);
}

// this function gets a token and checks if it is
// of the expected type. If it is, the token is
// returned, otherwise, syntax_error() is generated
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
    //program -> var_section
    parse_var_section();
    // program -> body    
    parse_body();
}

void Parser:: parse_var_section(){
    parse_id_list();
    expect(SEMICOLON);
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
    else if(t.token_type == SEMICOLON){
        return ids;
    }
    else{
        syntax_error();
    }
}

void Parser::parse_body()
{   
    expect(LBRACE);
    parse_stmt_list();
    expect(RBRACE);
}

void Parser::parse_stmt_list()
{
    // stmt_list -> stmt
    // stmt_list -> stmt stmt_list
    
    parse_stmt();
    Token t = peek();
    if (t.token_type == WHILE || t.token_type == ID || t.token_type == print || t.token_type == IF || t.token_type == SWITCH)
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
    // stmt -> print_stmt
    // stmt -> while_stmt
    // stmt -> if_stmt
    // stmt -> switch_stmt
 
    Token t = peek();
    if(t.token_type == ID){
        parse_assign_stmt();
    }
    else if(t.token_type == print){
        parse_print_stmt();
    }
    else if(t.token_type == WHILE){
       parse_while_stmt();
    }
    else if(t.token_type == IF){
        parse_if_stmt();
    }
    else if(t.token_type == SWITCH){
        parse_switch_stmt();
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
    expect(EQUAL);
    parse_primary();
    Token t = peek();
    if(t.token_type == PLUS || t.token_type == MINUS || t.token_type == MULT || t.token_type == DIV){
        parse_op();
        parse_primary();
    }
    expect(SEMICOLON);
}

void Parser::parse_print_stmt(){
    expect(print);
    expect(ID);
    expect(SEMICOLON);
}

void Parser::parse_while_stmt()
{
   // while_stmt -> WHILE condition LBRACE stmt list RBRACE

    expect(WHILE);
    parse_condition();
    parse_body();
}

void Parser::parse_if_stmt(){
    expect(IF);
    parse_condition();
    parse_body();
}

void Parser::parse_switch_stmt(){
    expect(SWITCH);
    expect(ID);
    expect(LBRACE);
    parse_case_list();
    Token t = peek();
    if(t.token_type == DEFAULT){
        parse_default_case();
    }
    expect(RBRACE);

void Parser::parse_expr()
{
    // expr -> primary op primary

    parse_primary();
    parse_op();
    parse_primary();
}

void Parser:: parse_op(){
    // op -> PLUS
    // op -> MINUS
    // op -> MULT
    // op -> DIV

    Token t = peek();
    if(t.token_type == PLUS){
        Token t = lexer.GetToken();
    }
    else if(t.token_type == MINUS){
        Token t = lexer.GetToken();
    }
    else if(t.token_type == MULT){
        Token t = lexer.GetToken();
    }
    else if(t.token_type == DIV){
        Token t = lexer.GetToken();
    }
    else{
        syntax_error();
    }
}

void Parser::parse_condition()
{
    // condition -> primary relop primary

    parse_primary();
    parse_relop();
    parse_primary();
}

string Parser::parse_primary()
{
    // primary -> ID
    // primary -> NUM
    Token t = peek();
    if(t.token_type == ID){
        Token t = lexer.GetToken();
        return t.lexeme;
    }
    else if(t.token_type == NUM){
        Token t = lexer.GetToken();
        return "INT";
    }
    else{
        syntax_error();
    }
}

void Parser::parse_relop()
{
    // relop -> GREATER
    // relop -> LESS
    // relop -> NOTEQUAL

    Token t = peek();
    if(t.token_type == GREATER){
        Token t = lexer.GetToken();
    }
    else if(t.token_type == LESS){
        Token t = lexer.GetToken();
    }
    else if(t.token_type == NOTEQUAL){
        Token t = lexer.GetToken();
    }
    else{
        syntax_error();
    }
}

void Parser::parse_for_stmt(){
    expect(FOR);
    expect(LPAREN);
    parse_assign_stmt();
    parse_condition();
    expect(SEMICOLON);
    parse_assign_stmt();
    expect(RPAREN);
    parse_body();
}

void Parser::parse_case_list(){
    parse_case();
    Token t = peek();
    if(t.token_type == CASE){
        parse_case_list();
    }
}

void Parser::parse_case(){
    expect(CASE);
    expect(NUM);
    expect(COLON);
    parse_body();
}

void Parser::parse_default_case(){
    expect(DEFAULT);
    expect(COLON);
    parse_body();
}

void Parser::ParseInput()
{
    parse_program();
    expect(END_OF_FILE);
}
struct StatementNode * parse_generate_intermediate_representation()
{   
    Parser parser;
    StatementNode* s1 = parser.ParseInput();
    return s1;
    // Sample program for demonstration purpose only
    // Replace the following with a parser that reads the program from stdin &
    // creates appropriate data structures to be executed by execute_program()
    // This is the imaginary input for the following construction:
    // a, b;
    // {
    //    a = 10;
    //    b = 1;
    //    WHILE a > 0
    //    {
    //        b = b * a;
    //        a = a - 1;
    //    }
    //    print b;
    // }

    /*struct ValueNode * A = new ValueNode;
    struct ValueNode * B = new ValueNode;
    struct ValueNode * ONE = new ValueNode;
    struct ValueNode * TEN = new ValueNode;
    struct ValueNode * ZERO = new ValueNode;
    struct StatementNode * s1 = new StatementNode;
    struct StatementNode * s2 = new StatementNode;
    struct StatementNode * s3 = new StatementNode;
    struct StatementNode * s4 = new StatementNode;
    struct StatementNode * s5 = new StatementNode;
    struct StatementNode * s6 = new StatementNode;
    struct StatementNode * s7 = new StatementNode;
    struct StatementNode * s8 = new StatementNode;

    A->name = "a";      A->value = 0;
    B->name = "b";      B->value = 0;
    ONE->name = "";     ONE->value = 1;
    TEN->name = "";     TEN->value = 10;
    ZERO->name = "";    ZERO->value = 0;

    s1->type = ASSIGN_STMT;
    s1->assign_stmt = new AssignmentStatement;
    s1->assign_stmt->left_hand_side = A;
    s1->assign_stmt->op = OPERATOR_NONE;
    s1->assign_stmt->operand1 = TEN;
    s1->assign_stmt->operand2 = NULL;
    s1->next = s2;

    s2->type = ASSIGN_STMT;
    s2->assign_stmt = new AssignmentStatement;
    s2->assign_stmt->left_hand_side = B;
    s2->assign_stmt->op = OPERATOR_NONE;
    s2->assign_stmt->operand1 = ONE;
    s2->assign_stmt->operand2 = NULL;
    s2->next = s3;

    s3->type = IF_STMT;
    s3->if_stmt = new IfStatement;
    s3->if_stmt->condition_op = CONDITION_GREATER;
    s3->if_stmt->condition_operand1 = A;
    s3->if_stmt->condition_operand2 = ZERO;
    s3->if_stmt->true_branch = s4;
    s3->if_stmt->false_branch = s7;
    s3->next = s7;

    s4->type = ASSIGN_STMT;
    s4->assign_stmt = new AssignmentStatement;
    s4->assign_stmt->left_hand_side = B;
    s4->assign_stmt->op = OPERATOR_MULT;
    s4->assign_stmt->operand1 = B;
    s4->assign_stmt->operand2 = A;
    s4->next = s5;

    s5->type = ASSIGN_STMT;
    s5->assign_stmt = new AssignmentStatement;
    s5->assign_stmt->left_hand_side = A;
    s5->assign_stmt->op = OPERATOR_MINUS;
    s5->assign_stmt->operand1 = A;
    s5->assign_stmt->operand2 = ONE;
    s5->next = s6;

    s6->type = GOTO_STMT;
    s6->goto_stmt = new GotoStatement;
    s6->goto_stmt->target = s3;    // Jump to the if statement
    s6->next = s7;

    s7->type = NOOP_STMT;
    s7->next = s8;

    s8->type = PRINT_STMT;
    s8->print_stmt = new PrintStatement;
    s8->print_stmt->id = B;
    s8->next = NULL;
*/
}
