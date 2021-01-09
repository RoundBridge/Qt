#include <iostream>
using namespace std;

extern "C" {
#include <libavcodec/avcodec.h>
}

#pragma comment(lib, "avcodec.lib")

int main() {
	cout << avcodec_configuration() << endl;
	return 0;
}