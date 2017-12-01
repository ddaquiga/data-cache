# Data-Cache Simulator

To compile System 1 (direct-mapped cache) first compile 
the cpp file into the object code file "sys1.o":
 	g++ -c sys1.cpp

Next link to the object code creating the executable
file "sys1":
	g++ -o sys1 sys1.o

Now the executable file, "sys1.exe", can be called to run the program:
	./sys1 <filename> <cache size(KB)> [-v ic1 ic2]
where filename is the name of the trace file to be analyzed,
cache size is the size of the cache(kb),
-v toggles verbose mode
ic1 is the instruction count for the start of the verbose stream
ic2 is the instruction count for the end of the verbose stream

System 2 (k-way set associative cache) is compiled similarly,
but requires an additional argument a follows:
	./sys2 <filename> <cache size(KB)> <associativity> [-v ic1 ic2]

I have included two trace files: FFT.xex and mad.xex. These can be used as inputs for either of the cache systems.