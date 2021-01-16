<?php

/*==============================================================================================
    
    binary_builder.php
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

class instruction
{
    private $flags;
    private $mnemonic;
    private $arg_combo = array();
    private $arg_combo_num;
    
    function __construct($name)
    {
        $this->mnemonic = $name;
        $this->flags = strlen($name)-2;
        $this->arg_combo_num = 0;
    }
    
    function add_arg_combo($combo)
    {
        array_push($this->arg_combo, $combo);
        $this->arg_combo_num++;
    }
    
    function write_instruction($file)
    {
        fwrite($file, chr($this->flags));
        
        //write the letters in the instruction mnemonic
        for ($i = 0; $i < strlen($this->mnemonic); $i++)
            fwrite($file, $this->mnemonic[$i]);
        
        fwrite($file, chr($this->arg_combo_num));
        
        //write all the argument combinations
        foreach ($this->arg_combo as $combo)
            fwrite($file,$combo->export());
    }
}

class argument_combo
{
    private $argument1;
    private $argument2;
    private $value;
    private $prefix;
    
    private $table = array(
        "",   "N",    "NN", "(NN)", "DIS", ":",
        "b",  "c",    "bc", "(bc)", "d",   "e",  "de",  "(de)",
        "h",  "l",    "hl", "(hl)", "a",   "af", "af'",
        "sp", "(sp)", "i",  "r",    "(c)",
        "nz", "z",    "nc", "po",   "pe",  "p",  "m",
        "0",  "1",    "2",  "3",    "4",   "5",  "6",   "7",    "8",
        "16", "24",   "32", "40",   "48",  "56"
    );
    
    function __construct($arg1, $arg2, $val, $pfx)
    {
        $this->argument1 = $this->table_of_arguments($arg1);
        $this->argument2 = $this->table_of_arguments($arg2);
        $this->value = $val;
        $this->prefix = $pfx;
    }
    
    function table_of_arguments($arg)
    {
        $output = -1;
        
        for ($i = 0; $i < sizeof($this->table); $i++)
        {
            if ($arg == $this->table[$i])
            {
                $output = $i;
                break;
            }
        }
        
        if ($output == -1)
            echo "WARNING! Found an argument that doesn't exist!<br />\r\n";
        
        return $output;
    }
    
    function export()
    {
        return (
            chr($this->argument1)
            . chr($this->argument2)
            . chr($this->value)
            . chr($this->prefix)
        );
    }
}

function tokenize_user_inst($instruction, &$mnemonic, &$argument1, &$argument2)
{
    $inst_char_iter = 0;
    $read_stage = 0;
    $ic = "";

    $instruction = trim($instruction);
    
    //run through the characters of an instruction
    while ($inst_char_iter < strlen($instruction))
    {
        $ic = $instruction[$inst_char_iter];
        
        if ($read_stage == 0) //read in the mnemonic of the instruction
        {
            if ($ic != ' ')
                $mnemonic .= $ic;
            else
                $read_stage = 1;
        }
        else if ($read_stage == 1) //read in the first argument of the instruction
        {
            if ($ic != ',')
                $argument1 .= $ic;
            else
                $read_stage = 2;
        }
        else //read in the second argument of the instruction
            $argument2 .= $ic;
            
        $inst_char_iter++;
    }
    
    $argument1 = trim($argument1);
    $argument2 = ltrim($argument2);
}

function starting_procedure($filename)
{
    $inst_array = array();
    $inst_count = 0;
    
    $servername = "localhost";
    $username =   "root";
    $password =   "";
    $dbname =     "z80";
    
    $mnemonic =   "";
    $argument1 =  "";
    $argument2 =  "";
    
    $version =    0;
    $inst_count = 0;
    
    $conn = new mysqli($servername,$username,$password,$dbname);
    
    if ($conn->connect_error)
        die("Failed connection: " . $conn->connect_error);
    
    $res = $conn->query(
        "SELECT DISTINCT substring(`mnemonic`,1,LOCATE(' ',`mnemonic`)) AS `allinst`
        FROM `z80`.`instructions`
        UNION
        SELECT `mnemonic` as `allinst`
        FROM `z80`.`instructions`
        WHERE LOCATE(' ',`mnemonic`) = 0"
    );
    
    if ($res->num_rows > 0)
    {
        while ($row = $res->fetch_assoc())
            array_push($inst_array, rtrim($row["allinst"]));
        
        $inst_count = $res->num_rows;
    }
    
    $ptr = fopen($filename,"wb");
    fwrite($ptr, "siasm");
    fwrite($ptr, chr($version));
    fwrite($ptr, chr($inst_count-1)); //-1 is a quick way to get rid of effect of having a blank row
    
    for ($i = 1; $i < $inst_count; $i++) //starts at one because first row is empty for some reason
    {
        $obj = new instruction($inst_array[$i]);
        $item_len = strlen($inst_array[$i]);
        
        //Not going to worry about SQL injection, this is not part of a publicly-accessible page.
        //I don't care about speed, just want my damned file.
        $res = $conn->query(
            "SELECT *
            FROM `instructions`
            WHERE SUBSTRING(`mnemonic`,1,$item_len) = '$inst_array[$i]'
            AND (
                LENGTH(`mnemonic`) = $item_len
                OR SUBSTRING(`mnemonic`, ".($item_len+1).",1) = ' '
            )
            ORDER BY `prefix_byte`, `code`"
        );
        
        if ($res->num_rows > 0)
        {
            while ($row = $res->fetch_assoc())
            {
                $mnemonic = "";
                $argument1 = "";
                $argument2 = "";
                tokenize_user_inst($row["mnemonic"],$mnemonic,$argument1,$argument2);
                $arg = new argument_combo($argument1,$argument2,$row["code"],$row["prefix_byte"]);
                $obj->add_arg_combo($arg);
            }
        }
        
        $obj->write_instruction($ptr);
    }
    
    fclose($ptr);
}

starting_procedure("z80.tpl");

?>