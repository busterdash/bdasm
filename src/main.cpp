/*==============================================================================================
    
    main.cpp
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

#include <iostream>
#include <string>
#include "assembler.hpp"
#include "preprocessor.hpp"

using namespace std;

int main(int argc, char* argv[])
{
    string tpl = "z80.tpl";
    string input_file = "testfile.bda";
    
    if (argc == 2)
        input_file = string(argv[1]);
    
    preprocessor* pr = new preprocessor(input_file);
    pr->export_to_file(input_file + ".combined");
    
    cout << "//// DISPLAYING PREPROCESSOR LABELS ////" << endl;
    
    for (int i = 0; i < pr->labels.size(); i++)
        cout << pr->labels[i]->name << " @ " << pr->labels[i]->line << endl;
    
    cout << endl << "////  DISPLAYING ASSEMBLER RESULTS  ////" << endl;
    
    if (!pr->errors_exist)
    {
        assembler* ir = new assembler();
        ir->take_label_table(&pr->labels);
        ir->run(input_file + ".combined", tpl);
        delete ir;
    }
    
    pr->cleanup();
    delete pr;
    return 0;
}
