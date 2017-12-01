/*
 *	sys1.cpp
 *	direct-mapped cache
 *	Author: Darrel Daquigan
 *	May , 2017
 *
 *	To compile this program using g++, first make the file using
 *  either:
 		make sys1

 			or

 		g++ -c sys1.cpp
 		g++ -o sys1 sys1.cpp

 *	Now the executable file, "sys1.exe", can be called 
 *	to run the program:
 *			./sys1 <filename> <cache size(KB)> [-v ic1 ic2]
 *	where filename is the name of the trace file to be analyzed.
 *
 */


#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <algorithm>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <iomanip>

using namespace std;


int main (int argc, char* argv[]){
	if (argc == 3 || argc == 6){
		ifstream trace(argv[1]);
		int cacheSize = strtol(argv[2], NULL, 10)*1024;

		int ic1 = -1, ic2 = -1;
		bool verbose= false;
		if (argc == 6){
			if (strcmp(argv[3], "-v") == 0){
				ic1 = atoi(argv[4]);
				ic2 = atoi(argv[5]);
				verbose = true;
			}
		}	

		int blockSize = 16;

		vector<unsigned int> insAddress;
		vector<string> accessType;
		vector<unsigned int> dataAddress;
		vector<int> dataSize;
		vector<string> data;

		string parts[5];
		string line;

		//parse the file
		while (getline(trace,line)){
			istringstream iss(line);
			if(!(iss >> parts[0] >> parts[1] >> parts[2] >> parts[3] >> parts[4]))
				break;
			
			//get instruction address
			stringstream ssIns;
			ssIns << hex<< parts[0];
			unsigned int address;
			ssIns >> address;
			insAddress.push_back(address);

			//get access type
			accessType.push_back(parts[1]);

			//get memory address
			stringstream ssMem;
			ssMem << hex << parts[2];
			unsigned int mem;
			ssMem >> mem;
			dataAddress.push_back(mem);

			//get data size
			dataSize.push_back(atoi(parts[3].c_str()));

			//get data
			data.push_back(parts[4]);

		}
		
		int reads = 0,
			writes = 0,
			accesses = 0,
			readMiss = 0,
			writeMiss = 0,
			dataMiss = 0,
			dirtyRead = 0,
			dirtyWrite = 0,
			bytesRead = 0,
			bytesWrite = 0,
			readTime = 0,
			writeTime = 0,


			offsetSize = (int)log2((double)blockSize),
			numBlocks = cacheSize/blockSize,
			indexSize = (int)log2((double)numBlocks),	
			tagSize = 32 - offsetSize - indexSize;


		//cache metadata
		vector< vector<int> > cacheMeta(numBlocks, vector<int>(3)); 
		vector< vector<string> > cacheBlock(numBlocks, vector<string>(blockSize << 2));

		for (int i = 0; i < numBlocks; i++){
			cacheMeta[i][0] = 0; //tag
			cacheMeta[i][1] = 0; //valid
			cacheMeta[i][2] = 0; //dirty
			for (int j = 0; j< (blockSize<<2); j++){
				cacheBlock[i][j] = ""; //data in block at offset j
			}
		}

		int numLines = data.size();
		stringstream verboseStream;

		for (int i = 0; i < numLines; i++){
			int index = (dataAddress[i] >> offsetSize) & (numBlocks - 1);
			int offset = dataAddress[i] & ((int)pow(2,offsetSize) - 1);
			int tag = dataAddress[i] >> (offsetSize + indexSize);

			verboseStream.str(string());
			verboseStream << dec << i << ' ';
			verboseStream << hex << index << ' ' << tag << ' ' <<
				cacheMeta[index][1] << ' ' << cacheMeta[index][0] <<
				' ' << cacheMeta[index][2] << ' ';

			accesses++;

			//cache hit
			if(cacheMeta[index][0] == tag && cacheMeta[index][1] == 1){
				verboseStream << "1 1\n";
				//read hit
				if (accessType[i].compare("R") == 0){
					reads++;
					readTime++; // cache to CPU
				}
				//write hit
				else{
					writes++;
					writeTime++; // CPU to cache
					cacheMeta[index][2] = 1;
					cacheBlock[index][offset<<2] = data[i];
				}
			}
			//cache miss
			else{
				verboseStream << "0 2";
				dataMiss++;
				cacheMeta[index][0] = tag;
				cacheMeta[index][1] = 1;
				cacheBlock[index][offset<<2] = data[i];

				//clean miss
				if(cacheMeta[index][2] == 0){
					verboseStream << "a\n";
					//clean read miss
					if(accessType[i].compare("R") == 0){
						reads++;
						readMiss++;
						readTime+= 81; 
						bytesRead+= blockSize;
						cacheMeta[index][2] = 0;
					}
					//clean write miss
					else{
						writes++;
						writeMiss++;
						writeTime+= 81; 	
						bytesRead+= blockSize;
						cacheMeta[index][2] = 1;
					}
				}
				//dirty miss
				else{
					verboseStream << "b\n";
					//dirty read miss
					if(accessType[i].compare("R") == 0){
						reads++;
						readMiss++;
						dirtyRead++;
						readTime+=161;
						bytesWrite+= blockSize;
						bytesRead+=blockSize;
						cacheMeta[index][2] = 0;
					}
					//dirty write miss
					else{
						writes++;
						writeMiss++;
						dirtyWrite++;
						writeTime+=161; 
						bytesWrite+= blockSize;
						bytesRead+=blockSize;
						cacheMeta[index][2] = 1;
					}
				}
			}
	
			if (verbose && i >= ic1 && i <= ic2)
				cout << verboseStream.str();

		}

		cout << "direct-mapped, writeback, size = " << cacheSize/1024 <<
			"KB\nloads " << reads << " stores " << writes <<
			" total " << accesses << 

			"\nrmiss " << readMiss << " wmiss " << writeMiss <<
			" total " << dataMiss <<

			"\ndirty rmiss " << dirtyRead <<" dirty wmiss " << dirtyWrite <<

			"\nbytes read " << bytesRead << " bytes written " << bytesWrite <<

			"\nread time " << readTime << " write time " << writeTime <<

			"\ntotal time " << (readTime + writeTime) <<


			"\nmiss rate " << fixed << setprecision(6) << (double)dataMiss/(double)accesses;
		
	}
	else{
		cout << "Incorrect number of arguments\nUsage: dir tracefile cachesize [-v ic1 ic2]";
	}
}


