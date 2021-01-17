/*==============================================================================================
    
    assembler.cpp
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
    
==============================================================================================*/

#include "assembler.hpp"
#include <exception>

struct instruction_not_found : public exception
{
    const char* what() const throw() { return "could not resolve"; }
};

struct argument_out_of_range : public exception
{
    const char* what() const throw() { return "argument out of range"; }
};

struct corrupt_template : public exception
{
    const char* what() const throw() { return "template file corruption"; }
};

assembler::assembler(string instfile, string tplfile)
{
    byte_count = 0;

    filename_tpl = tplfile;
    filename_inst = instfile;
    stream_tpl.open(filename_tpl.c_str(), ios::binary|ios::in);
    stream_inst.open(filename_inst.c_str());
}

void assembler::take_label_table(vector<label*>* table)
{
    labels = *table;
}

void assembler::run()
{
    int line_number = 0;
    int error_count = 0;
    int next_label_line = 0; //we wait until we get to a label so we can give it an accurate address
    string instline;
    
    if (labels.size() > 0) //priming the system that resolves label addresses
        next_label_line = labels[0]->line;
    
    if (stream_tpl.is_open() && stream_inst.is_open())
    {	
        if (!template_file_check())
        {
            stream_tpl.close();
            stream_inst.close();
            return;
        }
        
        //begin reading user instructions
        while (getline(stream_inst, instline))
        {
            string mnemonic;
            string argument1;
            string argument2;

            line_number++;

            if (instline.length() == 0)
                continue;

            read(instline, mnemonic, argument1, argument2);
            process_label_arguments(argument1, argument2);
            resolve_label_addresses(line_number, next_label_line);

            if (!resolve_instruction(error_count, line_number, mnemonic, argument1, argument2))
                break;
        }
        
        if (error_count == 0)
        {
            for (int i = 0; i < outbytes.size(); i++)
                cout << outbytes[i] << endl;
            
            cout << "SUCCESS" << endl;
            cout << endl << "Displaying label table: " << endl;
            
            for (int i = 0; i < labels.size(); i++)
                cout << labels[i]->name << ", " << labels[i]->line << ", " << labels[i]->value << endl;
        }
        else cout << "Could not go further due to " << error_count << " error(s).";
    }
    else cout << "File(s) could not be opened to read!" << endl;
    
    stream_tpl.close();
    stream_inst.close();
}

void assembler::read(string instruction, string &mnem, string &arg1, string &arg2)
{
    char ic; //instruction character

    //run through the characters of an instruction
    for (int i = 0, j = instruction.length(), k = 0; i < j; i++)
    {
        ic = instruction[i];

        //split the instruction into a mnemonic and 0-2 arguments
        switch (k)
        {
            case 0: if (ic != ' ') mnem += ic; else k = 1; break;
            case 1: if (ic != ',') arg1 += ic; else k = 2; break;
            default: arg2 += ic; break;
        }
    }
    
    mnem = bs_util::trim(mnem);
    arg1 = bs_util::trim(arg1);
    arg2 = bs_util::trim(arg2);
}

bool assembler::template_file_check()
{
    char read_buffer;
    const short FORMAT_CHECK_SIZE = 5;
    char format_check[FORMAT_CHECK_SIZE+1];
    int version_check;
    
    //check if template file is correct
    stream_tpl.read(format_check, sizeof(char)*FORMAT_CHECK_SIZE);
    format_check[FORMAT_CHECK_SIZE] = '\0'; //null terminated string
    
    if (string(format_check) != "siasm")
    {
        cout << filename_tpl << " is not of the correct format!" << endl;
        return false;
    }
    
    //check if template file is of the correct version
    stream_tpl.get(read_buffer);
    version_check = (uchar)read_buffer;
    
    if (version_check != version)
    {
        cout << filename_tpl << " is outdated. Cannot continue!" << endl;
        return false;
    }
    
    //get instruction count from template file
    stream_tpl.get(read_buffer);
    tpl_inst_count = (uchar)read_buffer;
    
    return true; //true indicates the file passes the check
}

bool assembler::scan_template_file(string mnem, string arg1, string arg2)
{
    bool      complete = false;
    char      read_buffer;
    char      inst_byte1;
    int       name_length;
    char*     inst_name;
    
    int       arg_combo_num;
    const int ARG_BYTES = 4;
    char      arg_combo[ARG_BYTES];
    int       inst_crnt = 0;                        //current instruction
    
    stream_tpl.seekg(7,stream_tpl.beg);             //move to beginnning of search section after file version
    
    while (inst_crnt < tpl_inst_count && !complete)
    {
        stream_tpl.read(&inst_byte1, sizeof(char)); //read first byte to tell us length of instruction string
        
        //instruction name
        name_length = (inst_byte1 & 3) + 2;         //length is the rightmost two bits, 00 = 2, 10 = 4
        inst_name = new char[name_length+1];        //plus one for null terminator
        stream_tpl.read(inst_name, name_length);    //get the name characters
        inst_name[name_length] = '\0';              //end the string
        
        //argument combinations
        stream_tpl.read(&read_buffer, sizeof(char));
        arg_combo_num = (uchar)read_buffer;
        
        if (string(inst_name) == mnem)
        {
            for (int i = 0; i < arg_combo_num; i++)
            {
                stream_tpl.read(arg_combo, sizeof(char)*ARG_BYTES);
                
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
        else //skip to next instruction
            stream_tpl.seekg(4*((int)(uchar)read_buffer), stream_tpl.cur);
        
        inst_crnt++;
        delete inst_name;
    }
    
    return complete;
}

int assembler::table_of_arguments(string arg)
{
    int output = -1;
    const int table_length = 48; //this value will need to change if you update the table
    
    static const char* table[table_length] = {
        "",     "N",  "NN",  "(NN)", "DIS", "$",
        "b",    "c",  "bc",  "(bc)",
        "d",    "e",  "de",  "(de)",
        "h",    "l",  "hl",  "(hl)",
        "a",    "af", "af'", "sp",
        "(sp)", "i",  "r",   "(c)",
        "nz",   "z",  "nc",  "po",
        "pe",   "p",  "m",
        "0",    "1",  "2",   "3",    "4",   "5",  "6", "7",
        "8",    "16", "24",  "32",   "40",  "48", "56"
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

void assembler::adjust_values(int pass, string *arg)
{
    switch (pass)
    {
        case ARG_2B_CONST:
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
            else
            {
                for (int i = 0; i < labels.size(); i++)
                    is_label = (is_label || arg_copy == labels[i]->name);
                
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

            break;
        }

        case ARG_1B_CONST:
        {
            if (bs_util::can_be_one_byte_value(atoi((*arg).c_str())))
            {
                if (bs_util::is_pointer(*arg)) //eight-bit values are never pointers
                    throw instruction_not_found();
                else
                    *arg = "N";
            }
            else throw argument_out_of_range();

            break;
        }

        case ARG_1B_DISP:
        {
            if (bs_util::can_be_signed_one_byte_value(atoi((*arg).c_str())))
                *arg = "DIS";
            else
                throw argument_out_of_range();
            
            break;
        }
    }
}

bool assembler::resolve_instruction(int &error_amount, int &line_num, string mnem, string arg1, string arg2)
{
    // 32 zero bits | ABCC
    // A is
    // B is which argument-1
    // C is # of bytes
    unsigned int arg_byte_output = 8;

    string* test_arg;
    string temp_arg1 = arg1; //preserves original arguments
    string temp_arg2 = arg2;

    try
    {
        if (temp_arg1 == "" && temp_arg2 == "") //zero arguments
        {
            if (!scan_template_file(mnem, arg1, arg2))
                throw instruction_not_found();
        }
        else //one or two arguments
        {
            if (!scan_template_file(mnem, arg1, arg2))
            {
                if (bs_util::is_pointer(arg1))
                    arg1 = bs_util::remove_outer_chars(arg1);
                
                if (bs_util::is_all_numeric(arg1)) //testing to see if arg1 is a const pointer
                    test_arg = &arg1;
                else
                {
                    if (bs_util::is_pointer(arg2)) //arg1 isn't a const pointer, let's see if arg2 is
                        arg2 = bs_util::remove_outer_chars(arg2);
                    
                    if (bs_util::is_all_numeric(arg2))
                        arg_byte_output = arg_byte_output | 4; //set B to indicate arg2 as output bytes
                    
                    test_arg = &arg2;
                }

                arg_byte_output = arg_byte_output & ~8; //lower bit A to indicate we need to keep looking

                arg1 = temp_arg1;
                arg2 = temp_arg2;
                adjust_values(ARG_2B_CONST, test_arg); //substitute value with NN and try again
            }

            for (int i = 0; i < 3 && ((arg_byte_output & 8) == 0); i++)
            {
                if (!scan_template_file(mnem, arg1, arg2))
                {
                    if (i == 2) throw instruction_not_found();
                    arg1 = temp_arg1;
                    arg2 = temp_arg2;
                    adjust_values(i+1, test_arg); //substitute dummy var and try again
                }
                else //found it: raise bit A, and set C as one or two byte arg
                    arg_byte_output = arg_byte_output | (i == 0 ? (8|2) : (8|1)); 
            }
        }
    }
    catch (exception &e)
    {
        display_error(line_num, e.what(), mnem, arg1, arg2);
        error_amount++;
        return false;
    }
    
    byte_count += (arg_byte_output & 3) + 1; //number of bytes for argument plus opcode byte
    
    if (inst_prefix != 0)
    {
        outbytes.push_back(inst_prefix); //push any potential opcode prefixes
        byte_count++;
    }
    
    outbytes.push_back(inst_value); //push on the opcode
    
    if ((arg_byte_output & 3) > 0) //non-zero values indicate we need to push on the argument
    {
        //decide which argument gets pushed
        test_arg = ((arg_byte_output & 4) == 4 ? &temp_arg2 : &temp_arg1);
        
        switch (arg_byte_output & 3) //decide what to do depending on the size of the output
        {
            case 1:
                outbytes.push_back(atoi(test_arg->c_str()));
            break;
            
            case 2:
                if (bs_util::is_pointer(*test_arg))
                    *test_arg = bs_util::remove_outer_chars(*test_arg);	
                outbytes.push_back(bs_util::num_get_lsb(atoi(test_arg->c_str())));
                outbytes.push_back(bs_util::num_get_msb(atoi(test_arg->c_str())));
            break;
        }
    }
    
    return true;
}

void assembler::process_label_arguments(string &arg1, string &arg2)
{

}

void assembler::resolve_label_addresses(int &line_num, int &next_line)
{
    if (next_line > 0 && next_line == line_num)
    {
        int item = 0;
        
        while (labels[item]->line < line_num)
        {
            if (item < labels.size()-1)
                item++;
            else
            {
                next_line = 0;
                return;
            }
        }
        
        while (labels[item]->line == line_num)
        {
            labels[item]->value = byte_count;
            
            if (item < labels.size()-1)
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

void assembler::display_error(int line_num, string err_msg, string mnem, string arg1, string arg2)
{	
    cout << "Assembly error, in " << filename_inst;
    cout << " at line " << line_num << " -> " << err_msg << ' ' << mnem;
    
    if (arg1 != "")
    {
        cout << ' ' << arg1;
        
        if (arg2 != "")
            cout << ',' << arg2;
    }
    
    cout << endl;
}