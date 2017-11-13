# ![Sieve Assembler (SIASM)](icons/siasm_title.png) 
A work in progress; at the moment it will assemble all TS1000 compatible instructions. However, it is not yet a "true" assembler since it does not yet include label support.

## Files
* Assembler program files currently have the extension .bda
  * // Uses C++ style line comments. 
  * Will work with any amount of spacing between arguments. 
  
* z80.tpl is a binary file that contains laws that assembler programs must abide by.
  * Files within the directory called bda_template_gen are used to generate the z80.tpl file from a mysql database.
  * TPL files begins with five bytes: siasm
  * The sixth byte is the file version.
  * The seventh byte is the number of instruction mnemonics.
  * Subsequent data are formatted as rows of data corresponding to a specific instruction mnemonic.
    * First byte of a row are flags, only the first two rightmost bits are currently used. They indicate mnemonic length where 00 => 2 and 11 => 5.
    * Next two to five bytes are literal mnemonic spellings. 
    * The next byte is the number of argument combinations.
    * The following data are the combinations of arguments that determine input legality and the corresponding values used for exporting programs as raw data. They are formatted as (arg1, arg2, value, prefix.)

## Tasks
* Add label support - Preprocessor already collects labels, so now the assembler just needs to be able calculate the addresses.
* Export - ASCII, binary, and EightyOne emulator snapshot or memory block. Right now it only exports to console.

## Compiling
* For simplicity, I use Orwell Dev-C++ to compile on Windows.
* On GNU/Linux, a makefile is provided for compiling with the GNU C++ Compiler. 

This program is available to you as free software licensed under the GNU General Public License (GPL-3.0-or-later)