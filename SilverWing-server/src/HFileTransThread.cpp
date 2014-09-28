#include <iostream>
#include <File.h>
#include <stdio.h>
#include <Mime.h>
#include <NodeInfo.h>
#include <Debug.h>

#include "HFileTransThread.h"
#include "TServer.h"
#include "hl_magic.h"
#include "hx_types.h"
#include "dhargs.h"
#include "HApp.h"
#include "TextUtils.h"
#include "HPrefs.h"

#define BUFFER_SIZE 16384
#define XFER_BUFSIZ	(0xffff * 4)

/**************************************************************
 * Constructor.
 **************************************************************/
HFileTransThread::HFileTransThread(const char* filename
							,BNetEndpoint *endpoint
							,uint32 data_pos
							,bool isDownload
							,BLooper *target)
	:LThread(filename,B_LOW_PRIORITY)
	,fIsDownload(isDownload)
	,fData_pos(data_pos)
	,fFilename(filename)
	,fEndpoint(endpoint)
	,fTarget(target)
{
}

/**************************************************************
 * Destructor.
 **************************************************************/
HFileTransThread::~HFileTransThread()
{
	PRINT(("Deleted file trans class\n"));
	delete fEndpoint;
}

/**************************************************************
 * Thread main function.
 **************************************************************/
int32
HFileTransThread::Main()
{
	if(fIsDownload)
		this->Download();
	else
		this->Upload();
	BMessage msg(T_REMOVE_TRANS);
	msg.AddPointer("pointer",this);
	fTarget->PostMessage(&msg);
	return 0;
}

/**************************************************************
 * Download.
 **************************************************************/
void
HFileTransThread::Download()
{
	BFile file(fFilename.String(),B_READ_ONLY);
	uint32 data_pos;
	int32 len,slen,tot_len;
	char buf[BUFFER_SIZE];

	
	if(file.InitCheck() != B_OK)
	{
		PRINT(("Not such a file.\n"));
		return;
	}
	off_t file_size;
	file.GetSize(&file_size);

	// fData_pos is remained data size.
	// Data_pos = file_size - data_pos;
	data_pos = file_size - fData_pos;
	PRINT(("data_pos: %d\n",data_pos));
	
	// if user's file is bigger than server's file.
	if(file_size < fData_pos)
	{
		fEndpoint->Send("",0);
		data_pos = 0;
		goto end;
	}
	
	file.Seek(data_pos,SEEK_SET);
	
	while(fData_pos > 0)
	{
		::memset(buf,0,BUFFER_SIZE);
		len = file.Read(buf,BUFFER_SIZE > fData_pos ? fData_pos:BUFFER_SIZE);
		
		slen = 0;
		tot_len = len;
		while(len > 0)
		{
			if( this->Lock() )
			{
				slen = fEndpoint->Send(buf,len);	
				this->Unlock();
				if(slen <= 0)
					goto end;			
			}
			len -= slen;
		}
		fData_pos -= tot_len;
	}
end:
	fEndpoint->Close();
	PRINT(("End download\n"));
	return;
}


/**************************************************************
 * Upload.
 **************************************************************/
void
HFileTransThread::Upload()
{
	/******* Convert to UTF8 ************/
	int32 encoding;
	((HApp*)be_app)->Prefs()->GetData("encoding",&encoding);
	TextUtils utils;
	if(encoding>0)
		utils.ConvertToUTF8(fFilename,encoding);
	/*************************************/
	BFile file(fFilename.String(),B_WRITE_ONLY|B_CREATE_FILE);
	uint32 data_pos;
	off_t file_size;
	uint32 offset = 0;
	file.GetSize(&file_size);
	int32 len;
	char buf[BUFFER_SIZE];
	BMimeType mime;
		
	if(file.InitCheck() != B_OK)
	{
		PRINT(("Cound not create file.\n"));
		return;
	}

	char *header = new char[XFER_BUFSIZ+1];
	if (ReceiveData(header, 40) != 40)
			return;
	int r = header[39] + 12;
	if (ReceiveData(header, r) != r)
		return;
	if (ReceiveData(header, 4) != 4)
		return;
	::memcpy(&data_pos,header,4);
	data_pos = ntohl(data_pos);
	
	delete[] header;
	PRINT(("data_pos: %d fData_pos: %d\n",data_pos,fData_pos ));
	offset = data_pos - fData_pos;
	if(file_size != 0&&file_size < fData_pos)
		file.Seek(fData_pos,SEEK_SET);
	PRINT(("Seek pos: %d\n",offset));
	
	while(offset > 0)
	{
		::memset(buf,0,BUFFER_SIZE);
		len = ReceiveData(buf,BUFFER_SIZE > offset ? offset:BUFFER_SIZE);
		if(len == B_ERROR)
			goto end;
		if(len == 0)
			goto end;
		offset -= file.Write(buf,len);
	}
	// set mime type
	if(BMimeType::GuessMimeType(fFilename.String(),&mime) == B_OK)
	{
		BNodeInfo ninfo(&file);
		ninfo.SetType(mime.Type());
	}
end:
	file.Sync();
	fEndpoint->Close();
	delete fEndpoint;
	fEndpoint = NULL;
	PRINT(("End upload\n"));
	return;
}


/***********************************************************
 * ReceiveData
 ***********************************************************/
int32
HFileTransThread::ReceiveData(char* buf,size_t size)
{
	bigtime_t timeout = 120*1000000; // 120sec.
	int32 rcv_len = 0;
	
	if(fEndpoint->IsDataPending(timeout))
		rcv_len = fEndpoint->Receive(buf,size);
	return rcv_len;
}