// requires C++14 or greater
// use gcc-6.3.0 or greater on linserv1

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <regex>
#include <iostream>
#include <vector>
#include <map>
#include <unordered_set>

using namespace std;

char delim[] = " \n\t";
char *line = NULL;
size_t linecap = 0;
ssize_t linelen;
int line_num = 0;
int col_num = 0;

void parseerror(int errcode){
    static string errstr[] = {
        "NUM_EXPECTED", // [0] Number expect
        "SYM_EXPECTED", // [1] Symbol Expected
        "ADDR_EXPECTED", // [2] Addressing Expectedwhich is A/E/I/R  
        "SYM_TOO_LONG", // [3] Symbol Name is toolong
        "TOO_MANY_DEF_IN_MODULE", // [4] > 16 
        "TOO_MANY_USE_IN_MODULE", // [5] > 16
        "TOO_MANY_INSTR" // [6] total num_instr exceeds memory size (512)  
    };
    printf("Parse Error line %d offset %d: %s\n",line_num, col_num, errstr[errcode].c_str());
}

void printError(int errcode){
    static string errstr[] = {
        "Error: Absolute address exceeds machine size; zero used", // [0]
        "Error: Relative address exceeds module size; zero used", // [1]
        "Error: External address exceeds length of uselist; treated as immediate", // [2]  
        "Error: This variable is multiple times defined; first value used", // [3] 
        "TOO_MANY_DEF_IN_MODULE", // [4] > 16 
        "TOO_MANY_USE_IN_MODULE", // [5] > 16
        "TOO_MANY_INSTR" // [6] total num_instr exceeds memory size (512)  
    };
    printf("Parse Error line %d offset %d: %s\n",line_num, col_num, errstr[errcode].c_str());
}


struct Module{
    int base_address;
    int defcount;
    int usecount;
    int codecount;
};

struct Symbol{
    string txt;
    int position;
    bool is_mult_def;
    bool in_use_list;
    bool is_used;
    int module_num;
};

vector<Module> modules;
map<string, Symbol> symbol_table;

void createSymbol(string txt, int val, int module_num){
    if(symbol_table.find(txt) == symbol_table.end()){ // not already defined
        Symbol symbol;
        symbol.txt = txt;
        symbol.position = val;
        symbol.module_num = module_num;
        symbol.is_mult_def = false;
        symbol.in_use_list = false;
        symbol.is_used = false;
        symbol_table.insert(pair<string, Symbol>(txt, symbol));
    } else { // symbol already defined
        symbol_table[txt].is_mult_def = true;
    }
}

bool isNumber(string str){
    for(int i = 0; i < str.length(); i++){
        if(!isdigit(str[i])) return false;
    }
    return true;
}

bool isSymbol(string str){
    return regex_match(str, regex("[a-z|A-Z][a-z|A-Z|0-9]*"));
}

bool isAddress(string str){
    return (str == "A" | str == "E" | str == "I" | str == "R");
}

// read next token from the file
char* getToken(FILE *fptr){
    char *ptr;
    ssize_t len;
    ptr = strtok(NULL, delim);
    while(ptr == NULL && !feof(fptr)){ //line is completely scanned. load a new line.
        len = getline(&line, &linecap, fptr);
        if(len > 0){
            linelen = len;
            line_num++;
            ptr = strtok(line, delim);
        }
    }
    if(ptr != NULL){
        col_num = ptr - line + 1;
    }else{
        col_num = linelen;
    }
    return ptr;
}

// read next token from the file, check if valid integer and return
int readInt(FILE *fptr){
    char* tkn = getToken(fptr);
    if(tkn == NULL){
        return -1;
    }
    if(!isNumber(tkn)) {
        parseerror(0);
        exit(0);
    }
    int i = atoi(tkn);
    return i;
}

// read next token from the file, check if valid number (address) and return
int readNumber(FILE *fptr){
    char* tkn = getToken(fptr);
    if(tkn == NULL){
        return -1;
    }
    if(!isNumber(tkn)) {
        parseerror(0);
        exit(0);
    }
    int i = atoi(tkn);
    return i;
}

// read next token from the file, check if valid symbol and return
string readSymbol(FILE *fptr){
    char* tkn = getToken(fptr);
    if(tkn == NULL || !isSymbol(tkn)){
        parseerror(1);
        exit(0);
    } else if(strlen(tkn) > 16){
        parseerror(3);
        exit(0);
    }
    return tkn;
}

// read next token from the file, check if address mode integer and return
string readIEAR(FILE *fptr){
    char* tkn = getToken(fptr);
    if(tkn == NULL || !isAddress(tkn)) {
        parseerror(2);
        exit(0);
    }
    return tkn;
}

void pass1(char* input_file){
    FILE *fptr;
    fptr = fopen(input_file, "r");
    if(fptr == NULL){
        printf("cannot open file");
    }
    
    int offset = 0;
    int total_code = 0;
    int module_num = 0;

    while(!feof(fptr)){
        Module module;
        vector<Symbol> deflist;
        
        // def list - defcount pairs of (S, R)
        int defcount = readInt(fptr);
        if(defcount >= 0 && defcount <= 16){
            module.defcount = defcount;
            for(int i = 0; i < defcount; i++) {
                string symbol = readSymbol(fptr);
                int val = readInt(fptr);
                Symbol s;
                s.txt = symbol;
                s.position = val;
                s.module_num = module_num + 1;
                deflist.push_back(s);
            }
        } else if(defcount > 16){
            parseerror(4);
            exit(0);
        }
        
        // use list - usecount symbols
        int usecount = readInt(fptr);
        if(usecount >= 0 && usecount <=16){
            module.usecount = usecount;
            for (int i = 0; i < usecount; i++) {
                string symbol = readSymbol(fptr);
            }
        } else if(usecount > 16){
            parseerror(5);
            exit(0);
        }
        
        // program text - codecount pairs of (type, instr)
        int codecount = readInt(fptr);
        total_code += codecount;
        if(codecount >= 0 && total_code <= 512){
            module.codecount = codecount;
            module.base_address = offset;
            for (int i = 0; i < codecount; i++) {
                string addressmode = readIEAR(fptr);
                int instr = readNumber(fptr);
            }
            offset += codecount;
            modules.push_back(module);
        }
        else if(total_code > 512){
            parseerror(6);
            exit(0);
        }
        // error_check[5] - is val > codecount? => set it as 0 (relative to module)
        for(int i = 0; i < deflist.size(); i++){
            Symbol s = deflist[i];
            int val = s.position;
            if(symbol_table.find(s.txt) == symbol_table.end()){ // if symbol not defined
                if(val >= codecount){ // display warning, 0 relative offset
                    printf("Warning: Module %d: %s too big %d (max=%d) assume zero relative\n", module_num + 1, s.txt.c_str(), val, codecount - 1);
                    val = 0;
                }
            }
            createSymbol(s.txt, val + (offset - codecount), module_num + 1);
        }
        module_num++;
    }

    // iterate and print symbol table
    printf("Symbol Table\n");
    for(map<string, Symbol>::iterator i = symbol_table.begin(); i != symbol_table.end(); ++i){
        // error_check[2]
        if(!(*i).second.is_mult_def){
            printf("%s=%d\n", (*i).first.c_str(), (*i).second.position);
        } else {
            printf("%s=%d Error: This variable is multiple times defined; first value used\n", (*i).first.c_str(), (*i).second.position);
        }
    }
    fclose(fptr);
}

void pass2(char* input_file){
    FILE *fptr;
    fptr = fopen(input_file, "r");
    if(fptr == NULL){
        printf("cannot open file");
    }

    int count = 0;
    int module_num = 0;

    printf("\nMemory Map\n");

    while(!feof(fptr)){

        vector<string> uselist;
        unordered_set<string> actually_used; 
        
        // def list - defcount pairs of (S, R)
        int defcount = readInt(fptr);
        if(defcount >= 0){
            for(int i = 0; i < defcount; i++) {
                string symbol = readSymbol(fptr);
                int val = readInt(fptr);
            }
        }
        
        // use list - usecount symbols
        int usecount = readInt(fptr);
        if(usecount >= 0){
            for (int i = 0; i < usecount; i++) {
                string symbol = readSymbol(fptr);
                uselist.push_back(symbol);
                if(symbol_table.find(symbol) == symbol_table.end()){
                    // ERROR TO BE HANDLED - handled in E instr case
                } else {
                    symbol_table.at(symbol).in_use_list = true;
                }
            }
        }
        
        // program text - codecount pairs of (type, instr)
        int codecount = readInt(fptr);
        if(codecount >= 0){
            for (int i = 0; i < codecount; i++) {
                string addressmode = readIEAR(fptr);
                int  instr = readNumber(fptr);
                int opcode = instr / 1000;
                int operand = instr % 1000;
                // error_check[11] - contains error_check[10]
                if(opcode <= 9){ // valid

                    // make relevant modifications to instr
                    if(!strcmp(addressmode.c_str(), "I")){
                        printf("%0.3d: %0.4d", count, instr);
                    } 
                    else if(!strcmp(addressmode.c_str(), "A")){
                        // error_check[8] - abs addr <= size of machine (i.e. 512)
                        if(operand < 512){
                            printf("%0.3d: %0.4d", count, instr);
                        } else {
                            operand = 0;
                            instr = (opcode * 1000) + operand;
                            printf("%0.3d: %0.4d Error: Absolute address exceeds machine size; zero used", count, instr);
                        }
                    }
                    else if(!strcmp(addressmode.c_str(), "E")){
                        // error_check[6]
                        if(operand < uselist.size()){ // valid
                            string sym = uselist[operand];
                            // error_check[3] - check if symbol present in symbol_table
                            if(symbol_table.find(sym) == symbol_table.end()){ // symbol not present
                                instr = (opcode * 1000) +  (0);
                                actually_used.insert(sym);
                                printf("%0.3d: %0.4d Error: %s is not defined; zero used", count, instr, sym.c_str());
                            } else { // symbol is present
                                Symbol s = symbol_table.at(sym);
                                actually_used.insert(sym);
                                instr = (opcode * 1000) +  (s.position);
                                printf("%0.3d: %0.4d", count, instr);
                            }
                        } else { // error
                            printf("%0.3d: %0.4d Error: External address exceeds length of uselist; treated as immediate", count, instr);
                        }
                    }
                    else if(!strcmp(addressmode.c_str(), "R")){
                        // error_check[9]
                        if(operand < modules[module_num].codecount){ //VERIFY THIS
                            instr = (opcode * 1000) +  (operand + modules[module_num].base_address);
                            printf("%0.3d: %0.4d", count, instr);
                        } else {
                            operand = 0;
                            instr = (opcode * 1000) +  (operand + modules[module_num].base_address);
                            printf("%0.3d: %0.4d Error: Relative address exceeds module size; zero used", count, instr);
                        }
                    }

                } else { // error
                    instr = 9999;
                    opcode = instr / 1000;
                    operand = instr % 1000;
                    if(!strcmp(addressmode.c_str(), "I")) printf("%0.3d: %0.4d Error: Illegal immediate value; treated as 9999", count, instr);
                    else printf("%0.3d: %0.4d Error: Illegal opcode; treated as 9999", count, instr);
                }
                printf("\n");
                count++;
            }

            // error_check[7] - in uselist but not actually used
            for(int i = 0; i < uselist.size(); i++){
                if(actually_used.find(uselist[i]) == actually_used.end()){
                    printf("Warning: Module %d: %s appeared in the uselist but was not actually used\n", module_num + 1, uselist[i].c_str());
                }
            }
            module_num++;
        }
    }

    // (warning) error_check[4]
    for(map<string, Symbol>::iterator i = symbol_table.begin(); i != symbol_table.end(); ++i){
        if(!(*i).second.in_use_list){ 
            printf("Warning: Module %d: %s was defined but never used\n", (*i).second.module_num, (*i).second.txt.c_str());
        }
    }

    fclose(fptr);
}

int main(int argc, char *argv[]){

    if(argc==1) printf("\nNo Extra Command Line Argument Passed Other Than Program Name"); 
    char *filename = argv[1];

    pass1(filename);
    pass2(filename);

    return 0;
}