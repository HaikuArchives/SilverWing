#ifndef __HTASKVIEW_H__
#define __HTASKVIEW_H__

#include <StatusBar.h>
#include <Message.h>
#include <String.h>
#include <NetworkKit.h>

enum{
	M_SET_TRAILING_TEXT = 'MSTT',
	M_RESET = 'MRES',
	M_UPDATE_POS = 'MUPP'
};

class HTaskView :public BStatusBar {
public:
					HTaskView(BRect rect,
								const char* label,
								uint32 task,
								uint32 type = 1);
		virtual		~HTaskView();
			uint32  Task() {return fTask;}
			uint32	Type() {return fType;}
			uint32 	Ref() {return fRef;}
	/**************** file transfer *****************/
			void	SetServer(const char* address,uint16 port);
			void	SetFiles(const char* localpath,const char* remotepath);
			void	SetRefAndSize(uint32 ref,uint32 size);
			void	SetDownload(bool download) {fDownload = download;}
			void	SetQueue(uint32 queue,bool init = false);
	const char*		Address() {return fAddress.String();}
	static int32	DownloadThread(void *data);
	static int32	UploadThread(void *data);
			uint16  Port() {return fPort;}
			void	Start();
			bool	IsRunning();
			void	Cancel();
			bool	ConnectWithSocks5(BNetEndpoint &endpoint);

protected:
	virtual void	MouseDown(BPoint point);
	virtual void	MessageReceived(BMessage *message);
	virtual void	Draw(BRect rect);
	virtual void 	MakeFocus(bool focused = true);	

private:
	uint32 		fTask;
	uint32 		fType;
	uint32		fServerQueue;
	bool 		fDownload;
	BString 	fAddress;
	uint16 		fPort;
	BString 	fLocalpath;
	BString 	fRemotepath;
	uint32 		fRef;
	thread_id 	fThread;
	// Uploadの場合はdata_offset
	uint32 		fSize;
	bool		fCancel;
	int32		fEncoding;
	// timer
	time_t		fStart_Time;
	time_t		fEnd_Time;
};
#endif