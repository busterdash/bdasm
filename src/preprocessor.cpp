/*==============================================================================================
    
    preprocessor.cpp
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

#include "preprocessor.hpp"
#include <fstream>
#include <iostream>

preprocessor::preprocessor(string file)
{
    string line;
    ifstream procstream;
    procstream.open(file.c_str());
    
    errors_exist = false;
    line_num_in = 0;
    line_num_out = 1;
    filename = file;
    
    if (procstream.is_open())
    {
        while (getline(procstream, line))
        {
            line_num_in++;
            line = bs_util::trim(line);
            if (filter_comments(line)) continue;
            if (process_includes(line)) continue;
            if (process_labels(line)) continue;
            out = out + line + '\n';
            line_num_out++;
        }
    }
    else
    {
        cout << "File(s) could not be opened to read!" << endl;
        errors_exist = true;
    }
    
    procstream.close();
}

bool preprocessor::filter_comments(string &line)
{
    string buffer;
    
    if (line.length() > 0) //in this block we look for comments and strip them out
    {
        if (line[0] == '/' && line[1] == '/')
            return true;
        
        for (int i = 0; i < line.length()-1; i++) //get everything upto potential comments
        {
            if (line[i] == '/' && line[i+1] == '/')
                break;
            
            buffer += line[i];

            //add last character that is otherwise skipped
            if (i == line.length()-2)
                buffer += line[i+1]; 
        }
        
        line = bs_util::trim_right(buffer);
    }

    //true indicates the input line is a comment and should be skipped, false otherwise
    return (line.length() == 0);
}

bool preprocessor::process_includes(string &line)
{
	string path;
    
    if (line[0] != '#')
        return false;
    
    if (line.length() < 11 || line.substr(0,9) != "#inject <" || line[line.length()-1] != '>')
    {
        display_error(line_num_in, "unknown or malformed preprocessor command");
        return false;
    }
    
    //unwrap the candy bar
    path = bs_util::remove_outer_chars(line.substr(8,string::npos)); 
    
    if (path == filename)
    {
        display_error(line_num_in, "included file cannot be source file");
        return false;
    }
    
    preprocessor* pr = new preprocessor(path);
    out += pr->export_to_str(); //append included file to ours
    
    //since we're injecting, we need to adjust line numbers of upstream labels
    //to reflect the values of our current file
    for (int i = 0; i < pr->labels.size(); i++)
    {
        pr->labels[i]->line += line_num_out - 1; 
        //minus one accounts for the fact we number our lines starting at one
    }
    
    line_num_out += pr->line_num_out - 1; //take child's line out count and append
    
    //create space and add included labels
    labels.reserve(labels.size() + pr->labels.size());
    labels.insert(labels.end(), pr->labels.begin(), pr->labels.end());
    
    //pass down whether it was successful upstream
    if (pr->errors_exist) errors_exist = true; 
    
    delete pr;
    return true;
}

bool preprocessor::process_labels(string &line)
{
    bool already_exists = false;
    string label_name;
    label* l;
    
    if (line.length() > 1 && line[0] == '.')
    {
        label_name = line.substr(1,string::npos);
        
        for (int i = 0; i < labels.size(); i++) //check to see our new found line isn't already defined
        {
            if (labels[i]->name == label_name)
                display_error(line_num_out, "duplicate labels, " + label_name + " is already in use.");
        }
        
        if (bs_util::is_var_name(label_name)) //add our label to the list if it is properly defined
        {
            l = new label();
            l->name = label_name;
            l->line = line_num_out;
            labels.push_back(l);
            return true;
        }
        else display_error(line_num_out, "incorrect label format");
    }
    
    return false;
}

string preprocessor::export_to_str()
{
    return out;
}

void preprocessor::export_to_file(string file)
{
    ofstream outstream;
    outstream.open(file.c_str());
    
    if (outstream.is_open())
        outstream << out << endl;
    
    outstream.close();
}

void preprocessor::display_error(int line_num, string err_msg)
{
    cout << "Preprocess error, in " << filename;
    cout << " at line " << line_num << " -> " << err_msg << endl;
    errors_exist = true;
}

void preprocessor::cleanup()
{
    for (int i = 0; i < labels.size(); i++)
        delete labels[i];
}
