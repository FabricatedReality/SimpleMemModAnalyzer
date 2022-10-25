# SimpleMemModAnalyzer
Analyzes the memory diagram of a file on the Linux system, then prints it out in the command prompt

This program first attempts to read the file given by CLA

If the file exist, this program will read each line of it as well as keeping its next line in mind.
The program stores all data in different linked lists for printing in format after reading the entire file.
Each linked list will store a string, the strings will be in the format:
   var_name   scope   var_type   var_size
The linked list for functions are different, they are in the format:
   func_name num_lines num_variables

If the next line starts with {, then it is a function, and the program will change the current scope,
and store all the arguments in the static data linked list
If a line contains an alloc function, the program will determine the name of the pointer to the malloc,
using the name it will find the data type by matching the variable names with data previously stored in the program

The program will check the first word of each line for the data type, if there is no data type, go to the next line
When there is a data type, the entire line will be that data type
The program then get the names of each variable
if there are quotes in the line, then the program checks for string literals and stores them

The program have a string during the loop that holds the scope string, the program also keep count of how many curly brackets there are, with { = +1 and } = -1 the program can easily check if the function reached its end or not. 
The default scope is "global", and it changes with what function the program is currently reading in
