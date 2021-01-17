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

#define ARG_2B_CONST 0
#define ARG_1B_CONST 1
#define ARG_1B_DISP  2

#include <fstream>
#include <iostream>
#include <vector>
#include "bs_util.hpp"
using namespace std;

class assembler
{
    int version = 0x0;     //template file version
    string filename_tpl;   //filename of template for displaying errors
    ifstream stream_tpl;   //file stream for template
    int tpl_inst_count;    //number of instructions available in the template file
    
    string filename_inst;  //filename of source file for displaying errors
    ifstream stream_inst;  //file stream for instructions

    int inst_prefix;       //instruction prefix byte
    int inst_value;        //instruction value byte
    int start_address;     //mem location of first byte of assembled code on the foreign machine
    int byte_count;        //output-byte count; increases through program execution and is used for address calculation
    bool line_is_label;    //if the line we are on is a label, then this will be true
    vector<int> outbytes;  //assembled instructions
    vector<label*> labels; //location of preprocessor's labels

    //gets information out of instruction file
    void read(string instruction, string &mnem, string &arg1, string &arg2);

    //checks to see if the contents of the template file are valid and not just some random file
    bool template_file_check();

    //goes through template file and tries to determine if the instruction is valid and if so gets its values
    bool scan_template_file(string mnem, string arg1, string arg2);

    //used to compare the information from the template file with information out of the instructions
    int table_of_arguments(string arg);

    //changes constants to a specified placeholder for template scans
    void adjust_values(int pass, string* arg);

    //attempts alternatives if a single scan cannot decide how to assemble an instruction
    bool resolve_instruction(int &error_amount, int &line_num, string mnem, string arg1, string arg2);
    
    //sets memory addresses for each label found in the program
    void resolve_label_addresses(int &line_num, int &next_line);

    //to be called when an irrecoverrable error occurs
    void display_error(int line_num, string err_msg, string mnem, string arg1, string arg2);

    public:
        assembler(string instfile, string tplfile);
        void take_label_table(vector<label*>* table); //gets location of label table for us to use
        void run();                                   //main function of the assembler, this does the work
};

#endif
