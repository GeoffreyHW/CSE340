/*
 * Copyright (C) Mohsen Zohrevandi, 2017
 *
 * Do not share this file with anyone
 */
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <map>
#include <set>
#include <iterator>
#include <vector>
#include <algorithm>
#include <iterator>

#include "lexer.h"
#include "inputbuf.h"

using namespace std;
    
struct Rule{
    Token LHS;
    vector<Token> RHS;
    Rule(Token aLHS, vector<Token> aRHS){
        LHS = aLHS;
        RHS = aRHS;
    }
};

vector<bool> generateUseless(set<string> terminals, vector<Rule> rules){
    map<string, bool> generating;
    map<string, bool> reachable;
    vector<Rule> :: iterator itr3;
    vector<Token> :: iterator itr4;

    Token startSymbol = rules.begin()->LHS;
    for(itr3 = rules.begin(); itr3 != rules.end(); ++itr3){
        generating[itr3->LHS.lexeme] =  false;
        for(itr4 = itr3->RHS.begin(); itr4 != itr3->RHS.end(); ++itr4){
            if(terminals.find(itr4->lexeme) != terminals.end()){
                generating[itr4->lexeme] = true;
            }
        }
    }
    generating["#"] = true;
    
    bool complete = true;
    while(complete){
        complete = false;
        for(itr3 = rules.begin(); itr3 != rules.end(); ++itr3){
            if(generating[itr3->LHS.lexeme] == false){
                bool addition = true;
                for(itr4 = itr3->RHS.begin(); itr4 != itr3->RHS.end(); ++itr4){
                    if(generating[itr4->lexeme] == false  || (itr4->lexeme == itr3->LHS.lexeme)){
                        addition = false;
                        break;
                    }
                }
                if(addition){
                    generating[itr3->LHS.lexeme] = true;
                    complete = true;
                }
            }
        }               
    }
    complete = true;
    vector<bool> rRules(rules.size(), false);
    while(complete){
        complete = false;
        int counter = 0;
        for(itr3 = rules.begin(); itr3 != rules.end(); ++itr3){                  
            if(itr3 == rules.begin()){
                if(generating[itr3->LHS.lexeme]){
                    reachable[itr3->LHS.lexeme] = true;
                }
            }
            
            if(reachable[itr3->LHS.lexeme]){
                bool addition = true;
                for(itr4 = itr3->RHS.begin(); itr4 != itr3->RHS.end(); ++itr4){
                    if(!generating[itr4->lexeme]){
                        addition = false;
                        break;
                    }
                }
                if(addition && (rRules[counter] == false)){
                    for(itr4 = itr3->RHS.begin(); itr4 != itr3->RHS.end(); ++itr4){
                        reachable[itr4->lexeme] = true;
                    }
                    reachable[itr3->LHS.lexeme] = true;
                    rRules[counter] = true;  
                    complete = true;
                }                       
            }
            counter++;
        }               
    }
    return rRules;
}
map<string, set<string> > generateFirst(set<string> terminals, vector<Rule> rules){
    map<string, set<string> > firstSets;
    set<string> firstRHS;
    vector<Rule> :: iterator itr1;
    vector<Token> :: iterator itr2;
    set<string> :: iterator itr3;
    set<string> :: iterator itr4;
        
    firstRHS.insert("#");
    firstSets["#"] = firstRHS;
    
    for(itr3 = terminals.begin(); itr3 != terminals.end(); ++itr3){  
        set<string> firstRHS;
        firstRHS.insert(*itr3);
        firstSets[*itr3] = firstRHS; 
    }           
    bool complete = true;
    while(complete){
        complete = false;
        for(itr1 = rules.begin(); itr1 != rules.end(); ++itr1){      
            set<string> firstRHS; 
            bool addition = true;            
            for(itr2 = itr1->RHS.begin(); itr2 != itr1->RHS.end(); ++itr2){
                if(itr2 == itr1->RHS.begin() || addition){
                    addition = false;
                    firstRHS = firstSets[itr2->lexeme]; 
                    set<string> symbol = firstSets[itr1->LHS.lexeme];                         
                    for (itr3 = firstRHS.begin(); itr3 != firstRHS.end(); ++itr3) {
                        if (symbol.find(*itr3) == symbol.end()) {
                            if((*itr3).compare("#") != 0){
                                symbol.insert(*itr3);
                                complete = true; 
                            }
                        }
                        firstSets[itr1->LHS.lexeme] = symbol;
                    }
                    
                    if(firstRHS.find("#") != firstRHS.end()){
                        if(itr2 == --(itr1->RHS.end())){
                            firstSets[itr1->LHS.lexeme].insert("#");
                            for (itr4 = symbol.begin(); itr4 != symbol.end(); ++itr4) {
                                firstSets[itr1->LHS.lexeme].insert(*itr4);
                            }
                            break;
                        }
                        addition = true;
                        continue;
                    }   
                }                                                          
                else{                           
                    break;
                }
            }                   
        }
    }
    return firstSets;
}

map<string, set<string> > generateFollow(set<string> terminals, set<string> nonterminals, vector<Rule> rules){
    map<string, set<string> > followSets;
    set<string> followRHS;
    vector<Rule> :: iterator itr1;
    vector<Token> :: reverse_iterator itr2;
    set<string> :: iterator itr3;
    set<string> :: iterator itr4;
    vector<Token> :: reverse_iterator itr5;
    
    map<string, set<string> > firstSet;
    firstSet = generateFirst(terminals, rules);
    
    followRHS.insert("$");
    followSets[rules.begin()->LHS.lexeme] = followRHS;
          
    bool complete = true;
    while(complete){
        complete = false;
        for(itr1 = rules.begin(); itr1 != rules.end(); ++itr1){      
            set<string> followRHS; 
            bool term = false;  
            bool containsEpsilon = true;
            for(itr2 = (itr1->RHS).rbegin(); itr2 != (itr1->RHS).rend(); ++itr2){
                if(terminals.find(itr2->lexeme) != terminals.end()){
                    term = true;
                }      
                
                if(itr2 == itr1->RHS.rbegin() && nonterminals.find(itr2->lexeme) != nonterminals.end()){
                    followRHS = followSets[itr1->LHS.lexeme]; 
                    set<string> symbol = followSets[itr2->lexeme];                         
                    for (itr3 = followRHS.begin(); itr3 !=followRHS.end(); ++itr3) {
                        if (symbol.find(*itr3) == symbol.end()) {
                            symbol.insert(*itr3);
                            complete = true; 
                        }
                        followSets[itr2->lexeme] = symbol;
                    }                       
                }                               
                bool epsilon = true;
                bool followAdd = false;
                
                int ruleCount = 0;
                for(itr5 = itr2; itr5 != itr1->RHS.rend(); ++itr5){
                    if(firstSet[itr2->lexeme].find("#") != firstSet[itr2->lexeme].end()){
                        followAdd = true;
                    }
                    if(epsilon && ruleCount > 0){
                        epsilon = false;
                        set<string> symbol2 = followSets[itr5->lexeme];
                        if(itr5 != itr1->RHS.rend() &&  !firstSet[itr2->lexeme].empty()){
                            if(followAdd){
                                followRHS = followSets[itr1->LHS.lexeme]; 
                                for (itr4 = followRHS.begin(); itr4 !=followRHS.end(); ++itr4) {
                                    if (symbol2.find(*itr4) == symbol2.end() && term == false && containsEpsilon) {
                                        symbol2.insert(*itr4);
                                    }
                                }
                                followAdd = false;
                            }
                            for(itr3 = firstSet[itr2->lexeme].begin(); itr3 != firstSet[itr2->lexeme].end(); ++itr3){
                                if(*itr3 != "#" && symbol2.find(*itr3) == symbol2.end()){
                                    symbol2.insert(*itr3);
                                    complete = true;
                                }
                            } 
                            followSets[itr5->lexeme] = symbol2;                                 
                        }
                        if(firstSet[itr5->lexeme].find("#") != firstSet[itr5->lexeme].end()){
                            epsilon = true;
                        }
                        else{
                            containsEpsilon = false;
                        }
                    }
                    else if(ruleCount == 0){
                        epsilon = true;
                    }
                    ruleCount++;
                }                                  
            }                   
        }
    }
    return followSets; 
}           

int main (int argc, char* argv[])
{
    int task;

    if (argc < 2)
    {
        cout << "Error: missing argument\n";
        return 1;
    }

    task = atoi(argv[1]);
    
    LexicalAnalyzer lexer;
    Token token;
    Token token2;

    vector<Rule> rules;   
    set<string> terminals;
    set<string> nonterminals;    
    vector<string> vTerminals;
    vector<string> vNonterminals;    
    vector<string> lexemes;
    set<string> lexemesSet;   
    vector<string> :: iterator itr1;
    vector<Rule> :: iterator itr2;

    token = lexer.GetToken();

    while (token.token_type != DOUBLEHASH)
    {   
        vector<Token> rhsTokens;

        if(lexemesSet.find(token.lexeme) == lexemesSet.end()){
            lexemes.push_back(token.lexeme);
            lexemesSet.insert(token.lexeme);
        }
        
        token2 = lexer.GetToken();
        
        if(token2.token_type == ARROW){
            token2 = lexer.GetToken();
        }
        
        while(token2.token_type != HASH){
            rhsTokens.push_back(token2);  
            
            if(lexemesSet.find(token2.lexeme) == lexemesSet.end()){
                lexemes.push_back(token2.lexeme);
                lexemesSet.insert(token2.lexeme);
            }
                                             
            token2 = lexer.GetToken();    
        }
        if(rhsTokens.size() == 0){
            token2.lexeme = "#";
            rhsTokens.push_back(token2);
        }
        rules.push_back(Rule(token, rhsTokens));
        token = lexer.GetToken();
    }
    
    for (itr1 = lexemes.begin(); itr1 != lexemes.end(); ++itr1) {
        bool isNonterminal = false;
        for (itr2 = rules.begin(); itr2 != rules.end(); ++itr2) {
            if ((itr2->LHS).lexeme.compare(*itr1) == 0) {
                isNonterminal = true;
                break;
            } 
        }
        
        if (isNonterminal) {
            vNonterminals.push_back(*itr1);
            nonterminals.insert(*itr1);
        } else {
            vTerminals.push_back(*itr1);
            terminals.insert(*itr1);
        }
    }

    switch (task) {
        case 1:{
            vector <string> :: iterator itr1;
            for (itr1 = vTerminals.begin(); itr1 != vTerminals.end(); ++itr1)
            {
                cout << *itr1 << "\t";
            }
            vector <string> :: iterator itr2;

            for (itr2 = vNonterminals.begin(); itr2 != vNonterminals.end(); ++itr2)
            {
                cout << *itr2 << "\t" ;
            }
            cout << "\n";
            break;
        }
        case 2:{
            vector<bool> rRules = generateUseless(terminals, rules);
            
            for(int i = 0; i < rRules.size(); i++){
                if(rRules[i]){
                    cout << rules[i].LHS.lexeme << "\t->\t";
                    for(int j = 0; j < rules[i].RHS.size(); j++){
                        cout << rules[i].RHS[j].lexeme << "\t";
                    }
                    cout << "\n";
                }
            }            
            break;
        }
        case 3:{          
            vector<string> :: iterator itr3;
            set<string> :: iterator itr4;
            vector<string>::iterator terminalIterator;     
            
            map<string, set<string> > aFirstSet;
            aFirstSet = generateFirst(terminals, rules);
            
            for(itr3 = vNonterminals.begin(); itr3 != vNonterminals.end(); ++itr3){                  
                cout << "FIRST(" << *itr3 << ") = { ";
                
                int printCount = 0;
                
                if (aFirstSet[*itr3].find("#") != aFirstSet[*itr3].end()) {
                    if(printCount == aFirstSet[*itr3].size()-1){
                        cout << "#";
                    }
                    else{
                        cout << "#" << ", ";
                        printCount++;
                    }
                }
                
                for (terminalIterator = lexemes.begin(); terminalIterator != lexemes.end(); ++terminalIterator) {
                    if (aFirstSet[*itr3].find(*terminalIterator) != aFirstSet[*itr3].end()) {
                        if(printCount == aFirstSet[*itr3].size()-1){
                            cout << *terminalIterator;
                        }
                        else{
                            cout << *terminalIterator << ", ";
                            printCount++;
                        }
                    }
                }
                cout << " }\n";
            }
            break;
        }
        case 4:{
            vector<string> :: iterator itr3;
            vector<string> :: iterator itr4;     
            
            map<string, set<string> > aFollowSet;
            aFollowSet = generateFollow(terminals, nonterminals, rules);
            lexemes.insert(lexemes.begin(), "$");
            
            for(itr3 = vNonterminals.begin(); itr3 != vNonterminals.end(); ++itr3){                  
                cout << "FOLLOW(" << *itr3 << ") = { ";
                int i = 0;                
                for(itr4 = lexemes.begin(); itr4 != lexemes.end(); ++itr4){
                    if(aFollowSet[*itr3].find(*itr4) != aFollowSet[*itr3].end()){
                        if(i == aFollowSet[*itr3].size()-1){
                            cout << *itr4;
                        }
                        else{
                            cout << *itr4 << ", ";
                        }
                        i++;
                    }
                }   
                cout << " }\n";
            }
            break;
        }
        case 5:{
            set<string> :: iterator itr1;
            set<string> :: iterator itr2;
            vector<Token> :: iterator itr4;
            vector<Token> :: iterator itr5;
            vector<bool> rRules = generateUseless(terminals, rules);
            map<string, set<string> > aFirstSet = generateFirst(terminals, rules);
            map<string, set<string> > aFollowSet = generateFollow(terminals, nonterminals, rules);
            
            bool parse = true;
            
            for(int i = 0; i < rRules.size(); i++){
                for(int j = 0; j < rRules.size(); j++){
                    if(!rRules[i]){
                        parse = false;
                        break;
                    }
                    bool epsilon = true;
                    if(rules[i].LHS.lexeme == rules[j].LHS.lexeme && i != j){
                        for(itr4 = rules[i].RHS.begin(); itr4 != rules[i].RHS.end(); ++itr4){
                            for(itr5 = rules[j].RHS.begin(); itr5 != rules[j].RHS.end(); ++itr5){
                                if(aFirstSet[itr5->lexeme].find(itr4->lexeme) != aFirstSet[itr5->lexeme].end()){
                                    parse = false;
                                    break;
                                }
                 
                                if(aFirstSet[itr5->lexeme].find("#") == aFirstSet[itr5->lexeme].end()){
                                    epsilon = false;
                                    break;
                                }        
                            }
                            if(!epsilon){
                                break;
                            }
                        }
                    }
                    for(int k = 0; k < rules[i].RHS.size(); k++){
                        if(rules[i].RHS[k].lexeme == ("#")){
                            for(itr1 = aFirstSet[rules[i].LHS.lexeme].begin(); itr1 != aFirstSet[rules[i].LHS.lexeme].end(); ++itr1){
                                for(itr2 = aFollowSet[rules[i].LHS.lexeme].begin(); itr2 != aFollowSet[rules[i].LHS.lexeme].end(); ++itr2){
                                    if(*itr1 == *itr2){
                                        parse = false;
                                        break;
                                    }
                                }
                            }
                        }
                    }
                }
            }
            
            if(parse){
                cout << "YES\n";
            }
            else{
                cout << "NO\n";
            }
            break;
        }
        default:
            cout << "Error: unrecognized task number " << task << "\n";
            break;
    }
    return 0;
}

