/*==============================================================================================
	
	assembler.cpp
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
	
==============================================================================================*/

#include "assembler.hpp"
#include <exception>

struct instruction_not_found : public exception
{
	const char* what() const throw()
	{
		return "could not resolve";
	}
};

struct argument_out_of_range : public exception
{
	const char* what() const throw()
	{
		return "argument out of range";
	}
};

struct corrupt_template : public exception
{
	const char* what() const throw()
	{
		return "template file corruption";
	}
};

assembler::assembler()
{
	byte_count = 0;
	clear_info_set();
}

void assembler::display_error(int line_num, string err_msg)
{	
	cout << "Assembly error, in " << in_filename;
	cout << " at line " << line_num << " -> " << err_msg << ' ' << mnemonic;
	
	if (argument1 != "")
	{
		cout << ' ' << argument1;
		if (argument2 != "") cout << ',' << argument2;
	}
	
	cout << endl;
}
		
void assembler::read(string instruction)
{
	int inst_char_iter = 0;
	int read_stage = 0;
	char ic;

	//Please trim input before calling.
	//instruction = bs_util::trim(instruction);
	
	//Run through the characters of an instruction.	
	while (inst_char_iter < instruction.length())
	{
		ic = instruction[inst_char_iter];
		
		if (read_stage == 0) //Read in the mnemonic of the instruction.
		{
			if (ic != ' ')
				mnemonic += ic;
			else
				read_stage = 1;
		}
		else if (read_stage == 1) //Read in the first argument of the instruction.
		{
			if (ic != ',')
				argument1 += ic;
			else
				read_stage = 2;
		}
		else //Read in the second argument of the instruction.
			argument2 += ic;
			
		inst_char_iter++;
	}
	
	argument1 = bs_util::trim(argument1);
	argument2 = bs_util::trim_left(argument2);
}

string assembler::get_mnemonic() { return mnemonic; }
string assembler::get_argument1() { return argument1; }
string assembler::get_argument2() { return argument2; }

void assembler::clear_info_set()
{
	mnemonic = "";
	argument1 = "";
	argument2 = "";
	line_is_label = false;
}

void assembler::run(string instfile, string tplfile)
{
	int tpl_inst_count;
	int line_number = 0;
	int error_count = 0;
	int next_label_line = 0; //We wait until we get to a label so we can give it an accurate address.
	
	string instline;
	ifstream tplstream;
	ifstream inststream;
	tplstream.open(tplfile.c_str(), ios::binary|ios::in);
	inststream.open(instfile.c_str());
	in_filename = instfile;
	
	if (labels.size() > 0) //Priming the system that resolves label addresses.
	{
		next_label_line = labels[0]->line;
	}
	
	if (tplstream.is_open() and inststream.is_open())
	{	
		if (template_file_check(tplstream, tpl_inst_count, tplfile))
		{
			tplstream.close();
			inststream.close();
			return;
		}
		
		//Begin reading user instructions.
		while (getline(inststream, instline))
		{
			line_number++;
			instline = bs_util::trim(instline);
			if (instline.length() == 0) continue;
			read(instline);
			resolve_label_addresses(line_number, next_label_line);
			if (resolve_instruction(error_count, line_number, tplstream, tpl_inst_count, argument1, argument2)) break;
			clear_info_set();
		}
		
		if (error_count == 0)
		{
			for (int i = 0; i < outbytes.size(); i++)
			{
				cout << outbytes[i] << endl;
			}
			
			cout << "SUCCESS" << endl;
			
			cout << endl << "Displaying label table: " << endl;
			
			for (int i = 0; i < labels.size(); i++)
			{
				cout << labels[i]->name << ", " << labels[i]->line << ", " << labels[i]->value << endl;
			}
		}
		else
		{
			cout << "Could not go further due to " << error_count << " error(s).";
		}
	}
	else
	{
		cout << "File(s) could not be opened to read!" << endl;
	}
	
	tplstream.close();
	inststream.close();
}

bool assembler::resolve_instruction(int &error_amount, int &line_num, ifstream &tpl, int &inst_count, string arg1, string arg2)
{
	string* test_arg;
	int arg_byte_output = 0; //28x0 0ABB where A is which argument-1, and B is # of bytes.
	
	try
	{
		if (argument1 == "" && argument2 == "") //Zero arguments
		{
			if (!scan_template_file(tpl, inst_count, arg1, arg2))
				throw instruction_not_found();
		}
		else if (argument1 != "" && argument2 == "") //One argument
		{
			if (!scan_template_file(tpl, inst_count, arg1, arg2))
			{
				adjust_values_first_pass(&arg1); //Substitute value with NN and try again.
				
				if (!scan_template_file(tpl, inst_count, arg1, arg2))
				{
					arg1 = argument1;
					adjust_values_second_pass(&arg1); //Substitute value with N and try again.
					
					if (!scan_template_file(tpl, inst_count, arg1, arg2))
					{
						adjust_values_third_pass(&arg1);
						
						if (!scan_template_file(tpl, inst_count, arg1, arg2))
							throw instruction_not_found();
						else
							arg_byte_output = 1;
					}
					else arg_byte_output = 1;
				}
				else arg_byte_output = 2;
			}
		}
		else //Two arguments
		{
			if (!scan_template_file(tpl, inst_count, arg1, arg2))
			{	
				if (bs_util::is_pointer(arg1))
				{
					arg1 = bs_util::remove_outer_chars(arg1);
				}
				
				if (bs_util::is_all_numeric(arg1)) //Testing to see if arg1 is a const pointer.
				{
					test_arg = &arg1;
				}
				else
				{
					if (bs_util::is_pointer(arg2)) //arg1 isn't a const pointer, let's see if arg2 is.
					{
						arg2 = bs_util::remove_outer_chars(arg2);
					}
					
					if (bs_util::is_all_numeric(arg2))
						arg_byte_output = 4; //Set to arg2 output mode.
					
					test_arg = &arg2;
				}
				
				arg1 = argument1;
				arg2 = argument2;
				
				adjust_values_first_pass(test_arg); //Substitute value with NN and try again.

				if (!scan_template_file(tpl, inst_count, arg1, arg2))
				{
					arg1 = argument1;
					arg2 = argument2;
					adjust_values_second_pass(test_arg); //Substitute value with N and try again.
					
					if (!scan_template_file(tpl, inst_count, arg1, arg2))
					{
						adjust_values_third_pass(test_arg);
						
						if (!scan_template_file(tpl, inst_count, arg1, arg2))
							throw instruction_not_found();
						else
							arg_byte_output = arg_byte_output | 1;
					}
					else arg_byte_output = arg_byte_output | 1;
				}
				else arg_byte_output = arg_byte_output | 2;
			}
		}
	}
	catch (exception &e)
	{
		display_error(line_num,e.what());
		error_amount++;
		return true;
	}
	
	byte_count += (arg_byte_output & 3) + 1; //Number of bytes for argument plus opcode byte.
	
	if (inst_prefix != 0)
	{
		outbytes.push_back(inst_prefix); //Push any potential opcode prefixes.
		byte_count++;
	}
	
	outbytes.push_back(inst_value); //Push on the opcode.
	
	if ((arg_byte_output & 3) != 0) //Non-zero values indicate we need to push on the argument.
	{
		if ((arg_byte_output & 4) == 4) //Decide which argument gets pushed.
			test_arg = &argument2;
		else
			test_arg = &argument1;
		
		switch (arg_byte_output & 3) //Decide what to do depending on the size of the output.
		{
			case 1:
			{
				outbytes.push_back(atoi(test_arg->c_str()));
				break;
			}
			
			case 2:
			{
				if (bs_util::is_pointer(*test_arg))
					*test_arg = bs_util::remove_outer_chars(*test_arg);	

				outbytes.push_back(bs_util::num_get_lsb(atoi(test_arg->c_str())));
				outbytes.push_back(bs_util::num_get_msb(atoi(test_arg->c_str())));
				break;
			}
		}
	}
	
	return false;
}

bool assembler::scan_template_file(ifstream &tpl, int inst_count, string arg1, string arg2)
{
	bool complete = false;
	char read_buffer;
	char inst_byte1;
	int name_length;
	char* inst_name;
	
	int arg_combo_num;
	const int ARG_BYTES = 4;
	char arg_combo[ARG_BYTES];
	int inst_crnt = 0; //Current instruction.
	
	tpl.seekg(7,tpl.beg); //Move to beginnning of search section after file version.
	
	while (inst_crnt < inst_count && !complete)
	{
		tpl.read(&inst_byte1,sizeof(char)); //Read first byte to tell us length of instruction string.
		
		//Instruction name.
		name_length = (inst_byte1 & 3) + 2; //Length is the rightmost two bits, 00 = 2, 10 = 4.
		inst_name = new char[name_length+1]; //Plus one is for null character.
		tpl.read(inst_name,name_length); //Get the name characters. 
		inst_name[name_length] = '\0'; //End the string.
		
		//Argument combinations.
		tpl.read(&read_buffer,sizeof(char));
		arg_combo_num = (uchar)read_buffer;
		
		if (string(inst_name) == mnemonic)
		{
			for (int i = 0; i < arg_combo_num; i++)
			{
				tpl.read(arg_combo,sizeof(char)*ARG_BYTES);
				
				if ((int)(uchar)arg_combo[0] == table_of_arguments(arg1))
				{
					if ((int)(uchar)arg_combo[1] == table_of_arguments(arg2))
					{
						inst_value = (int)(uchar)arg_combo[2];
						inst_prefix = (int)(uchar)arg_combo[3];
						complete = true;
						break;
					}
				}
			}
		}
		else
		{
			tpl.seekg(4*((int)(uchar)read_buffer),tpl.cur); //Skip to next instruction.
		}
		
		inst_crnt++;
		delete inst_name;
	}
	
	return complete;
}

int assembler::table_of_arguments(string arg)
{
	int output = -1;
	const int table_length = 48; //This value will need to change if you update the table.
	
	static const char* table[table_length] = {
		"","N","NN","(NN)","DIS","$",
		"b","c","bc","(bc)",
		"d","e","de","(de)",
		"h","l","hl","(hl)",
		"a","af","af'","sp",
		"(sp)","i","r","(c)",
		"nz","z","nc","po",
		"pe","p","m",
		"0","1","2","3","4","5","6","7",
		"8","16","24","32","40","48","56"
	};
	
	for (int i = 0; i < table_length; i++)
	{
		if (arg == table[i])
		{
			output = i;
			break;
		}
	}
	
	return output;
}

bool assembler::template_file_check(ifstream &tpl, int &inst_num, string &filename)
{
	char read_buffer;
	const short FORMAT_CHECK_SIZE = 5;
	char format_check[FORMAT_CHECK_SIZE+1];
	int version_check;
	
	//Check if template file is correct. 
	tpl.read(format_check,sizeof(char)*FORMAT_CHECK_SIZE);
	format_check[FORMAT_CHECK_SIZE] = '\0'; //Null terminated string.
	
	if (string(format_check) != "siasm")
	{
		cout << filename << " is not of the correct format!" << endl;
		return true;
	}
	
	//Check if template file is of the correct version.
	tpl.get(read_buffer);
	version_check = (uchar)read_buffer;
	
	if (version_check != version)
	{
		cout << filename << " is outdated. Cannot continue!" << endl;
		return true;
	}
	
	//Get instruction count from template file.
	tpl.get(read_buffer);
	inst_num = (uchar)read_buffer;
	
	return false; //False indicates the file passes the check.
}

void assembler::adjust_values_first_pass(string *arg)
{
	bool is_pointer = false;
	bool is_label = false;
	string arg_copy = *arg;
	
	if (bs_util::is_pointer(*arg))
	{
		arg_copy = bs_util::remove_outer_chars(*arg);
		is_pointer = true;
	}
	
	if (bs_util::is_all_numeric(arg_copy))
	{
		if (bs_util::can_be_two_byte_value(atoi((arg_copy).c_str())))
		{
			if (is_pointer)
				*arg = "(NN)";
			else
				*arg = "NN";
		}
		else throw argument_out_of_range();
	}
	else //throw instruction_not_found();
	{
		for (int i = 0; i < labels.size(); i++)
		{
			if (arg_copy == labels[i]->name)
			{
				is_label = true;
			}
		}
		
		if (is_label)
		{
			cout << "Found a label here." << endl;
			
			if (is_pointer)
				*arg = "(NN)";
			else
				*arg = "NN";	
		}
		else throw instruction_not_found();
	}
}

void assembler::adjust_values_second_pass(string *arg)
{
	if (bs_util::can_be_one_byte_value(atoi((*arg).c_str())))
	{
		if (bs_util::is_pointer(*arg)) //Eight-bit values are never pointers.
			throw instruction_not_found();
		else
			*arg = "N";
	}
	else throw argument_out_of_range();
}

void assembler::adjust_values_third_pass(string *arg)
{
	if (bs_util::can_be_signed_one_byte_value(atoi((*arg).c_str())))
	{
		*arg = "DIS";
	}
	else throw argument_out_of_range();
}

void assembler::take_label_table(vector<label*>* table)
{
	labels = *table;
}

void assembler::resolve_label_addresses(int &line_num, int &next_line)
{
	if (next_line > 0 && next_line == line_num)
	{
		int item = 0;
		
		while (labels[item]->line < line_num)
		{
			if (item < labels.size()-1) //Minus one prevents overflow.
				item++;
			else
			{
				next_line = 0;
				return;
			}
		}
		
		while (labels[item]->line == line_num)
		{
			labels[item]->value = byte_count + 1;
			//Plus one puts our location at the byte after the label instead of before.
			
			if (item < labels.size()-1) //Minus one prevents overflow.
				item++;
			else
			{
				next_line = 0;
				return;
			}
		}
		
		next_line = labels[item]->line;
	}
}

