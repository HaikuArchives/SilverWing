#include "HTrackerConnection.h"
#include "hx_types.h"
#include "hl_magic.h"
#include <String.h>
#include <File.h>
#include <netdb.h>
#include <Application.h>
#include <stdio.h>
#include <netinet/in.h>
#include <FindDirectory.h>
#include <Path.h>

#include "TextUtils.h"
#include "HWindow.h"
#include "MAlert.h"
#include "HApp.h"

/***********************************************************
 * Constructor.
 ***********************************************************/
HTrackerConnection::HTrackerConnection(const char *address,uint16 port,BListItem *parent)
					:LThread(address,B_NORMAL_PRIORITY),fEncoding(0)
{
	fAddress = address;
	fPort = port;
	fParentItem = parent;
	BMessage pref;
	fEndpoint = NULL;
	BPath path;
	::find_directory(B_USER_SETTINGS_DIRECTORY,&path);
	path.Append("SilverWing-Prefs");
	BFile file(path.Path(),B_READ_ONLY);
	if( file.InitCheck() != B_OK)
	{
		(new BAlert("",_("Could not load SilverWing setting file"),_("OK")))->Go();
	}else{
		pref.Unflatten(&file);
		fEncoding = pref.FindInt32("encoding");
	}
}

/***********************************************************
 * Destructor.
 ***********************************************************/
HTrackerConnection::~HTrackerConnection()
{
	if(fEndpoint != NULL)
	{
		fEndpoint->Close();
	}
	this->Cancel();
	delete fEndpoint;
}

/***********************************************************
 * Thread main function.
 ***********************************************************/
int32
HTrackerConnection::Main()
{
	BNetAddress addr;
	fEndpoint = new BNetEndpoint();
	uint16  nusers, nservers,sport;
	char buf[16], name[1024], desc[1024], outbuf[1024];
	char socks_buf[128];
	char* dname = NULL;
	char* ddesc = NULL;
	struct in_addr a;
	uint32 ad;
	uint8 len;
	bool sock5 = false;
	TextUtils utils;
	BString str;

	register int pos;
	//cout << "Receiving from tracker:" << fAddress.String() <<endl;
	
	BMessage msg(M_SET_STATUS);
	//******** SilverWingの設定ファイルの読み込み *************//
	BMessage pref;
	BFile file("/boot/home/config/settings/SilverWing-Prefs",B_READ_ONLY);
	if( file.InitCheck() != B_OK)
	{
		(new BAlert("",_("Could not load SilverWing setting file"),_("OK")))->Go();
		goto end;
	}
	pref.Unflatten(&file);
	
	str = "";
	str << _("Connecting to") << " " << fAddress.String();
	msg.AddString("text",str.String());
	be_app->PostMessage(&msg);
	str = "";
	sock5 = pref.FindBool("sock5");
	//******** Firewall support ***************//
	if(sock5)
	{
		const char* firewall = pref.FindString("firewall");
		uint32 port = pref.FindInt32("firewall_port");
		bool auth = pref.FindBool("auth");
		const char* user = pref.FindString("firewall_user");
		const char* password = pref.FindString("firewall_password");
	
		addr.SetTo(firewall,port);
		if(fEndpoint->Connect(addr) != B_OK)
		{
			(new MAlert(_("SOCKS5 Error"),_("Cannot connect to SOCKS5 server"),_("OK"),NULL,NULL))->Go();
			goto end;
		}
		
		// authenticate with firewall
		socks_buf[0] = 0x05; /* protocol version */
		socks_buf[1] = 0x01; /* number of methods */
		int8 authType;
		if( auth )
			authType = 0x02; /* method username/password */
		else
			authType = 0x00; /* no authorization required */
		socks_buf[2] = authType;
		fEndpoint->Send(socks_buf,3);
		int result = fEndpoint->Receive(socks_buf,2);
		if (result != 2 || socks_buf[0] != 0x05 || (socks_buf[1] != authType && socks_buf[1] != 0x00))	// didn't read all; not SOCKS5; or won't accept method requested
		{
			(new MAlert(_("SOCKS5 Error"),_("Firewall authentication method INVALID."),_("OK"),NULL,NULL,B_STOP_ALERT))->Go();
			goto end;
		}
		if(auth)
		{
			// authenticate with firewall
			int offset = 0;
			socks_buf[offset++] = 1; /* version of subnegotiation  - send username, password*/
			int8 size = strlen(user) + 1;
			socks_buf[offset++] = (int8) size;
			memcpy(&socks_buf[offset], user, size);		offset += size;
			size = strlen(password) + 1;
			socks_buf[offset++] = (int8) size;
				memcpy(&socks_buf[offset], password, size);		offset += size;
			fEndpoint->Send(socks_buf, offset, 0);
			result = fEndpoint->Receive(socks_buf, 2, 0);
			if (result != 2 || socks_buf[0] != 1 || socks_buf[1] != 0)
			{
				(new MAlert(_("SOCKS5 Error"),_("Firewall authentication FAILED."),_("OK"),NULL,NULL,B_STOP_ALERT))->Go();
				goto end;
			}
		}
		
		socks_buf[0] = 0x05; /* protocol version */
		socks_buf[1] = 0x01; /* command TCP connect */
		socks_buf[2] = 0x00; /* reserved */
		socks_buf[3] = 0x01; /* address type IP v4 */
		uint32 ad = inet_addr(fAddress.String());
		memcpy(&socks_buf[4], &ad, 4);
		uint16 pt = htons(fPort);
		memcpy(&socks_buf[8], &pt, 2);	
		fEndpoint->Send(socks_buf,10);
		result = fEndpoint->Receive(socks_buf, 10);
		// check what firewall says
		if (result != 10 || socks_buf[0] != 0x05 || socks_buf[1] != 0x00)	// didn't read all; not SOCKS5; or rejected
		{
			(new MAlert(_("SOCKS5 Error"),_("Firewall connect FAILED."),_("OK"),NULL,NULL,B_STOP_ALERT))->Go();
			goto end;
		}
	}else{
	//******** ノーマルな接続 ************//	
		addr.SetTo(fAddress.String(),fPort);		
		if(fEndpoint->InitCheck() != B_OK)
		{
			(new MAlert(_("Error"),_("Socket initialize failed"),_("OK")))->Go();
			goto end;
		}
		if( fEndpoint->Connect(addr) != B_OK)
		{
			BString s = _("Could not connect to tracker");
			
			s << ": " << fAddress;
			(new MAlert(_("Error"),s.String(),_("OK")))->Go(); 
			goto end;
		}

	}	
	str << _("Sending command to") << " " << fAddress.String();
	msg.ReplaceString("text",str.String());
	be_app->PostMessage(&msg);
	str = "";
	if (fEndpoint->Send( HTRK_MAGIC, HTRK_MAGIC_LEN) != HTRK_MAGIC_LEN)
		goto end;
	if (b_read(fEndpoint, buf, 14) != 14)
		goto end;
//#ifdef DEBUG
	printf("\n %u servers\n", (nservers = ntohs(*((uint16 *)(&(buf[10]))))));
//#endif
	//task.max = nservers;
	//UpdateTask(&task);
	
	str << _("Receiving servers from") << "  " << fAddress.String();
	msg.ReplaceString("text",str.String());
	be_app->PostMessage(&msg);
	str = "";
	for (; nservers && mCanceled == false ; nservers--) {
		uint16 pad;
		::memset(buf,0,16);
		if (b_read(fEndpoint,&ad, 4) == -1)
			break;
		a.s_addr = ad;
		ad = ntohl(ad);
		if( (ad >> 24) == 0)
		{
			b_read(fEndpoint,buf,2*sizeof(uint16));
			continue;
		}
		::memset(buf,0,16);
		if(b_read(fEndpoint,&sport,sizeof(uint16)) == -1)
			goto end;
		sport = ntohs(sport);
		::memset(buf,0,16);
		if(b_read(fEndpoint,&nusers,sizeof(uint16)) == -1)
			goto end;
		nusers = ntohs(nusers);
		b_read(fEndpoint,&pad,sizeof(uint16));
		
		b_read(fEndpoint,&len,sizeof(len));
	
		::memset(name,0,1024);
		if (b_read(fEndpoint, name, len) == -1)
			goto end;
		name[len] = 0;

		if (b_read(fEndpoint, &len, sizeof(len)) == -1)
			goto end;
		memset(desc, 0, 1024);
		if (b_read(fEndpoint, desc, len) == -1)
			goto end;
		desc[len] = 0;
		utils.ConvertReturnsToLF(desc);
		pos = sprintf(outbuf, "%16s:%-5u | %5u | %s | %s \n",
			inet_ntoa(a), sport, nusers, name,desc);
		

		dname = new char[1024];
		::memset(dname,0,1024);
		::memcpy(dname,name,strlen(name));
		utils.ConvertToUTF8(&dname,fEncoding-1);
		//utils.EUCKR2UTF8(&dname);
		
		ddesc = new char[1024];
		::memset(ddesc,0,1024);
		::memcpy(ddesc,desc,strlen(desc));
		utils.ConvertToUTF8(&ddesc,fEncoding-1);
		//utils.EUCKR2UTF8(&ddesc);
		
		BMessage message(H_RECEIVE_SERVER);
		message.AddString("tracker",fAddress.String());
		message.AddString("address",inet_ntoa(a));
		message.AddString("name",dname);
		message.AddString("desc",ddesc);
		message.AddInt16("users",nusers);
		message.AddInt16("port",sport);
		message.AddPointer("parent",fParentItem);
		be_app->PostMessage(&message);
		delete[] dname;
		delete[] ddesc;	
	}	
end:
	//hx_trans--;
	msg.ReplaceString("text","idle");
	be_app->PostMessage(&msg);
	fEndpoint->Close();
	be_app->PostMessage(H_END_SEARCH);
	delete fEndpoint;
	fEndpoint = NULL;
	return 0;
}

/***********************************************************
 * Read from socket.
 ***********************************************************/
int
HTrackerConnection::b_read (BNetEndpoint *endpoint, void *bufp, size_t len)
{
	register uint8 *buf = (uint8 *)bufp;
	register int r, pos = 0;

	while (len) {
		if ((r = endpoint->Receive( &(buf[pos]),len)) <= 0)
			return -1;
		pos += r;
		len -= r;
	}

	return pos;
}