/*
 * ファイル転送クラス
 * Start()でスレッドのスタート、Cancel()でスレッドの停止
 */
#ifndef __HFileTransThread_H__
#define __HFileTransThread_H__

#include <String.h>
#include <NetworkKit.h>
#include "LThread.h"


class HFileTransThread :public LThread{
public:
					HFileTransThread(const char* filename 
									,BNetEndpoint *endpoint
									,uint32 data_pos
									,bool isDownload
									,BLooper *target);
	virtual			~HFileTransThread();
	virtual	int32	Main();
protected:
			void	Download();
			void	Upload();
			bool	fIsDownload;
			int32	ReceiveData(char* buf,size_t size);
private:
			uint32	fData_pos;
		BString 	fFilename;
		BNetEndpoint *fEndpoint;
		BLooper		*fTarget;
};
#endif