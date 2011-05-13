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
#include "sdpk2.hpp"
#include "sdmd2.hpp"

using namespace PK2Unpack;

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
			pak.close();
			pak.printInfo(0, true);
		} else {
			return 1;
		}
	} else {
		printf("File extension not recognized\n");
	}
	return 0;
}

