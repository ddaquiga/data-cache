/*
 *	sys2.cpp
 *	k-way set-associative cache
 *	Author: Darrel Daquigan
 *	May , 2017
 *
 *	To compile this program using g++, first make the file using
 *  either:
 		make sys2

 			or

 		g++ -c sys2.cpp
 		g++ -o sys2 sys2.cpp
 		
 *	Now the executable file, "sys1.exe", can be called 
 *	to run the program:
 *		./sys1 <filename> <cache size(KB)> <associativity> [-v ic1 ic2]
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
	if (argc == 4 || argc == 7){
		ifstream trace(argv[1]);
		int cacheSize = strtol(argv[2], NULL, 10)*1024;
		int setAssoc = strtol(argv[3],NULL,10);

		int ic1 = -1, ic2 = -1;
		bool verbose= false;
		if (argc == 7){
			if (strcmp(argv[4], "-v") == 0){
				ic1 = atoi(argv[5]);
				ic2 = atoi(argv[6]);
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
			numBlocks = cacheSize/(blockSize * setAssoc),
			indexSize = (int)log2((double)numBlocks),	
			tagSize = 32 - offsetSize - indexSize;


		//cache metadata
		vector< vector < vector<int> > >cacheMeta(setAssoc, vector< vector<int> >(numBlocks, vector<int>(4)));

		vector< vector < vector<string> > >cacheBlock(setAssoc, vector< vector<string> >(numBlocks, vector<string>(blockSize << 2)));

		for (int k = 0; k < setAssoc; k++){
			for (int i = 0; i < numBlocks; i++){
				cacheMeta[k][i][0] = 0; //tag
				cacheMeta[k][i][1] = 0; //valid
				cacheMeta[k][i][2] = 0; //dirty
				cacheMeta[k][i][3] = 0; //last used
				for (int j = 0; j< (blockSize<<2); j++){
					cacheBlock[k][i][j] = ""; //data in block at offset j
				}
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
			verboseStream << hex << index << ' ' << tag << ' ';

			accesses++;
			bool hasEmpty = false,
				hit = false,
				clean = true;
			int hitId = -1;

			//check empty and hit
			for(int k = 0; k < setAssoc; k++){
				if(cacheMeta[k][index][1] == 0)
					hasEmpty = true;
				if (cacheMeta[k][index][1] == 1 && cacheMeta[k][index][0] == tag){
					hit = true;
					hitId = k;
				}
			}

			// cache hit
			if (hit){
				verboseStream << cacheMeta[hitId][index][1] << ' ' <<
					hitId << ' ';
				verboseStream << dec << cacheMeta[hitId][index][3] << ' ';
				verboseStream << hex << cacheMeta[hitId][index][0] << ' ' << 
					cacheMeta[hitId][index][2] << " 1 1\n";

				cacheMeta[hitId][index][3] = i;

				//read hit
				if(accessType[i].compare("R")==0){
					reads++;
					readTime++;
				}

				//write hit
				else{
					writes++;
					writeTime++;
					cacheMeta[hitId][index][2] = 1;
					cacheBlock[hitId][index][offset<<2] = data[i];
				}
			}

			// cache miss
			else{
				dataMiss++;

				//choose Id to replace
				int missId = -1,
					count = 0;

				// choose first empty Id
				if(hasEmpty){
					while(missId == -1){
						if (cacheMeta[count][index][1] == 0){
							missId = count;
						}
						count++;
					}

				}

				// choose least recently used
				else{
					int lowest = cacheMeta[0][index][3];
					missId = 0;
					for (int k = 0; k< setAssoc; k++){
						if (cacheMeta[k][index][3] < lowest){
							lowest = cacheMeta[k][index][3];
							missId = k;
						}
					}
				}

				verboseStream << cacheMeta[missId][index][1] << ' ' <<
					missId << ' ' << cacheMeta[missId][index][3] << ' ' <<
					cacheMeta[missId][index][0] << ' ' << 
					cacheMeta[missId][index][2] << " 0 2";

				cacheMeta[missId][index][0] = tag;
				cacheMeta[missId][index][1] = 1;
				cacheMeta[missId][index][3] = i;
				cacheBlock[missId][index][offset<<2] = data[i];

				//clean miss
				if (cacheMeta[missId][index][2] == 0){
					verboseStream << "a\n";
					//clean read miss
					if(accessType[i].compare("R") == 0){
						reads++;
						readMiss++;
						readTime+= 81;
						bytesRead+= blockSize;
						cacheMeta[missId][index][2] = 0;
					}
					//clean write miss
					else{
						writes++;
						writeMiss++;
						writeTime+= 81;
						bytesRead+= blockSize;
						cacheMeta[missId][index][2] = 1;
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
						readTime+= 161;
						bytesWrite+= blockSize;
						bytesRead+= blockSize;
						cacheMeta[missId][index][2] = 0;
					}

					//dirty write miss
					else{
						writes++;
						writeMiss++;
						dirtyWrite++;
						writeTime+= 161;
						bytesWrite+= blockSize;
						bytesRead += blockSize;
						cacheMeta[missId][index][2] = 1;
					}
				}

			}

			if (verbose && i >= ic1 && i <= ic2)
				cout << verboseStream.str();
		}

		cout << setAssoc << "-way, writeback, size = " << cacheSize/1024 <<
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
		cout << "Incorrect number of arguments\nUsage: set tracefile cachesize associativity [-v ic1 ic2]";
	}
}


