#include <Message.h>
#include <iostream>
#include <Autolock.h>

#include "HTrackerConnection.h"
#include "HLPacket.h"
#include "hl_magic.h"
#include "HApp.h"
#include "HWindow.h"
#include "TServer.h"
#include "HPrefs.h"
#include "TextUtils.h"

/**************************************************************
 * Constructor.
 **************************************************************/
HTrackerConnection::HTrackerConnection(const char* address,uint32 port,
const char* server_addr,uint16 server_port,uint32 users,const char* name,const char* desc,uint32 id)
	:BLooper(address)
	,fAddress(address)			// tracker's address.
	,fPort(port)				// trackre's port.
	,fServerAddress(server_addr)// your server address & port.
	,fServerPort(server_port)	// id will be needed when use non free tracker.
	,fUsers(users)
	,fID(id)
	,fName(name)
	,fDesc(desc)
	,fInited(false)
	,fEndpoint(NULL)
{
}

/**************************************************************
 * Destructor.
 **************************************************************/
HTrackerConnection::~HTrackerConnection()
{
	if(fEndpoint)
		fEndpoint->Close();
	delete fEndpoint;
}

/**************************************************************
 * MessageReceived.
 **************************************************************/
void
HTrackerConnection::MessageReceived(BMessage *message)
{
	switch(message->what)
	{
	case M_START_WRITE:
		if(!fInited)
			InitTracker();
		TrackerRegist();
		break;
	default:
		BLooper::MessageReceived(message);
	}
}

/**************************************************************
 * Send regist info to tracker.
 **************************************************************/
int32
HTrackerConnection::TrackerRegist()
{
	if(fResolved )
	{
		//BNetDebug::Enable(true);
		//BAutolock lock(this);
		HLPacket tracker_packet;
		// version
		tracker_packet.AppendUint16(1);
		// port
		tracker_packet.AppendUint16(fServerPort); 
		// automatically converted to network byte ordering
		// not working?
	
		// users
		tracker_packet.AppendUint16(this->fUsers);

		// reserve
		tracker_packet.AppendUint16(0);
		// id
		tracker_packet.AppendUint32(fID);
		// name len
		uint8 nlen = this->fName.Length();
		// get encoding setting
		int32 encoding;
		((HApp*)be_app)->Prefs()->GetData("encoding",&encoding);
		TextUtils utils;
		
		if(nlen != 0)
		{
			if(encoding>0)
			{
				utils.ConvertFromUTF8(fName,encoding);
				nlen = fName.Length();	
			}
			tracker_packet.AppendUint8(nlen);
			tracker_packet.AppendData(this->fName.String(),nlen);
		}
		// desc
		uint8 dlen = this->fDesc.Length();
		
		if(dlen != 0)
		{
			if(encoding>0)
			{
				utils.ConvertFromUTF8(fDesc,encoding);
				dlen = fDesc.Length();	
			}
			
			tracker_packet.AppendUint8(dlen);
			tracker_packet.AppendData(this->fDesc.String(),dlen);
		}	
		uint8 plen = this->fPassword.Length();
		
		if(plen != 0)
		{
			tracker_packet.AppendUint8(plen);
			tracker_packet.AppendData(this->fPassword.String(),plen);
		}
		fEndpoint->SendTo(tracker_packet,fTrackerAddress);
		
		/*
		BString log = "Regist to tracker ";
		log << fAddress.String() << "(" << fPort << ")" << "\n";
		this->Log(log.String());
		*/
	}
}

/**************************************************************
 * Init tracker address.
 **************************************************************/
void
HTrackerConnection::InitTracker()
{
	//BAutolock lock(this);
	if( fTrackerAddress.SetTo(fAddress.String(),fPort) == B_OK)
	{
		//BNetAddress addr(fServerAddress.String(),0);	
		fEndpoint = new BNetEndpoint(SOCK_DGRAM);
		//fEndpoint->Bind(addr);
		fEndpoint->SetNonBlocking(true);
		fResolved = true;
		fInited = true;
	}else
		fResolved = false;
	
}

/**************************************************************
 * Set tracker login info.
 **************************************************************/
void
HTrackerConnection::SetLoginInfo(uint32 id,const char* password)
{
	BAutolock lock(this);
	fPassword = password;
	fID = id;
}

/**************************************************************
 * Send log to main window.
 **************************************************************/
void
HTrackerConnection::Log(const char* text) const
{
	BMessage msg(T_LOG_MESSAGE);
	BString log;
	time_t t = time(NULL);
	const char *tmp = ctime(&t);
	char *time = new char[strlen(tmp)+1];
	::strcpy(time,tmp);
	time[strlen(tmp)-1] = '\0';
	log << time << "     " << text;
	delete[] time;
	msg.AddString("log",log.String());
	msg.AddInt32("type",(int32)0);
	((HApp*)be_app)->Window()->PostMessage(&msg);
}