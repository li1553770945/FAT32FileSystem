#pragma once
#include "FAT32.h"
#include "utils.h"
#include "error.h"
#include <iostream>
const Uint64 SECTIONSIZE = 512;
const Uint64 CLUSTEREND = 0x0FFFFFFF;
using namespace std;

int FileNode::Read(char* buffer, int size)
{
	return Vfs->Read(this, buffer, size);
}
int FileNode::Seek(int size)
{
	return Vfs->Seek(this, size);
}





const char* FAT32::FileSystemName()
{
	return "FAT32";
}


int FAT32::ReadRawData(int lba,int offset, int size, unsigned char* buffer)
{
	unsigned char buffer_temp[SECTIONSIZE];
	ErrorType error = file.Read(lba, buffer_temp);
	if (error == 0)
	{
		return error;
	}
	memcpy(buffer, buffer_temp + offset, size);
	return  ERR_None;
}
int FAT32::Init()
{
	cout << "initing fat32 file system..." << endl;
	if (file.Init() != 0)
	{
		return -1;
	}
	unsigned char buffer[SECTIONSIZE];
	file.Read(0, buffer);
	if (!((Uint8)buffer[510] == 0x55 && (Uint8)buffer[511] == 0xAA) )
	{
		return -1;
	}
	DBRLba = (buffer[0x1c9] << 24) | (buffer[0x1c8] << 16) | (buffer[0x1c7] << 8) | (buffer[0x1c6]);
	cout <<hex<< "DBR_lba:" << DBRLba << endl;


	file.Read(DBRLba, buffer); //buffer��DBR��������

	Dbr.BPBSectionPerClus = buffer[0x0d];
	Dbr.BPB_rsvd_sec_cnt = (buffer[0x0f] << 8) | buffer[0x0e];
	Dbr.BPB_FAT_num = buffer[0x10];
	Dbr.BPB_hiden_section_num = (buffer[0x1f] << 24) | (buffer[0x1e] << 16) | (buffer[0x1d] << 8) | (buffer[0x1c]);
	Dbr.BPB_section_per_FAT_area = (buffer[0x27] << 24) | (buffer[0x26] << 16) | (buffer[0x25] << 8) | (buffer[0x24]); //FAT����С

	cout << hex << "BPB reserved section count:" << Dbr.BPB_rsvd_sec_cnt<<endl;
	cout << hex << "BPB FAT number:" << Dbr.BPB_FAT_num << endl;
	cout << hex << "BPB hiden section num:" << Dbr.BPB_hiden_section_num << endl;
	cout << hex << "BPB FAT section num:" << Dbr.BPB_section_per_FAT_area << endl;
	FAT1Lba = DBRLba + Dbr.BPB_rsvd_sec_cnt; //FAT1 = DBR + ��������
	FAT2Lba = FAT1Lba + Dbr.BPB_section_per_FAT_area; //FAT2 = FAT1 + FAT����С
	RootLba = FAT1Lba + Dbr.BPB_section_per_FAT_area * Dbr.BPB_FAT_num;
	cout << hex<<"FAT1_lba:" << FAT1Lba << " FAT2_lba:" << FAT2Lba << endl;
	cout << hex << "root lba:" << RootLba << endl;
	return ERR_None;
}

FileNode* FAT32::FindFile(const char* path, const char* name)
{
	return nullptr;
}
int FAT32::GetAllFileIn(const char* path, char* result[], int bufferSize)
{

	return 0;

}
ErrorType FAT32::CreateDirectory(const char* path)
{
	return 0;

}
ErrorType FAT32::CreateFile(const char* path)
{
	return 0;

}
ErrorType FAT32::Move(const char* src, const char* dst)
{
	return 0;

}
ErrorType FAT32::Copy(const char* src, const char* dst)
{
	return 0;
}
ErrorType FAT32::Delete(const char* path)
{

	return 0;
}
FileNode* FAT32::Open(const char* path)
{
	return 0;
}
ErrorType FAT32::Close(FileNode* p)
{
	return 0;
}
FileNode* FAT32::LoadShortFileInfoFromBuffer(unsigned char * buffer) //�ӵ�lba����ƫ��offset��λ�ö�ȡ�ļ�ͷ��Ϣ
{
	
	if (buffer[0] == 0x00 || buffer[0] == 0xE5)//������λ���Ѿ���ɾ����û�������򷵻�Null
	{
		return nullptr;
	}
	
	Uint16 attr = buffer[11];
	/*if (attr & (1 << 4))
	{
		return nullptr;
	}*/
	FAT32FileNode* node = new FAT32FileNode(this);
	node->IsDir = attr & (1 << 4);
	Uint16 file_name_length = 0; //��ȡ�ļ���
	for (int i = 0; i < 8; i++)
	{
		
		if (buffer[i] == 0x20)
		{
			break;
		}
		file_name_length++;
	}

	Uint16 extend_name_length = 0;
	char* file_name;
	int total_length;
	for (int i = 0x08; i < 0x0B; i++)//��ȡ��չ��
	{
		if (buffer[i] == 0x20)
		{
			break;
		}
		extend_name_length++;
	}
	if (!node->IsDir&&extend_name_length!=0)//�����ļ��У�������չ��
	{
		
		total_length = file_name_length + extend_name_length + 1;
		file_name = new char[total_length + 1];
		memcpy(file_name, buffer, file_name_length);
		file_name[file_name_length] = '.';
		memcpy(file_name + file_name_length + 1, buffer + 0x08, extend_name_length);
	}
	else
	{
		total_length = file_name_length;
		file_name = new char[total_length + 1];
		memcpy(file_name, buffer, file_name_length);
	}

	//1. ��ֵΪ18Hʱ���ļ�������չ����Сд��
	//2. ��ֵΪ10Hʱ���ļ�����д����չ��Сд��
	//3. ��ֵΪ08Hʱ���ļ���Сд����չ����д��
	//4. ��ֵΪ00Hʱ���ļ�������չ������д��

	if (buffer[0x0C] == 0x08 || buffer[0x0C] == 0x18)
	{
		for (int i = 0; i < file_name_length; i++)
		{
			if (file_name[i] < 'Z' && file_name[i] > 'A')
			{
				file_name[i] += 32;
			}
		}
	}
	if (buffer[0x0C] == 0x10 || buffer[0x0C] == 0x18)
	{
		for (int i = file_name_length + 1; i <total_length; i++)
		{
			if (file_name[i] < 'Z' && file_name[i] > 'A')
			{
				file_name[i] += 32;
			}
		}
	}
	file_name[total_length] = '\0';

	node->SetFileName(file_name,false);
	node->nxt = nullptr;
	node->FileSize = (buffer[0x1F] << 24) | (buffer[0x1E] << 16) | (buffer[0x1D] << 8) | (buffer[0x1C]);//��ȡ�ļ���С
	node->FirstCluster = (buffer[0x15] << 24) | (buffer[0x14] << 16) | (buffer[0x1B] << 8) | (buffer[0x1A]);
	node->CurCluster = node->FirstCluster;
	//delete [] file_name; ��ֲ��ʱ��ǵü���
	return node;

}
Uint64 FAT32::GetLbaFromCluster(Uint64 cluster)
{

	return RootLba + (cluster - 2)*Dbr.BPBSectionPerClus;

}
Uint64 FAT32::GetOffsetFromCluster(Uint64 cluster)
{
	return 0;
}

FileNode* FAT32::GetFileNodesFromCluster(Uint64 cluster) // cluster��Ӧ����Ŀ¼��
{
	FAT32FileNode* head = nullptr,* cur = nullptr;
	Uint64 lba = GetLbaFromCluster(cluster);
	for (Uint32 i = 0; i < Dbr.BPBSectionPerClus; i++)
	{
		unsigned char buffer[SECTIONSIZE];
		ReadRawData(lba+i, 0, 512, buffer);
		for (Uint32 j = 0; j < SECTIONSIZE / 32; j++)
		{
			unsigned char temp[32];
			memcpy(temp, buffer + j * 32, 32);
			Uint16 attr = temp[11];
			if (attr == 0x0F)//��Ŀ¼��
			{

			}
			else //��Ŀ¼��
			{
				FAT32FileNode* p = (FAT32FileNode*)LoadShortFileInfoFromBuffer(temp);
				if (p == nullptr)
				{
					continue;
				}
				
				if (head == nullptr)
				{
					head = p;
					cur = head;
					cur->nxt = nullptr;
				}
				else
				{
					cur->nxt = (FAT32FileNode*)LoadShortFileInfoFromBuffer(temp);
					cur = cur->nxt;
					cur->nxt = nullptr;
				}
			}
		}
	}
	Uint64 nxt = GetFATContentFromCluster(cluster);
	if (nxt != CLUSTEREND)
	{

	}
	return head;
	
}
Uint64 FAT32::GetFATContentFromCluster(Uint64 cluster)
{
	Uint64 lba = FAT1Lba + cluster * 4 / SECTIONSIZE;//��Ӧ����lba
	Uint64 offset = (cluster * 4) % SECTIONSIZE;
	unsigned char buffer[4];
	ReadRawData(lba, offset, 4, buffer);
	return (buffer[3] << 24) | (buffer[2] << 16) | (buffer[1] <<8) | buffer[0];
}
FileNode* FAT32::FindFileByNameFromCluster(Uint64 cluster, const char* name)
{
	FAT32FileNode* result;
	Uint64 lba = GetLbaFromCluster(cluster);
	for (Uint32 i = 0; i < Dbr.BPBSectionPerClus; i++)
	{
		unsigned char buffer[SECTIONSIZE];
		ReadRawData(lba + i, 0, 512, buffer);
		for (Uint32 j = 0; j < SECTIONSIZE / 32; j++)
		{
			unsigned char temp[32];
			memcpy(temp, buffer + j * 32, 32);
			if (temp[0] == 0x00) //����Ŀ¼��β
			{
				return nullptr;
			}
			Uint16 attr = temp[11];
			if (attr == 0x0F)//��Ŀ¼��
			{
				result = new FAT32FileNode(this, 2);
				result->Name = new char;
				result->Name[0] = 0;
			}
			else //��Ŀ¼��
			{
				result = (FAT32FileNode*)LoadShortFileInfoFromBuffer(temp);
			}
			if (result != nullptr)
			{
				if (strcmp(name, result->Name) == 0)
				{
					return result;
				}
			}
		}
	}
	Uint64 nxt = GetFATContentFromCluster(cluster);
	if (nxt != CLUSTEREND)
	{
		return FindFileByNameFromCluster(nxt,name);
	}
	else
	{
		return nullptr;
	}
}
FileNode* FAT32::FindFileByPath(const char* path)
{
	if (path[0] != '/')
	{
		cout << "now this system only support absolute path,which means begin with /";
		return nullptr;
	}

	if (strlen(path) < 2)
	{
		return nullptr;
	}

	FAT32FileNode* node = new FAT32FileNode(this, 2);
	Uint32 index, last_index = 1;
	for (index= 1; index < strlen(path); index++)
	{
		if (path[index] == '/')
		{
			if (index == last_index)
			{
				return nullptr;
			}
			char* temp = new char[index - last_index + 1];
			memcpy(temp, path + last_index, index - last_index);
			temp[index - last_index] = '\0';
			cout << "find:" << temp << endl;
			FAT32FileNode* last_node = node;
			node = (FAT32FileNode*)FindFileByNameFromCluster(node->CurCluster,temp);
			if (node == nullptr)
			{
				return nullptr;
			}
			delete[] temp;
			delete last_node;
			if (index != strlen(path) && node->IsDir == false)
			{
				return nullptr;
			}
			last_index = index + 1;
		}
	}
	if (last_index < strlen(path)) //���һ��section
	{
		char* temp = new char[strlen(path)- last_index + 1];
		memcpy(temp, path + last_index, strlen(path) - last_index);
		temp[strlen(path) - last_index] = '\0';
		FAT32FileNode* last_node = node;
		node = (FAT32FileNode*)FindFileByNameFromCluster(node->CurCluster, temp);
		delete[] temp;
		delete last_node;
	}
	return node;
}
bool FAT32::IsExist(const char* path)
{
	return FindFileByPath(path) != nullptr;
}

int FAT32::Read(FileNode* file_node, char* buffer, int size)
{
	
	Uint64 total_has_read_size = 0;//����Ҫ��ض�ȡ���������READ����ض�ȡ����������
	Uint64 bytes_per_cluster = SECTIONSIZE * Dbr.BPBSectionPerClus;
	FAT32FileNode* node = (FAT32FileNode*)file_node;
	if (size + node->ReadSize > node->FileSize)
	{
		size = node->FileSize - node->ReadSize;
	}
	if (size == 0)
	{
		return 0;
	}

	Uint64 lba = GetLbaFromCluster(node->CurCluster) + (node->ReadSize % bytes_per_cluster) / SECTIONSIZE;//��ǰ��LBA

	while (size)
	{
		cout << "current lba:" << lba << endl;
		if (node->CurCluster == CLUSTEREND)
		{
			cout << "error:" << "try to read FAT32 from cluster end"<<endl;
			return ERR_SystemError;
		}
		Uint64 section_has_read = node->ReadSize % (SECTIONSIZE);//��ǰ�����Ѷ��ֽ�
		Uint64 section_need_read_size;//��ǰ������Ҫ��ȡ���ֽ�
		if (section_has_read + size <= SECTIONSIZE)//�����������ֱ������
		{
			section_need_read_size = size;
			size = 0;
		}
		else//��ǰ�����������㣬�������������
		{
			section_need_read_size = SECTIONSIZE - section_has_read;
			size -= section_need_read_size;
		}
		
		ReadRawData(lba, section_has_read, section_need_read_size , (unsigned char*)(buffer+total_has_read_size));
		total_has_read_size += section_need_read_size;
		node->ReadSize += section_need_read_size;

		if (node->ReadSize % SECTIONSIZE == 0)//��������Ѿ�����
		{
			lba++;
		}
		if (node->ReadSize % bytes_per_cluster == 0) //������Ѿ�����
		{
			node->CurCluster = GetFATContentFromCluster(node->CurCluster);
			lba = GetLbaFromCluster(node->CurCluster);//�´�LBA
		}
	}
	
	return total_has_read_size;
}
int FAT32::Seek(FileNode* file_node, int size)
{
	return 0;
}

FAT32FileNode::FAT32FileNode(FAT32* _vfs, Uint64 _cluster)
{
	IsDir = false;
	nxt = nullptr;
	Vfs = _vfs;
	FirstCluster = _cluster;
	CurCluster = _cluster;
	ReadSize = 0;
}

FAT32FileNode::~FAT32FileNode()
{
	
}

