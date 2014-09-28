#ifndef __HTRACKER_CONNECTION_H__
#define __HTRACKER_CONNECTION_H__
#include <String.h>
#include <Looper.h>
#include <NetworkKit.h>
enum{
	M_START_WRITE = 'MSTW',
	M_END_WRITE = 'MEDW'	
};


class HTrackerConnection :public BLooper{
public:
				HTrackerConnection(const char* address
								,uint32 port
								,const char* server_addr
								,uint16 server_port
								,uint32 users
								,const char* name
								,const char* desc
								,uint32 id = 0);
	virtual 	~HTrackerConnection();
		void	SetLoginInfo(uint32 id,const char* password);
	 	int32	TrackerRegist();
	 	void	InitTracker();
	 	void	SetUsers(uint32 users) {fUsers = users;}
protected:
virtual void	MessageReceived(BMessage *message);
		void	Log(const char* text) const;
private:
	// tracker address.
	BString 	fAddress;
	uint32		fPort;
	// your hotline server address.
	BString	 	fServerAddress;
	uint16		fServerPort;
	uint32		fUsers;
	uint32		fID;
	BString		fPassword;
	BString		fName;
	BString		fDesc;
	BNetAddress fTrackerAddress;
	bool		fResolved;
	bool		fInited;
	BNetEndpoint *fEndpoint;
};
#endif