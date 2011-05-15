/*
Copyright (c) 2011 Tim Howard

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#include <stdio.h>
#include <string>
#include <duct/filestream.hpp>
#include "sdpk2.hpp"
#include "sdmd2.hpp"

using namespace PK2Unpack;

char __hash_str[33]={0x00};
const char* __hash_str_ptr=NULL;

void dump_entry(SDPK2& pak, const Entry& entry, const char* outdir, const char* outpath) {
	std::string path(outdir);
	path.append(outpath);
	printf("Dumping [%.*s] to %s\n", 32, __hash_str_ptr, path.c_str());
	FileStream* out=FileStream::writeFile(path.c_str());
	if (out) {
		if (entry.readToStream(pak.getStream(), out, pak)!=0) {
			printf("\tFailed to decompress/write some blocks\n");
		}
		out->close();
		delete out;
	} else {
		printf("\tFailed to open %s for writing\n", path.c_str());
	}
}

int main(int argc, char** argv) {
	if (argc<2) {
		printf("ERROR: sdpk2/sdmd2 path required\n");
		return 1;
	}
	const char* path=argv[1];
	size_t len=strlen(path);
	if (strncmp((path+len)-5, "sdmd2", 5)==0) {
		SDMD2 table(path);
		if (table.load()) {
			table.printInfo();
		} else {
			return 1;
		}
	} else if (strncmp((path+len)-5, "sdpk2", 5)==0) {
		SDPK2 pak(path);
		if (pak.open()) {
			//pak.printInfo(0, true);
			if (argc>2) {
				__hash_str_ptr=argv[2];
				if (argc>3) {
					path=argv[3];
				} else {
					path=argv[2];
				}
				MD5Hash hash;
				if (strncmp(__hash_str_ptr, "-a", 2)==0) {
					__hash_str_ptr=__hash_str;
					if (strncmp(path, "-a", 2)==0) {
						path="dump/";
					}
					const EntryVec& entries=pak.getEntries();
					for (size_t i=0; i<entries.size(); ++i) {
						entries[i].hash().getExisting(__hash_str, false);
						dump_entry(pak, entries[i], path, __hash_str_ptr);
					}
				} else if (hash.set(__hash_str_ptr)) {
					const Entry* entry=pak.findEntry(hash);
					if (entry) {
						dump_entry(pak, *entry, "dump/", path);
					} else {
						printf("Entry [%s] not found\n", __hash_str_ptr);
						return 1;
					}
				} else {
					printf("Malformed hash\n");
					return 1;
				}
			}
			pak.close();
		} else {
			return 1;
		}
	} else {
		printf("File extension not recognized\n");
	}
	return 0;
}

