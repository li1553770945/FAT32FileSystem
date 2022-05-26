#define _X86_
#define _CRT_SECURE_NO_WARNINGS

#include <algorithm>
#include <iostream>
#include "FAT32.h"
#include <fileapi.h>
#include<windows.h>
#include <iomanip>
#include "utils.h"

using namespace std;


int main()
{
    FAT32 fat32;
    if (fat32.Init())
    {
        cout << "fat32 init fail!";
        return 0;
    }
   /* FileBaseSystem file;
    file.Init();*/


    unsigned char buffer[1000000];
    if (fat32.ReadRawData(0,0, 512,buffer))
    {
        cout << "read fail" << GetLastError()<<endl;
        return 0;
    }
    fat32.file.Read(fat32.PartHeader[0].offset, buffer);
    //DataWithSize(buffer, 512);
    FAT32FileNode* node;

    node = (FAT32FileNode *)fat32.FindFileByPath("/folder/cyx.txt");
    if (node == nullptr)
    {
        return 0;
    }
    int size = node->FileSize;
    cout << dec <<fat32.GetLbaFromCluster(node->CurCluster)<<" file size:" << dec << node->FileSize << endl;
    while (size)
    {
        int read_size = node->Read((char*)buffer, 260);
        for (int i = 0; i < read_size; i++)
        {
            cout << buffer[i];
        }
        cout << endl;
        size -= read_size;
    }
    
    return 0;
}
