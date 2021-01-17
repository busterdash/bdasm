/*==============================================================================================
    
    bs_util.hpp
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

    Binary and String Utilities
    4/6/17 - B.D.S.
    Purpose: Provides binary interface and input cleansing.
    
==============================================================================================*/

#ifndef _BS_UTIL_HPP
#define _BS_UTIL_HPP

#include <string>
using namespace std;

typedef unsigned char uchar;

struct label
{
    string name;
    int    line;  //line number of first instruction after label in the combined file
    int    value; //memory address that the label represents
};

namespace bs_util
{
    int    num_get_msb(int value);                  //returns left byte of 16-bit integer, big-endian
    int    num_get_lsb(int value);                  //returns right byte of 16-bit integer, big-endian
    int    num_get_msb(string value);               //see above
    int    num_get_lsb(string value);					
    bool   can_be_two_byte_value(int value);			
    bool   can_be_one_byte_value(int value);			
    bool   can_be_signed_one_byte_value(int value);	
    string trim_left(string input);                 //trims only spaces from the left side of a string
    string trim_right(string input);                //trims only spaces from the right side of a string
    string trim(string input);                      //trims only spaces from both sides of a string
    bool   is_var_name(string input);               //returns true if input follows variable naming convention
    bool   is_all_numeric(string input);            //returns true if all characters are numeric
    bool   is_pointer(string input);                //returns true if surrounded by parenthesis
    int    quad_str_to_int(string input);           //turns a string of four characters into an int
    string remove_non_numerics(string input);       //removes all characters that are not numeric
    string remove_outer_chars(string input);        //removes just outer characters
}

#endif
