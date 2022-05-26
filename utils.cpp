#include "utils.h"
#include <iostream>
#include <iomanip>
#include "Types.h"
using namespace std;
void DataWithSize(char* buffer, int size,int width)
{

    for (int i = 0; i < size; i++)
    {
        //cout << uppercase << hex << setfill('0') << setw(2) << ((unsigned short)buffer[i] >> 8) << " " << endl;
        unsigned char mask = 1 << 7;
        unsigned char temp = buffer[i];
        cout <<  setfill('0')<<setw(width) << i << ":";
        cout << uppercase << setfill('0') << hex << setw(2) << (int)temp << " " << endl;

    }
    cout << endl;
}

void ReverseBits(Uint64& num, Uint64 bits_cnt)
{

}

void PrintBits(Uint64 num, Uint64 bits_cnt)
{
    Uint64 mask = 1ull << (bits_cnt - 1);
    for (int i = 0; i < bits_cnt; i++)
    {
        cout << ((num & mask) >> (bits_cnt - 1 - i));
        mask >>= 1;
    }
    cout << endl;
}