/*==============================================================================================
    
    assembler.hpp
    Copyright 2019-2021 Buster Schrader
    
    This file is part of SIASM.
    
    SIASM is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    any later version.
    
    SIASM is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    
    You should have received a copy of the GNU General Public License
    along with SIASM.  If not, see <https://www.gnu.org/licenses/>.
    
==============================================================================================

    Assembler
    9/5/17 - B.D.S.
    Purpose: Reads instructions and converts them to values.
    
==============================================================================================*/

#ifndef _ASSEMBLER_HPP
#define _ASSEMBLER_HPP

#include <fstream>
#include <iostream>
#include <vector>
#include "bs_util.hpp"
using namespace std;

class assembler
{
    int version = 0x0;     //template file version
    int inst_prefix;       //instruction prefix byte
    int inst_value;        //instruction value byte
    int start_address;     //mem location of first byte of assembled code on the foreign machine
    int byte_count;        //output-byte count; increases through program execution and is used for address calculation
    bool line_is_label;    //if the line we are on is a label, then this will be true
    string in_filename;    //filename to throw into errors
    string mnemonic;       //identifier of instruction
    string argument1;      //instruction argument, usually the destination when there are two arguments
    string argument2;      //instruction argument, usually the source when there are two arguments
    vector<int> outbytes;  //assembled instructions
    vector<label*> labels; //location of preprocessor's labels

    //goes through template file and tries to determine if the instruction is valid and if so gets its values
    bool scan_template_file(ifstream &tpl, int inst_count, string arg1, string arg2);
    
    //checks to see if the contents of the template file are valid and not just some random file
    bool template_file_check(ifstream &tpl, int &inst_num, string &filename);
    
    //attempts alternatives if a single scan cannot decide how to assemble an instruction
    bool resolve_instruction(int &error_amount, int &line_num, ifstream &tpl, int &inst_count, string arg1, string arg2);
    
    //sets memory addresses for each label found in the program
    void resolve_label_addresses(int &line_num, int &next_line);
    
    //used to compare the information from the template file with information out of the instructions
    int table_of_arguments(string arg);
    
    //to be called when an irrecoverrable error occurs
    void display_error(int line_num, string err_msg);
        
    public:
        assembler();
        void read(string instruction);                //gets information out of instruction file
        string get_mnemonic();                        //safe access to our instruction portion variables
        string get_argument1();
        string get_argument2();
        void clear_info_set();                        //use after done with instruction to avoid piling strings, like ldldaddld
        void run(string instfile, string tplfile);    //main function of the assembler, this does the work
        void adjust_values_first_pass(string* arg);   //changes constants to a 16-bit placeholder for template scans
        void adjust_values_second_pass(string* arg);  //changes constants to an 8-bit placeholder for template scans
        void adjust_values_third_pass(string* arg);   //changes constants to an 8-bit displacement p.h. for template scans
        void take_label_table(vector<label*>* table); //gets location of label table for us to use
};

#endif
