#include <Path.h>
#include <File.h>
#include <Entry.h>
#include <NodeInfo.h>
#include <Directory.h>
#include <iostream>
#include <string.h>
#include <Debug.h>

#include "HLPacket.h"
#include "hl_magic.h"

/**************************************************************
 * Constructor.
 **************************************************************/
HLPacket::HLPacket(size_t size):BNetBuffer(size)
{	
}

/**************************************************************
 * Destructor.
 **************************************************************/
HLPacket::~HLPacket()
{
}


/**************************************************************
 * Create Hotline packet header.
 **************************************************************/
void
HLPacket::CreateHeader(uint32 type,uint32 trans,uint32 flag,uint32 len,uint16 data_count)
{
	this->AppendUint32(type);
	this->AppendUint32(trans);
	this->AppendUint32(flag);
	this->AppendUint32(len);
	this->AppendUint32(len);
	this->AppendUint16(data_count);
}

/**************************************************************
 * Add uint32 to HLPacket.
 **************************************************************/
void
HLPacket::AddUint32(uint16 type,uint32 data)
{
	this->AppendUint16(type);
	this->AppendUint16(4);
	this->AppendUint32(data);
}

/**************************************************************
 * Add uint16 to HLPacket.
 **************************************************************/
void
HLPacket::AddUint16(uint16 type,uint16 data)
{
	this->AppendUint16(type);
	this->AppendUint16(2);
	this->AppendUint16(data);
}

/**************************************************************
 * Add uint8 to HLPacket.
 **************************************************************/
void
HLPacket::AddUint8(uint16 type,uint8 data)
{
	this->AppendUint16(type);
	this->AppendUint16(1);
	this->AppendUint8(data);
}

/**************************************************************
 * Add string to HLPacket
 **************************************************************/
void
HLPacket::AddString(uint16 type,const char* data)
{
	uint16 len = 0;
	if(data != NULL)
		len = ::strlen(data);
	this->AppendUint16(type);
	if(len == 0)
	{
		this->AppendUint16(2);
		this->AppendUint16(0);
	}else{
		this->AppendUint16(len);
		// You must not call AppendString. It include NULL char at the end of data.
		//this->AppendString(data);
		this->AppendData(data,len);
	}
}

/**************************************************************
 * Add user to HLPacket.
 **************************************************************/
void
HLPacket::AddUser(uint16 type,uint16 sock,uint16 icon,uint16 color,const char* nick)
{
	this->AppendUint16(type);
	this->AppendUint16(8 + strlen(nick));
	this->AppendUint16(sock);
	this->AppendUint16(icon);
	this->AppendUint16(color);
	this->AppendUint16(strlen(nick));
	this->AppendData(nick,strlen(nick));	
}

/**************************************************************
 * Add local file info to HLPacket
 **************************************************************/
void
HLPacket::AddFile(const char* path,bool IsSilverWing)
{
	off_t size;
	uint32 size32;
	BPath filepath(path);
	BFile file(filepath.Path(),B_READ_ONLY);
	BEntry entry(filepath.Path());
	if(file.InitCheck() == B_OK)
	{
		file.GetSize(&size);	
		size32 = size;
		char mime[B_MIME_TYPE_LENGTH];
		char type[5];
		type[4] = '\0';
		BNodeInfo ninfo(&file);
		ninfo.InitCheck();
		time_t modtime = 0;
		if(IsSilverWing)
			entry.GetModificationTime(&modtime);
		
		if(ninfo.GetType(mime) != B_OK)
		{
			BEntry entry(filepath.Path());
			if(entry.IsDirectory() == false)
				::strcpy(type,"????");
			else{
				BDirectory dir( &entry );
				dir.InitCheck();
   				
   				size32 = dir.CountEntries();	
				::strcpy(type,"fldr");
			}
		}else{
			BMimeType mimeType(mime);
			BMimeType superType;
			mimeType.GetSupertype(&superType);
			if(::strcmp(superType.Type(),"text") == 0)
				::strcpy(type,"TEXT");
			else if(::strcmp(mimeType.Type(),"application/zip")== 0)
				::strcpy(type,"ZIP");
			else if(::strcmp(superType.Type(),"audio") == 0)
				::strcpy(type,"MP3");
			else if(::strcmp(superType.Type(),"image") == 0)
				::strcpy(type,"BMP");
			else if(::strcmp(mimeType.Type(),"application/x-vnd.Be-directory") == 0)
			{
				::strcpy(type,"fldr");
				BEntry entry(filepath.Path());
   				BDirectory dir( &entry );
				dir.InitCheck();
   				
   				size32 = dir.CountEntries();
			}
		}
		PRINT(("Type: %s\n", type ));
		this->AddFile(size32,filepath.Leaf(),type,"????",modtime);
	}
}

/**************************************************************
 * Add file to HLPacket.
 **************************************************************/
void
HLPacket::AddFile(uint32 size
				,const char* name
				,const char type[4]
				,const char creator[4]
				,time_t modtime)
{
	uint32 nlen = strlen(name);
	uint32 data_len = nlen + 20;
	if(modtime)
		data_len += 4;
	this->AppendUint16(HTLS_DATA_FILE_LIST);

	this->AppendUint16(data_len);
	this->AppendData(type,4);
	this->AppendData(creator,4);
	
	this->AppendUint32(size);	
	this->AppendUint32(0);
	if(modtime)
		this->AppendUint32(modtime);
	this->AppendUint32(nlen);
	this->AppendData(name,nlen);
}

/**************************************************************
 * Add file path to HLPacket
 **************************************************************/
void
HLPacket::AddFilePath(const char* kPath)
{
	register char const *p, *p2;
	uint16 pos = 2, dc = 0;
	register uint8 nlen;
	BNetBuffer data;
	if(data.InitCheck() != B_OK)
	{
		PRINT(( "No memory... Could not convert path.\n" ));
		return;
	}
	p = kPath;
	while ((p2 = strchr(p, '/'))) {
		if (!(p2 - p)) {
			p++;
			continue;
		}
		nlen = (uint8)(p2 - p);
		pos += 3 + nlen;
		char *tmp = new char[nlen+1];
		::memcpy(tmp,p,nlen);
		tmp[nlen] = '\0';
		data.AppendUint16(0);
		data.AppendUint8(nlen);
		data.AppendString(tmp);
		delete[] tmp;
		dc++;
		p = &(p2[1]);
	}
	this->AppendUint16(HTLC_DATA_DIR);
	this->AppendUint16(pos);
	this->AppendUint16(dc);	
	this->AppendData(data.Data(),data.Size());
}

/***********************************************************
 * Create category list packet.
 ***********************************************************/
uint16
HLPacket::CreateCategoryList(const char* path)
{
	BDirectory dir( path );
   	status_t err = B_NO_ERROR;
   	BEntry entry;
   	uint16 hc = 0;
	while( err == B_NO_ERROR )
	{
		err = dir.GetNextEntry( &entry, true );			
		if( entry.InitCheck() != B_NO_ERROR )
			break;
		BPath filepath;
		if( entry.IsDirectory() )
		{
			AddBundle(BPath(&entry).Path());
		}else{
			AddCategory(BPath(&entry).Path());
		}
		hc++;
	}
	return hc;
}

/***********************************************************
 * Add bundle (type2)
 ***********************************************************/
void
HLPacket::AddBundle(const char* path)
{
	char name[B_FILE_NAME_LENGTH+1];
	BNode node(path);
	if(node.InitCheck() != B_OK)
		return;
	
	::strcpy(name,BPath(path).Path());
	uint16 posted = 0;
	uint16 len = strlen(name) + 6;
	AppendUint16(HTLS_DATA_NEWS_CATEGORYITEM);
	AppendUint16(len);
	AppendUint16(2);
	AppendUint16(posted);
	
}

/***********************************************************
 * Add category (type3)
 ***********************************************************/
void
HLPacket::AddCategory(const char* path)
{
	char name[B_FILE_NAME_LENGTH+1];
	BNode node(path);
	if(node.InitCheck() != B_OK)
		return;
	
	::strcpy(name,BPath(path).Path());
	uint16 posted = 0;
	uint16 len = strlen(name) + 6;
	AppendUint16(HTLS_DATA_NEWS_CATEGORYITEM);
	AppendUint16(len);
	AppendUint16(2);
	AppendUint16(posted);
	
}

/***********************************************************
 * Add create date
 ***********************************************************/
void
HLPacket::AddDate(uint32 type,uint32 sec)
{
	AppendUint16(type);
	AppendUint16(8); 	//data len
	AppendUint16(1970); //base_year
	AppendUint16(0); 	// pad
	AppendUint32(sec); 	// sec
}

/**************************************************************
 * Get integer from HLPacket.
 **************************************************************/
uint32 
HLPacket::GetUint(uint16 len)
{
	uint32 result = 0;
	if(len == 4)
		this->RemoveUint32(result);
	else if(len == 2)
	{
		uint16 tmp;
		this->RemoveUint16(tmp);
		result = tmp;
	}else{
		char* tmp = new char[len+1];
		this->RemoveData(tmp,len);
		delete[] tmp;
	}
	return result;
}