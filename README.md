# Data-Cache Simulator

To compile System 1 (direct-mapped cache) first compile 
the cpp file into the object code file "sys1.o":

 	g++ -c sys1.cpp

Next link to the object code creating the executable
file "sys1":

	g++ -o sys1 sys1.o

Now the executable file, "sys1.exe", can be called to run the program:

	./sys1 <filename> <cache size(KB)> [-v ic1 ic2]

where filename is the name of the trace file to be analyzed, cache size is the size of the cache(kb), -v toggles verbose mode ic1 is the instruction count for the start of the verbose stream ic2 is the instruction count for the end of the verbose stream.

System 2 (k-way set associative cache) is compiled similarly,
but requires an additional argument a follows:

	./sys2 <filename> <cache size(KB)> <associativity> [-v ic1 ic2]

I have included two trace files: FFT.xex and mad.xex. These can be used as inputs for either of the cache systems.

Each line in a trace file represents an access to data memory.

A few lines from a sample trace file:

	0xb7fc7489: W 0xbff20468 4 0xb7fc748e

	0xb7fc748e: R 0xbff20468 4 0xb7fc748e

	0xb7fc7495: W 0xbff20478 4 0xbff204b0

	0xb7fc749e: R 0xb7fd9ff4 4 0x15f24

-The first hexadecimal integer is the address of the instruction that caused the data memory access.

-Then follows W for write, or R for read. 

-The next hexadecimal integer is the memory address for that data access.

-Then follows the number of bytes read or written.

-Finally, the data read/written is displayed. 


In the example shown, for the top line,an instruction at 0xb7fc7489 caused a data write to address 0xbff20468 of 4 bytes; 0xb7fc748e was written to memory.

