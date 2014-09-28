#ifndef __HLPACKET_H__
#define __HLPACKET_H__

#include <NetworkKit.h>

#define CATEGORY_ITEM_TYPE 3
#define BUNDLE_ITEM_TYPE 2

typedef struct{
	char	hlnz[4];
	uint32	posted;
	uint8   nlen;
}CategoryHeader;


class HLPacket :public BNetBuffer {
public:
			HLPacket(size_t size=0);
virtual		~HLPacket();
//********************************************//
void		AddUint32(uint16 type,uint32 data);
void		AddUint16(uint16 type,uint16 data);
void		AddUint8(uint16 type,uint8 data);
void		AddString(uint16 type,const char* text);
void		CreateHeader(uint32 type,uint32 trans,uint32 flag,uint32 len,uint16 data_count);
void		AddUser(uint16 type,uint16 sock,uint16 icon,uint16 color,const char* nick);
void		AddFilePath(const char* path);
void		AddFile(uint32 size
					,const char* name
					,const char type[4]
					,const char creator[4]
					,time_t modtime = 0);
void		AddFile(const char* path,bool IsSilverWing = false);
void		AddDate(uint32 type,uint32 sec);
void		path_to_hldir (const char *path);
uint16		CreateCategoryList(const char* category_path);
void		AddBundle(const char* path);
void		AddCategory(const char* path);
uint32		GetUint(uint16 len);

protected:
		
};
#endif