/*
 * Copyright (C) Rida Bazzi, 2017
 *
 * Do not share this file with anyone
 */
#ifndef __PARSER_H__
#define __PARSER_H__

#include <string>
#include "lexer.h"

class Parser {
  private:
    LexicalAnalyzer lexer;

    void syntax_error();
    Token expect(TokenType expected_type);
    Token peek();

    void parse_program();
    void parse_scope();
    void parse_scope_list();
    void parse_declaration();
    void parse_type_decl();
    std::string parse_type_name();
    void parse_var_decl();
    std::vector<Token> parse_id_list();
    void parse_stmt_list();
    void parse_stmt();
    void parse_assign_stmt();
    void parse_while_stmt();
    std::string parse_expr();
    std::string parse_term();
    std::string parse_factor();
    void parse_condition();
    std::string parse_primary();
    void parse_relop();

  public:
    void ParseInput();
};

#endif

