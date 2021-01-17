/*==============================================================================================
    
    bs_util.cpp
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

#include "bs_util.hpp"

int bs_util::num_get_msb(int value)
{
    value = value & 65280;
    value = value >> 8;
    return value;
}

int bs_util::num_get_lsb(int value)
{
    value = value & 255;
    return value;
}

int bs_util::num_get_msb(string value)
{
    return num_get_msb(atoi(remove_non_numerics(value).c_str()));
}

int bs_util::num_get_lsb(string value)
{
    return num_get_lsb(atoi(remove_non_numerics(value).c_str()));
}

bool bs_util::can_be_two_byte_value(int value)
{
    return (value >= -32768 && value < 65536);
}

bool bs_util::can_be_one_byte_value(int value)
{
    return (value >= -128 && value < 256);
}

bool bs_util::can_be_signed_one_byte_value(int value)
{
    return (value >= -128 && value < 128);
}

string bs_util::trim_left(string input)
{
    int iter = 0;
    char c = input[0];
    bool start_collecting = false;
    string output = "";
    
    if (input == "") return input;
    
    while (iter < input.length())
    {
        if (!start_collecting)
        {
            if (c != ' ' && c != '\t')
            {
                start_collecting = true;
                output += c;
            }
        }
        else output += c;
        
        iter++;
        c = input[iter];
    }
    
    return output;
}

string bs_util::trim_right(string input)
{
    int iter = 0;
    int len = input.length();
    char c = input[len-1];
    bool start_collecting = false;
    string output = "";

    if (input == "") return input;
    
    while (iter < len)
    {
        if (!start_collecting)
        {
            if (c != ' ' && c != '\t')
            {
                start_collecting = true;
                output = c + output;
            }
        }
        else output = c + output;
        
        iter++;
        c = input[len-iter-1];
    }
    
    return output;
}

string bs_util::trim(string input)
{
    input = trim_left(input);
    input = trim_right(input);
    return input;
}

bool bs_util::is_var_name(string input)
{
    int i = 0;
    bool output = true;

    if (input.length() < 1) output = false;
    
    while (i < input.length())
    {
        short c = short(input[i]);
        bool alpha =   ((c > 64 && c <= 90) || (c > 96 && c <= 122));
        bool numeric = (c > 47 && c <= 57);
        bool symbol =  (c == 95); //underscore
        
        if (!(alpha || (i > 0 && (symbol || numeric))))
        {
            output = false;
            break;
        }
        
        i++;
    }
    
    return output;
}

bool bs_util::is_all_numeric(string input)
{
    int i = 0;
    bool output = true;
    
    if (input.length() < 1) output = false;
    
    while (i < input.length())
    {
        short c = short(input[i]);
        
        //do not trip over negative sign or null char
        if ((c <= 47 || c > 57) && !(i == 0 && c == 45))
        {
            output = false;
            break;
        }
        
        i++;
    }
    
    return output;
}

bool bs_util::is_pointer(string input)
{
    return (input[0] == '(' && input[input.length()-1] == ')');
}

int bs_util::quad_str_to_int(string input)
{
    int len = 4;
    int c[4];
    c[0] = 0;
    c[1] = 0;
    c[2] = 0;
    c[3] = 0;
    
    if (input.length() > 0)
    {
        if (input.length() < len)
            len = input.length();

        c[0] = int(input[0]);
        
        for (int i = 1; i < len; i++)
            c[i] = int(input[i]) << i*8;
    }
    
    return c[3] | c[2] | c[1] | c[0];	
}

string bs_util::remove_non_numerics(string input)
{
    string output = "";
    
    for (unsigned int i = 0; i < input.length(); i++)
    {
        if ((int)input[i] > 47 && (int)input[i] <= 57)
            output += input[i];
    }
    
    return output;
}

string bs_util::remove_outer_chars(string input)
{
    return input.substr(1,input.length()-2);
}
