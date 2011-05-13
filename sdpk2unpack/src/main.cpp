
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

