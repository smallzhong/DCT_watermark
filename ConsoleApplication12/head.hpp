#pragma once
#include <iostream>
#include <fstream>
#include <bitset>
#include <string>
#include <time.h>

using namespace std;

//extern "C" int getopt(int argc, char* const* argv, const char* optstring);

typedef union _LARGE_INTEGER {
    struct {
        unsigned int LowPart;
        int HighPart;
    } DUMMYSTRUCTNAME;
    struct {
        unsigned int LowPart;
        int HighPart;
    } u;
    long long QuadPart;
} LARGE_INTEGER;

#define EXIT_ERROR(x)                                 \
	do                                                \
	{                                                 \
		cout << "error in file " << __FILE__ << " in function " << __FUNCTION__ << " in line " << __LINE__ << endl; \
		cout << x;                                    \
		getchar();                                    \
		exit(EXIT_FAILURE);                           \
	} while (0)
