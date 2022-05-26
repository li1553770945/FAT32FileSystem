#pragma once
#define _CRT_SECURE_NO_WARNINGS

#include "Types.h"
#define _X86_

#include <fileapi.h>
#include<windows.h>
class FileBaseSystem {
	HANDLE disk;
public:
	int Init()
	{
		disk = CreateFileA("\\\\.\\PhysicalDrive2", GENERIC_READ , FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		return (disk == INVALID_HANDLE_VALUE);

	}
	bool Read(int pos, unsigned char* buffer)
	{
		memset(buffer, 0, 512);
		LARGE_INTEGER li;
		li.QuadPart = pos * 0x200;//0x200 = 512,求出扇区的 字节地址，通过设置读取的地址和长度进行read
		SetFilePointer(disk, li.LowPart, &li.HighPart, FILE_BEGIN);
		DWORD count = 0; //计数
		BOOL result = ReadFile(disk, buffer, 512, &count, NULL);
		return result;
	}
};
class VirtualFileSystem;
class FileNode
{

public:
	enum :Uint64
	{
		Attri_Dir = 1ull << 0,
		Attri_Deleted	=1ull<<1,
		Attri_Link		=1ull<<2,
		Attri_VFS		=1ull<<3,
	};


protected:
	VirtualFileSystem* Vfs;//Belonging VFS
public:
	char* Name;
	Uint64 Attributes;
	Uint64 FileSize;
	Uint64 RefCount;

	inline Uint64 Size()
	{
		return FileSize;
	}
	FileNode(VirtualFileSystem* _vfs = nullptr) :Vfs(_vfs) {}
	virtual inline void SetFileName(char* name, bool outside=false)
	{
		Name = name;
	}
	virtual int Read(char* buffer, int size) ;//读size字节的数据放到buffer里面，返回实际读取的字节数
	virtual int Seek(int size);

};
class VirtualFileSystem
{
public://Path parameter in VFS is relative path to the VFS root.
	virtual const char* FileSystemName() = 0;
	virtual FileNode* FindFile(const char* path, const char* name) = 0;
	virtual int GetAllFileIn(const char* path, char* result[], int bufferSize) = 0;//if unused ,user should free the char*
	virtual ErrorType CreateDirectory(const char* path) = 0;
	virtual ErrorType CreateFile(const char* path) = 0;
	virtual ErrorType Move(const char* src, const char* dst) = 0;
	virtual ErrorType Copy(const char* src, const char* dst) = 0;
	virtual ErrorType Delete(const char* path) = 0;
	virtual FileNode* Open(const char* path) = 0;
	virtual ErrorType Close(FileNode* p) = 0;
	virtual int Read(FileNode * file_node,char* buffer,int size) = 0;
	virtual int Seek(FileNode* file_node, int size) = 0;
	virtual bool IsExist(const char* path) = 0;
};

class FAT32File :public FileNode
{
protected:


public:


};

struct PartHeader {
	Uint32 boot_partition_flag:8; // 可引导标志，0x00不可引导，0x80可引导
	Uint32 start_chs:24; // 分区起始CHS地址(CHS＝磁头、柱⾯、扇区)，起始地址
	Uint32 type:8; //分区类型
	Uint32 end_chs:24;//分区结束CHS地址
	Uint32 offset:32;//从磁盘开始到该分区开始的偏移量（分区起始LBA地址Little-endian顺序
	Uint32 size:32;//总扇区数（Little-endian顺序）
};
struct DBR {
	Uint32 BPB_rsvd_sec_cnt;  //保留扇区数⽬ 
	Uint32 BPB_FAT_num;   //此卷中FAT表数 
	Uint32 BPB_section_per_FAT_area;   //⼀个FAT表扇区数 
	Uint32 BPB_hiden_section_num; //隐藏扇区数
	Uint64 BPBSectionPerClus;//每个簇有多少个扇区
};
class FAT32 :public VirtualFileSystem
{
public:
	PartHeader PartHeader[16];
	Uint32 DBRLba;
	DBR Dbr;

	Uint32 FAT1Lba;
	Uint32 FAT2Lba;
	Uint32 RootLba;//数据区(根目录)起始lba
	FileBaseSystem file;
	FileNode* LoadShortFileInfoFromBuffer(unsigned char* buffer);
	Uint64 GetOffsetFromCluster(Uint64 cluster);
	Uint64 GetLbaFromCluster(Uint64 cluster);
	FileNode * GetFileNodesFromCluster(Uint64 cluster);//读取cluster开始的目录对应的所有目录项
	Uint64 GetFATContentFromCluster(Uint64 cluster);//读取cluster对应的FAT表中内容
	

public:
	int ReadRawData(int lba, int offset, int size, unsigned char* buffer);//从lba偏移offset字节的位置读取size字节大小的数据
	int Init();
	const char* FileSystemName() override;
	virtual FileNode* FindFile(const char* path, const char* name) override;
	virtual int GetAllFileIn(const char* path, char* result[], int bufferSize) override;//if unused ,user should free the char*
	virtual ErrorType CreateDirectory(const char* path) override;
	virtual ErrorType CreateFile(const char* path) override;
	virtual ErrorType Move(const char* src, const char* dst) override;
	virtual ErrorType Copy(const char* src, const char* dst) override;
	virtual ErrorType Delete(const char* path) override;
	virtual FileNode* Open(const char* path)override;
	virtual ErrorType Close(FileNode* p) override;
	FileNode* FindFileByNameFromCluster(Uint64 cluster, const char* name);
	FileNode* FindFileByPath(const char* path);
	bool IsExist(const char* path)override;
	virtual int Read(FileNode* file_node, char* buffer, int size) override;
	virtual int Seek(FileNode* file_node, int size) override;
};


class FAT32FileNode :public FileNode {
	
public:
	Uint64 FirstCluster; //起始簇号
	Uint64 CurCluster;//当前簇号（读取导致的向后偏移）
	FAT32FileNode* nxt;
	bool IsDir; //是否是文件夹
	Uint64 ReadSize;//已经读取的数据大小
	FAT32FileNode(FAT32* _vfs,Uint64 cluster=0);
	~FAT32FileNode();
};