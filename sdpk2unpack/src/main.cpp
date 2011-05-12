
#include <stdio.h>
#include "sdpk2.hpp"

using namespace PK2Unpack;

int main(int argc, char** argv) {
	if (argc<2) {
		printf("ERROR: Archive key path required\n");
		return 1;
	}
	Archive arch(argv[1]);
	if (arch.open()) {
		arch.printInfo();
		arch.close();
	} else {
		return 1;
	}
	return 0;
}

