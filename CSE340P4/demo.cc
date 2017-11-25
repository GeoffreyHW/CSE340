#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <string.h>
#include <iostream>
#include <string>
#include <map>
#include <iterator>
#include "demo.h"

using namespace std;
   
map<string, ValueNode*> nodeMap;

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
Token Parser::expect(TokenType expected_type)
{
    Token t = lexer.GetToken();
    if (t.token_type != expected_type)
        syntax_error();
    return t;
}

// this function simply checks the next token without
// consuming the input
Token Parser::peek()
{
    Token t = lexer.GetToken();
    lexer.UngetToken(t);
    return t;
}


ValueNode* Parser:: get_value_node(Token tok){
    if(nodeMap.find(tok.lexeme) != nodeMap.end()){
        return nodeMap[tok.lexeme];
    }
    else{
        int num = atoi((tok.lexeme).c_str());
        ValueNode* numNode = new ValueNode();
        numNode->value = num;
        return numNode;
    }
}

void Parser::append_to_end(StatementNode *body, StatementNode *node){
    StatementNode *it = body;
    if(it == NULL){
        syntax_error();
    }
    while(it->next != NULL){
        it = it->next;
    }
    
    it->next= node;
}
    
// Parsing

StatementNode* Parser::parse_program()
{
    //program -> var_section
    parse_var_section();
    // program -> body    
    StatementNode *programNode = parse_body();
    return programNode;
}

void Parser:: parse_var_section(){
    vector<Token> tokens = parse_id_list();
    for(vector<Token>::iterator it = tokens.begin(); it != tokens.end(); it++){
        ValueNode* vn = new ValueNode;
        vn->name = it->lexeme;
        vn->value = 0;
        nodeMap[it->lexeme] = vn;
    }
    expect(SEMICOLON);
}

vector<Token> Parser::parse_id_list()
{
    // id_list -> ID
    // id_list -> ID COMMA id_list

    vector<Token> ids;
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

StatementNode* Parser::parse_body()
{   
    StatementNode *stl;
    
    expect(LBRACE);
    stl = parse_stmt_list();
    expect(RBRACE);
    
    return stl;
}

StatementNode* Parser::parse_stmt_list()
{
    // stmt_list -> stmt
    // stmt_list -> stmt stmt_list
    Token tok = peek();
    StatementNode *st;
    StatementNode *stl;
    
    st = parse_stmt();
    Token t = peek();
    if (t.token_type == WHILE || t.token_type == ID || t.token_type == PRINT || t.token_type == IF || t.lexeme == "FOR" || t.token_type == SWITCH)
    {
        // stmt_list -> stmt stmt_list

        stl = parse_stmt_list();
        if(tok.lexeme == "FOR"){
            st->next->next->next = stl;
        }
        else if(tok.token_type == WHILE || tok.token_type == IF){
            st->next->next = stl;
        }
        else if(tok.token_type == SWITCH){
            append_to_end(st, stl);
        }
        else{            
            st->next = stl;
        }

        return st;
    }
    else if (t.token_type == RBRACE)
    {
        // stmt_list -> stmt
        return st;
    }
    else
    {
        syntax_error();
    }
}

StatementNode* Parser::parse_stmt()
{
    // stmt -> assign_stmt
    // stmt -> print_stmt
    // stmt -> while_stmt
    // stmt -> if_stmt
    // stmt -> switch_stmt
 
    Token t = peek();
    if(t.token_type == ID && t.lexeme != "FOR"){
        return parse_assign_stmt();
    }
    else if(t.token_type == PRINT){
        return parse_print_stmt();
    }
    else if(t.token_type == WHILE){
       return parse_while_stmt();
    }
    else if(t.token_type == IF){
        return parse_if_stmt();
    }
    else if(t.lexeme == "FOR"){
        return parse_for_stmt();
    }
    else if(t.token_type == SWITCH){
        return parse_switch_stmt();
    }
    else{
        syntax_error();
    }
}

StatementNode* Parser::parse_assign_stmt()
{
    // assign_stmt -> ID EQUAL expr SEMICOLON
    StatementNode * s1 = new StatementNode;
    s1->type = ASSIGN_STMT;
    s1->assign_stmt = new AssignmentStatement;
    
    Token first = lexer.GetToken();
    if (first.token_type != ID){
        syntax_error();
    }

    s1->assign_stmt->left_hand_side = get_value_node(first);
    
    expect(EQUAL);
    Token op1 = parse_primary();
    s1->assign_stmt->operand1 = get_value_node(op1);
    Token t = peek();
    if(t.token_type == PLUS || t.token_type == MINUS || t.token_type == MULT || t.token_type == DIV){
        ArithmeticOperatorType op = parse_op();
        s1->assign_stmt->op = op;
        Token op2 = parse_primary();
        s1->assign_stmt->operand2 = get_value_node(op2);
    }
    else{
        s1->assign_stmt->op = OPERATOR_NONE;
        s1->assign_stmt->operand2 = NULL;
    }
    expect(SEMICOLON);
    return s1;
}

StatementNode* Parser::parse_print_stmt(){
    StatementNode * s2 = new StatementNode;
    s2->type = PRINT_STMT;
    s2->print_stmt = new PrintStatement;
    
    expect(PRINT);    
    Token t = expect(ID);
    expect(SEMICOLON);
    s2->print_stmt->id = get_value_node(t);
    
    return s2;
}

StatementNode* Parser::parse_while_stmt()
{
   // while_stmt -> WHILE condition LBRACE stmt list RBRACE
    expect(WHILE);
    StatementNode *s3 = parse_condition();   
    StatementNode *whileBody = parse_body();
    
    s3->if_stmt->true_branch = whileBody;
    
    StatementNode *gt = new StatementNode;
    gt->type = GOTO_STMT;
    gt->goto_stmt = new GotoStatement;
    gt->goto_stmt->target = s3;
    append_to_end(whileBody, gt);
    
    StatementNode* noopWhile = new StatementNode;
    noopWhile->type = NOOP_STMT;
    
    s3->if_stmt->false_branch = noopWhile;   
    s3->next = noopWhile;
    gt->next = noopWhile;
    
    return s3;
}

StatementNode* Parser::parse_for_stmt(){
    Token t = lexer.GetToken();
    if (t.lexeme != "FOR"){
        syntax_error();
    }
    expect(LPAREN);
    
    StatementNode *forAssign = parse_assign_stmt();
    StatementNode *s5 = parse_condition();
    
    forAssign->next = s5;
    
    StatementNode* noopFor = new StatementNode;
    noopFor->type = NOOP_STMT;   
    s5->next = noopFor;
    
    expect(SEMICOLON);
    
    StatementNode *s6 = parse_assign_stmt();
    
    expect(RPAREN);
    
    StatementNode *forBody = parse_body();
    append_to_end(forBody, s6);
    
        
    StatementNode *gt2 = new StatementNode;
    gt2->type = GOTO_STMT;
    gt2->goto_stmt = new GotoStatement;
    gt2->goto_stmt->target = s5;
    append_to_end(forBody, gt2);
    
    s5->if_stmt->true_branch = forBody;
    s5->if_stmt->false_branch = noopFor;   
    s5->next = noopFor;
    gt2->next = noopFor;
    
    return forAssign;
}

StatementNode* Parser::parse_if_stmt(){ 
    expect(IF);
    StatementNode *s4 = parse_condition();
    StatementNode * body_node = parse_body();
    s4->if_stmt->true_branch = body_node;
    
    StatementNode* noop = new StatementNode;
    noop->type = NOOP_STMT;
    
    s4->if_stmt->false_branch = noop;
    append_to_end(body_node, noop);
    s4->next = noop;
    
    return s4;
}
ArithmeticOperatorType Parser:: parse_op(){
    // op -> PLUS
    // op -> MINUS
    // op -> MULT
    // op -> DIV

    ArithmeticOperatorType op;
    Token t = peek();
    if(t.token_type == PLUS){
        Token t = lexer.GetToken();
        op = OPERATOR_PLUS;
    }
    else if(t.token_type == MINUS){
        Token t = lexer.GetToken();
        op = OPERATOR_MINUS;
    }
    else if(t.token_type == MULT){
        Token t = lexer.GetToken();
        op = OPERATOR_MULT;
    }
    else if(t.token_type == DIV){
        Token t = lexer.GetToken();
        op = OPERATOR_DIV;
    }
    else{
        syntax_error();
    }
    return op;
}

ConditionalOperatorType Parser::parse_relop()
{
    // relop -> GREATER
    // relop -> LESS
    // relop -> NOTEQUAL
    
    ConditionalOperatorType cop;

    Token t = peek();
    if(t.token_type == GREATER){
        Token t = lexer.GetToken();
        cop = CONDITION_GREATER;
    }
    else if(t.token_type == LESS){
        Token t = lexer.GetToken();
        cop = CONDITION_LESS;
    }
    else if(t.token_type == NOTEQUAL){
        Token t = lexer.GetToken();
        cop = CONDITION_NOTEQUAL;
    }
    else{
        syntax_error();
    }
    
    return cop;
}

StatementNode* Parser::parse_condition()
{
    // condition -> primary relop primary
    StatementNode *cond = new StatementNode;
    cond->type = IF_STMT;
    cond->if_stmt = new IfStatement;

    Token op1 = parse_primary();
    cond->if_stmt->condition_operand1 = get_value_node(op1);
    ConditionalOperatorType rel = parse_relop();
    cond->if_stmt->condition_op = rel;
    Token op2 = parse_primary();
    cond->if_stmt->condition_operand2 = get_value_node(op2);
    
    return cond;
}

Token Parser::parse_primary()
{
    // primary -> ID
    // primary -> NUM
    Token t = peek();
    if(t.token_type == ID){
        Token t = lexer.GetToken();
        return t;
    }
    else if(t.token_type == NUM){
        Token t = lexer.GetToken();
        return t;
    }
    else{
        syntax_error();
    }
}

StatementNode* Parser::parse_switch_stmt(){
    expect(SWITCH);
    Token case_var = expect(ID);
    expect(LBRACE);
    
    ValueNode* switchValue = get_value_node(case_var);
    
    StatementNode* noopSwitch = new StatementNode;
    noopSwitch->type = NOOP_STMT;   
    
    StatementNode *switchNode = parse_case_list(switchValue, noopSwitch);
    StatementNode *defaultNode = NULL;
    
    Token t = peek();
    if(t.token_type == DEFAULT){
        defaultNode = parse_default_case(noopSwitch);
    }
    else{
        defaultNode = new StatementNode();
        defaultNode->type = NOOP_STMT;
        defaultNode->next = noopSwitch;
    }
    
    append_to_end(switchNode, defaultNode);
    
    expect(RBRACE);
    
    return switchNode;
}

StatementNode* Parser::parse_case_list(ValueNode* switchValue, StatementNode* noopSwitch){
    StatementNode* caseNode = parse_case(switchValue, noopSwitch);
    Token t = peek();
    if(t.token_type == CASE){
        StatementNode *caseListNode = parse_case_list(switchValue, noopSwitch);  
        caseNode->next->next = caseListNode;    
    }
    
    return caseNode;
}

StatementNode* Parser::parse_case(ValueNode* switchValue, StatementNode* noopSwitch){
    expect(CASE);
    Token case_num = expect(NUM);
    expect(COLON);
    
    StatementNode *caseIf = new StatementNode;
    caseIf->type = IF_STMT;
    caseIf->if_stmt = new IfStatement;
    caseIf->if_stmt->condition_op = CONDITION_NOTEQUAL;
    caseIf->if_stmt->condition_operand1 = switchValue;
    caseIf->if_stmt->condition_operand2 = get_value_node(case_num);
    
    StatementNode* case_body = parse_body();
    caseIf->if_stmt->false_branch = case_body;
    
    StatementNode *noopTrue = new StatementNode();
    noopTrue->type = NOOP_STMT;      
    caseIf->if_stmt->true_branch = noopTrue;
    caseIf->next = noopTrue;
    
    
    StatementNode *gt3 = new StatementNode;
    gt3->type = GOTO_STMT;
    gt3->goto_stmt = new GotoStatement;
    gt3->goto_stmt->target = noopSwitch;
    gt3->next = noopSwitch;
    append_to_end(case_body, gt3);
    
    return caseIf;
}

StatementNode* Parser::parse_default_case(StatementNode *noopSwitch){
    expect(DEFAULT);
    expect(COLON);
    
    StatementNode *bodyDefault = parse_body();
    append_to_end(bodyDefault, noopSwitch);
    
    return bodyDefault;
}

StatementNode* Parser::ParseInput()
{
    StatementNode *inputNode = parse_program();
    //expect(END_OF_FILE);
    return inputNode;
}

struct StatementNode * parse_generate_intermediate_representation()
{   
    Parser parser;
    StatementNode *s1 = parser.ParseInput();
    return s1;
}
