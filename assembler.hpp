/*==============================================================================================
	
	assembler.hpp
	Copyright 2019 Buster Schrader
	
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
	int version = 0x0;			//Template file version.
	int inst_prefix;			//Instruction prefix byte.
	int inst_value;				//Instruction value byte.
	int start_address;			//Mem location of first byte of assembled code on the foreign machine. 
	int byte_count;				//Output-byte count; increases through program execution and is used for address calculation.
	bool line_is_label;			//If the line we are on is a label, then this will be true. 
	string in_filename;			//Filename to throw into errors.
	string mnemonic;			//Identifier of instruction.
	string argument1;			//Instruction argument, usually the destination when there are two arguments.
	string argument2;			//Instruction argument, usually the source when there are two arguments.
	vector<int> outbytes;		//Assembled instructions.
	vector<label*> labels;		//Location of preprocessor's labels.

	//Goes through template file and tries to determine if the instruction is valid and if so gets its values.
	bool scan_template_file(ifstream &tpl, int inst_count, string arg1, string arg2);
	
	//Checks to see if the contents of the template file are valid and not just some random file.
	bool template_file_check(ifstream &tpl, int &inst_num, string &filename);
	
	//Attempts alternatives if a single scan cannot decide how to assemble an instruction.
	bool resolve_instruction(int &error_amount, int &line_num, ifstream &tpl, int &inst_count, string arg1, string arg2);
	
	//Sets memory addresses for each label found in the program.
	void resolve_label_addresses(int &line_num, int &next_line);
	
	//Used to compare the information from the template file with information out of the instructions.
	int table_of_arguments(string arg);
	
	//To be called when an irrecoverrable error occurs. 
	void display_error(int line_num, string err_msg);
		
	public:
		assembler();
		void read(string instruction);					//Gets information out of instruction file.
		string get_mnemonic();							//Safe access to our instruction portion variables.
		string get_argument1();
		string get_argument2();
		void clear_info_set();							//Use after done with instruction to avoid piling strings, like ldldaddld.
		void run(string instfile, string tplfile);		//Main function of the assembler. This does the work.
		void adjust_values_first_pass(string* arg);		//Changes constants to a 16-bit placeholder for template scans.
		void adjust_values_second_pass(string* arg);	//Changes constants to an 8-bit placeholder for template scans.
		void adjust_values_third_pass(string* arg);		//Changes constants to an 8-bit displacement p.h. for template scans.
		void take_label_table(vector<label*>* table);	//Gets location of label table for us to use.
};

#endif
