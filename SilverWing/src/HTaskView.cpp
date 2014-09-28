#include "HTaskView.h"
#include <iostream>
#include <File.h>
#include <Directory.h>
#include <Path.h>
#include <Mime.h>
#include <stdio.h>
#include <netdb.h>
#include <Debug.h>
#include <Beep.h>

#include "TextUtils.h"
#include "MAlert.h"
#include "HApp.h"
#include "hx_types.h"
#include "hl_magic.h"
#include "dhargs.h"
#include "HTaskWindow.h"
#include "Colors.h"
#include "HPrefs.h"

#define SLEEP_TIME 100

/***********************************************************
 * Constructor.
 ***********************************************************/
HTaskView::HTaskView(BRect rect, const char* label,uint32 task,uint32 type)
	:BStatusBar(rect,"",label)
	,fTask(task)
	,fType(type)
	,fServerQueue(0)
	,fThread(-1)
{
	SetFlags(B_WILL_DRAW|B_NAVIGABLE|B_FRAME_EVENTS);
	SetBarHeight(12);
	SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	SetMaxValue(10);
	
	((HApp*)be_app)->Prefs()->GetData("encoding",(int32*)&fEncoding);
}

/***********************************************************
 * Destructor.
 ***********************************************************/
HTaskView::~HTaskView()
{
	if(fThread >= 0)
	{
		Cancel();
	}
}

/***********************************************************
 * MessageReceived.
 ***********************************************************/
void
HTaskView::MessageReceived(BMessage *message)
{
	switch(message->what)
	{
	case M_SET_TRAILING_TEXT:
	{
		const char* text;
		if(message->FindString("text",&text) == B_OK)
			this->SetTrailingText(text);
		break;
	}
	case M_RESET:
	{
		const char* text;
		if(message->FindString("text",&text) == B_OK)
			this->Reset(text,NULL);
		break;
	}
	case M_UPDATE_POS:
	{
		int32 pos = message->FindInt32("data");
		Update(pos);
		break;
	}
	default:
		BStatusBar::MessageReceived(message);
	}
}

/***********************************************************
 * MouseDown
 ***********************************************************/
void
HTaskView::MouseDown(BPoint point)
{
	BStatusBar::MouseDown(point);
	if (!IsFocus())
		MakeFocus(true);
	return;
}

/***********************************************************
 * IsRunning.
 ***********************************************************/
bool
HTaskView::IsRunning()
{
	thread_info info;
	if( ::get_thread_info(fThread,&info) == B_OK)
	{
		if(info.state == B_THREAD_RUNNING)
		{
			return true;
		}
	}
	return false;
}

/***********************************************************
 * Start file transfer thread.
 ***********************************************************/
void
HTaskView::Start()
{
	if(fDownload && fServerQueue == 0)
	{
		fThread = ::spawn_thread(DownloadThread,"DownloadThread",B_LOW_PRIORITY,this);
		::resume_thread(fThread);
	}else if(!fDownload && fServerQueue == 0){
		fThread = ::spawn_thread(UploadThread,"UploadThread",B_LOW_PRIORITY,this);
		::resume_thread(fThread);
	}
}

/***********************************************************
 * Set server address and port.
 ***********************************************************/
void
HTaskView::SetServer(const char* address,uint16 port)
{
	fAddress = address;
	fPort = port;
}

/***********************************************************
 * Set file paths.
 ***********************************************************/
void
HTaskView::SetFiles(const char* localpath,const char* remotepath)
{
	fLocalpath = localpath;
	fRemotepath = remotepath;
}

/***********************************************************
 * Set transfer refs and size.
 ***********************************************************/
void
HTaskView::SetRefAndSize(uint32 ref,uint32 size)
{
	fRef = ref;
	fSize = size;
}

/***********************************************************
 * Cancel file transfers.
 ***********************************************************/
void
HTaskView::Cancel()
{
	if(fServerQueue == 0)
	{
		fCancel = true;
		status_t status;
		::wait_for_thread(fThread,&status);
	}else{
		BMessage msg(M_TRANS_COMPLETE);
		msg.AddPointer("pointer",this);
		Window()->PostMessage(&msg);
	}
}

/***********************************************************
 *
 ***********************************************************/
int32
HTaskView::DownloadThread(void* data)
{
	HTaskView* trans = (HTaskView*)data;
	bigtime_t timeout = 120*1000000;
	
	uint32 data_len = trans->fSize;
	BFile file;
	register int r;
	register uint32 pos=0;
	int32 len=40;
	BString sts;
	BNetEndpoint endpoint;
	BMessage trailingMessage(M_SET_TRAILING_TEXT);
	BMessage resetMessage(M_RESET);
	BMessage updateMessage(M_UPDATE_POS);
	updateMessage.AddInt32("data",0);
	struct htxf_hdr xfh;
	BDirectory dir;
	BPath path;
	float percent;
	uint32 intper;
	char* p = NULL;
	char labelBuf[1024];
	char buf[16384 + 16];
//******* SOCKS5 valiables ***********//
	bool useSock5;
	
//*********************************//
	trans->fCancel = false;
	trailingMessage.AddString("text","Connecting...");
	trans->Window()->PostMessage(&trailingMessage,trans);
	
	((HApp*)be_app)->Prefs()->GetData("sock5",&useSock5);
	if(endpoint.InitCheck()!= B_OK)
	{
		(new MAlert(_("Error"),_("Socket initialize error"),_("OK"),NULL,NULL,B_STOP_ALERT))->Go();
		goto done;
	}
	//endpoint.SetTimeout(60*100000);
	if(useSock5)
	{
		if( !(trans->ConnectWithSocks5(endpoint)) )
		{
			(new MAlert(_("Error"),_("Could not connect with SOCKS5"),_("OK")))->Go();
			goto done;
		}
	}else{
		if(endpoint.Connect(trans->Address(),trans->Port()+1) == B_ERROR)
		{
			BString title = _("Could not connect to server...\n");
			title << endpoint.ErrorStr() << " " << _("Type:") << " " << (long)endpoint.Error();
			(new MAlert(_("Error"),title.String(),_("OK"),NULL,NULL,B_STOP_ALERT))->Go();
			goto done;
		}
	}
	xfh.magic = htonl(HTXF_MAGIC_INT);
	xfh.len = xfh.type = 0;
	xfh.ref = htonl(trans->fRef);
	
	trailingMessage.ReplaceString("text",_("Sending commands…"));
	trans->Window()->PostMessage(&trailingMessage,trans);
	
	if(endpoint.Send((void*)&xfh,SIZEOF_HTXF_HDR) != SIZEOF_HTXF_HDR)
	{
		(new MAlert(_("Error"),_("Could not send command to server"),_("OK"),NULL,NULL,B_STOP_ALERT))->Go();
		goto done;
	}

	
	while (len && trans->fCancel == false) 
	{
		if(endpoint.IsDataPending(timeout))
		{
			if ((r = endpoint.Receive( &(buf[pos]), len)) <= 0)
			{
				if(!trans->fCancel)
					(new MAlert(_("Error"),_("Could not receive download infomation"),_("OK"),NULL,NULL,B_STOP_ALERT))->Go();
				goto done;
			}
			pos += r;
			len -= r;
		}else{
			goto done;
		}
	}		
	pos = 0;
	L16NTOH(len, &buf[38]);
	len += 16;
	while (len && trans->fCancel == false) 
	{
		if(endpoint.IsDataPending(timeout))
		{
			if ((r = endpoint.Receive( &(buf[pos]), len)) <= 1)
			{
				if(!trans->fCancel)
					(new MAlert(_("Error"),_("Could not receive download infomation"),_("OK"),NULL,NULL,B_STOP_ALERT))->Go();
				goto done;
			}
			pos += r;
			len -= r;
		}else{
			goto done;
		}
	}
	L32NTOH(len, &buf[pos - 4]);
	
	if (!len && trans->fCancel == false)
	{
		goto done;
	}
	p = new char[trans->fLocalpath.Length()+1];
	strcpy(p,trans->fLocalpath.String());
	TextUtils().ConvertToUTF8(&p,trans->fEncoding-1);
	resetMessage.AddString("text",BPath(p).Leaf());
	trans->Window()->PostMessage(&resetMessage,trans);

	if(file.SetTo(p,B_WRITE_ONLY|B_CREATE_FILE|B_OPEN_AT_END) != B_OK)
	{
		path = p;
		path.GetParent(&path);
		dir.CreateDirectory(path.Path(),&dir);
		if(file.SetTo(p,B_WRITE_ONLY|B_CREATE_FILE|B_OPEN_AT_END) != B_OK)
		{
			BString title = _("Could not create localfile");
			title << "…\n";
			title << path.Path();
			beep();
			(new MAlert(_("Error"),title.String(),_("OK"),NULL,NULL,B_STOP_ALERT))->Go();
			goto done;
		}
	}
	delete[] p;
	
	data_len = len;
	trans->SetMaxValue(trans->fSize);

	trailingMessage.ReplaceString("text",_("Receiving data…"));
	trans->Window()->PostMessage(&trailingMessage,trans);
	
	file.Lock();
	trans->fStart_Time = time(NULL);
	while (data_len && trans->fCancel == false) {
		if(endpoint.IsDataPending(timeout))
		{
			len = endpoint.Receive(buf, sizeof buf < data_len ? sizeof buf : data_len);
			if(len == B_ERROR)
				break;
			
			::snooze( SLEEP_TIME );
			if(len > 0 )
			{	
				file.Write(buf,len);
				data_len -= len;
				pos = len;
				trans->fEnd_Time = time(NULL);
				float diff = difftime(trans->fEnd_Time,trans->fStart_Time);
				if(diff < 1) diff = 1;
				float ave = (trans->fSize - data_len)/diff;
				percent = (trans->fSize - data_len)*100/trans->fSize;
				intper = static_cast<uint32>(percent);
				if(ave < 1024)
				{
#ifdef __INTEL__
					::snprintf(labelBuf,1024,"%ld%s %10.2f Bytes",intper,"%",ave);
#else
					::sprintf(labelBuf,"%ld%s %10.2f Bytes",intper,"%",ave);
#endif
				}else if(ave >= 1024)
				{
					ave = ave/1024.0;
#ifdef __INTEL__
					::snprintf(labelBuf,1024,"%ld%s %10.2f KB",intper,"%",ave);
#else
					::sprintf(labelBuf,"%ld%s %10.2f KB",intper,"%",ave);
#endif
				}
				sts << labelBuf;
				if(!trans->fCancel)
				{
					updateMessage.ReplaceInt32("data",pos);
					trans->Window()->PostMessage(&updateMessage,trans);
					trailingMessage.ReplaceString("text",sts.String());
					trans->Window()->PostMessage(&trailingMessage,trans);
				}
				sts = "";
			}
		}else{
			goto done;
		}
	}
	file.Unlock();
	file.Sync();
	file.Unset();
	if( !(trans->fCancel) )
		be_app->PostMessage(SOUND_FILE_DOWN_SND); 
done:
	endpoint.Close();
	
	trans->fCancel = false;
	BMessage msg(M_TRANS_COMPLETE);
	msg.AddPointer("pointer",trans);
	trans->Window()->PostMessage(&msg);
	return B_OK;
}

/***********************************************************
 * Upload thread
 ***********************************************************/
int32
HTaskView::UploadThread(void* data)
{
	HTaskView *trans = (HTaskView*)data;
	trans->SetBarColor(Red);
	trans->fCancel = false;
	uint32 data_len = trans->fSize;
	BNetEndpoint endpoint;
	BMessage trailingMessage(M_SET_TRAILING_TEXT);
	BMessage resetMessage(M_RESET);
	BMessage updateMessage(M_UPDATE_POS);
	updateMessage.AddInt32("data",0);
	uint32 data_offset = trans->fSize;
	BFile file;
	BString debug;
	struct htxf_hdr xfh;
	register int r;
	register uint32 pos, len;
#ifdef __MWERKS__
	char buf2[16384 + 16];
#else
	char buf2[0xffff + 16];
#endif
	char buf[133];
	char labelBuf[1024];
	float percent;
	uint32 intper;
	BString sts;
	off_t filesize;
	uint32 transsize;
	BString filename = BPath(trans->fLocalpath.String()).Leaf();
	int32 l; // for checking file type
//******* SOCKS5 valiables ***********//
	bool useSock5;
	BMessage prefs;
//*********************************//
	
	trailingMessage.AddString("text",_("Connecting…"));
	trans->Window()->PostMessage(&trailingMessage,trans);

//*********** 設定ファイルの読み込み ***********//
	BFile preffile("/boot/home/config/settings/SilverWing-Prefs",B_READ_ONLY);
	if( preffile.InitCheck() != B_OK)
	{
		(new MAlert(_("Error"),_("Could not open SilverWing setting file"),_("OK"),NULL,NULL))->Go();
		goto done;
	}
	prefs.Unflatten(&preffile);
	useSock5 = prefs.FindBool("sock5");
	
	if(endpoint.InitCheck() != B_OK)
	{
		(new MAlert(_("Error"),_("Cound not create socket..."),_("OK"),NULL,NULL,B_STOP_ALERT))->Go();
		goto done;
	}
	//********* Socks5を用いた接続 ***********//
	if(useSock5)
	{
		if(trans->ConnectWithSocks5(endpoint) != true)
		{
			PRINT(( "Could not connect through the firewall.\n" ));
			goto done;
		}
	//******** ノーマルな接続 **********//
	}else{
	
		if(endpoint.Connect(trans->Address(),trans->Port()+1) != B_OK)
		{	
			BString title = _("Could not connect to server…");
			title += "\n";
			title << endpoint.ErrorStr() << " " << _("Type") << ": " << (long)endpoint.Error();
			(new MAlert(_("Error"),title.String(),_("OK"),NULL,NULL,B_STOP_ALERT))->Go();
			goto done;
		}
	}

	if(file.SetTo(trans->fLocalpath.String(),B_READ_ONLY) != B_OK)
	{
		BString title = _("Could not open localfile…");
		title += "\n";
		title += trans->fLocalpath;
		(new MAlert(_("Failed to upload"),title.String(),_("OK"),NULL,NULL,B_STOP_ALERT))->Go(); 
		goto done;
	}
	file.Seek(data_offset,SEEK_SET);

	file.GetSize(&filesize);
	data_len = filesize - data_offset;	

	xfh.magic = htonl(HTXF_MAGIC_INT);
	xfh.len = 0;
	xfh.ref = htonl(trans->fRef);
	xfh.type = htonl(133 + (data_len));
	
	trailingMessage.ReplaceString("text",_("Sending commands…"));
	trans->Window()->PostMessage(&trailingMessage,trans);
	
	if( endpoint.Send((void*)&xfh,SIZEOF_HTXF_HDR) != SIZEOF_HTXF_HDR)
	{
		(new MAlert(_("Error"),_("Could not send commands to server"),_("OK"),NULL,NULL,B_STOP_ALERT))->Go();
		goto done;
	}
	memcpy(buf,	"FILP\0\x01\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" //21
					"\0\0\x03INFO\0\0\0\0\0\0\0\0\0\0\0MAMAC",44); //23
	//check file type
	l = filename.FindLast(".");
	if(l == B_ERROR)
	{
		BString type;
		file.ReadAttrString("BEOS:TYPE",&type);
		BMimeType mtype(type.String());
		BMimeType stype;
		mtype.GetSupertype(&stype);
		if(::strcmp(stype.Type(),"text") == 0)
			memcpy(&buf[44],"TEXT",4);
		else if(::strcmp(mtype.Type(),"audio/x-mpeg") == 0)
			memcpy(&buf[44],"MP3 ",4);
		else if(::strcmp(mtype.Type(),"audio/x-mpeg") == 0)
			memcpy(&buf[44],"MP3 ",4);
		else
			memcpy(&buf[44],"BINA",4);
	}else{
		BString ext;
		filename.CopyInto(ext,l+1,3);
		if(ext.ICompare("zip") == 0|| ext.ICompare("gz") == 0)
			memcpy(&buf[44],"ZIP ",4);
		else if(ext.ICompare("txt") == 0 || ext.ICompare("html") == 0
			|| ext.ICompare("cpp") == 0|| ext.ICompare("h") == 0
			|| ext.ICompare("c") == 0|| ext.ICompare("htm") == 0
			|| ext.ICompare("pl") == 0)
			memcpy(&buf[44],"TEXT",4);
		else if(ext.ICompare("mov") == 0 )
			memcpy(&buf[44],"MooV",4);
		else if(ext.ICompare("bmp") == 0 )
			memcpy(&buf[44],"BMP ",4);
		else if(ext.ICompare("gif") == 0 )
			memcpy(&buf[44],"GIFf",4);
		else if(ext.ICompare("png") == 0 )
			memcpy(&buf[44],"PNGf",4);
		else if(ext.ICompare("mp3") == 0 )
			memcpy(&buf[44],"MP3 ",4);
		else if(ext.ICompare("wav") == 0 )
			memcpy(&buf[44],"WAVE",4);
		else if(ext.ICompare("bmp") == 0 )
			memcpy(&buf[44],"BMP ",4);
		else if(ext.ICompare("mpg") == 0 || ext.ICompare("mpeg") == 0)
			memcpy(&buf[44],"MPEG",4);
		else if(ext.ICompare("pict") == 0 )
			memcpy(&buf[44],"PICT",4);
		else
			memcpy(&buf[44],"BINA",4);
	}
	memcpy(&buf[48],"????",4); //8
	memcpy(&buf[44+8],"\0\0\0\0\0\0\x01\0\0\0\0\0\0\0\0\0\0\0\0\0" //20
					"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\x07" //21
					"\0\0\0\0\0\0\0\x07\0\0\0\0\0\0\0\0\0\0\x03" //19 
					"hxd\0\0DATA\0\0\0\0\0\0\0\0",//17
					77);
	
	S32HTON(data_len, &buf[129]);
	if (endpoint.Send( buf, 133) != 133)
	{
		(new MAlert(_("Error"),_("Could not send commands to server…"),_("OK"),NULL,NULL,B_STOP_ALERT))->Go();
		goto done;
	}
	
	trans->SetMaxValue(data_len);
	transsize = data_len;
	file.Lock();
	
	trans->fStart_Time = time(NULL);

	while (data_len && trans->fCancel == false) 
	{
		len = file.Read( buf2, sizeof(buf2)<data_len?sizeof(buf2):data_len);
		pos = 0;
		while (len) 
		{
			if ((r = endpoint.Send( &(buf2[pos]), len)) < 1)
				continue;
			len -= r;	
			data_len -= r;
			pos += r;
			trans->fEnd_Time = time(NULL);
			float diff = difftime(trans->fEnd_Time,trans->fStart_Time);
			if(diff < 1) diff = 1;
			float ave = (transsize - data_len)/diff;
			
			percent = abs(transsize - data_len)*100/transsize;
			intper = static_cast<uint32>(percent);
			if(ave < 1024)
			{
#ifdef __INTEL__
				::snprintf(labelBuf,1024,"%ld%s %10.2f Bytes",intper,"%",ave);
#else
				::sprintf(labelBuf,"%ld%s %10.2f Bytes",intper,"%",ave);
#endif
			}else{
				ave = ave/1024.0;
#ifdef __INTEL__
				::snprintf(labelBuf,1024,"%ld%s %10.2f KB",intper,"%",ave);
#else
				::sprintf(labelBuf,"%ld% %10.2f KB",intper,"%",ave);
#endif			
			}
			sts += labelBuf;
			updateMessage.ReplaceInt32("data",pos);
			trans->Window()->PostMessage(&updateMessage,trans);
			trailingMessage.ReplaceString("text",sts.String());
			trans->Window()->PostMessage(&trailingMessage,trans);
			sts = "";
			::snooze( SLEEP_TIME );
		}
	}
done:
	file.Unlock();
	file.Unset();
	endpoint.Close();
	if( !(trans->fCancel) )
		be_app->PostMessage(SOUND_FILE_DOWN_SND); 
	trans->fCancel = false;
	BMessage msg(M_TRANS_COMPLETE);
	msg.AddPointer("pointer",trans);
	trans->Window()->PostMessage(&msg);
	return B_OK;
}

/***********************************************************
 * Connect through SOCKSv5 firewalls.
 ***********************************************************/
bool
HTaskView::ConnectWithSocks5(BNetEndpoint &endpoint)
{
//******* SOCKS5 valiables ***********//
	bool auth;
	const char* firewall;
	uint32 port;
	const char* user;
	const char* password;

	((HApp*)be_app)->Prefs()->GetData("firewall",&firewall);
	((HApp*)be_app)->Prefs()->GetData("firewall_port",(int32*)&port);
	if(endpoint.Connect(firewall,port) != B_OK)
	{
		(new MAlert(_("SOCKS5 Error"),_("Cannot connect to SOCKS5 server"),_("OK"),NULL,NULL))->Go();
		return false;
	}
	((HApp*)be_app)->Prefs()->GetData("auth",&auth);
	// ログイン
	char firewall_buf[128];
	// authenticate with firewall
	firewall_buf[0] = 0x05; /* protocol version */
	firewall_buf[1] = 0x01; /* number of methods */
	int8 authType;
	if( auth )
		authType = 0x02; /* method username/password */
	else
		authType = 0x00; /* no authorization required */
	firewall_buf[2] = authType;
	endpoint.Send(firewall_buf,3);
	int result = endpoint.Receive(firewall_buf,2);
	if (result != 2 || firewall_buf[0] != 0x05 || (firewall_buf[1] != authType && firewall_buf[1] != 0x00))	// didn't read all; not SOCKS5; or won't accept method requested
	{
		(new MAlert(_("SOCKS5 Error"),_("Firewall authentication method INVALID."),_("OK"),NULL,NULL,B_STOP_ALERT))->Go();
		return false;
	}
	if(auth)
	{
		((HApp*)be_app)->Prefs()->GetData("firewall_user",&user);
		((HApp*)be_app)->Prefs()->GetData("firewall_password",&password);

		// authenticate with firewall
		int offset = 0;
		firewall_buf[offset++] = 1; /* version of subnegotiation  - send username, password*/
		int8 size = strlen(user) + 1;
		firewall_buf[offset++] = (int8) size;
		memcpy(&firewall_buf[offset], user, size);		offset += size;
		size = strlen(password) + 1;
		firewall_buf[offset++] = (int8) size;
			memcpy(&firewall_buf[offset], password, size);		offset += size;
		endpoint.Send(firewall_buf, offset, 0);
		result = endpoint.Receive(firewall_buf, 2, 0);
		if (result != 2 || firewall_buf[0] != 1 || firewall_buf[1] != 0)
		{
			(new MAlert(_("SOCKS5 Error"),_("Firewall authentication FAILED."),_("OK"),NULL,NULL,B_STOP_ALERT))->Go();
			return false;
		}
	}
	//********* Connect through firewall ****************://
	firewall_buf[0] = 0x05; /* protocol version */
	firewall_buf[1] = 0x01; /* command TCP connect */
	firewall_buf[2] = 0x00; /* reserved */
	firewall_buf[3] = 0x01; /* address type IP v4 */
	uint32 ad = inet_addr(this->Address());
	memcpy(&firewall_buf[4], &ad, 4);
	uint16 pt = htons(this->Port()+1);
	memcpy(&firewall_buf[8], &pt, 2);	
	endpoint.Send(firewall_buf,10);
	result = endpoint.Receive(firewall_buf, 10);
	// check what firewall says
	if (result != 10 || firewall_buf[0] != 0x05 || firewall_buf[1] != 0x00)	// didn't read all; not SOCKS5; or rejected
	{
		(new MAlert(_("SOCKS5 Error"),_("Connect FAILED through firewall."),_("OK"),NULL,NULL,B_STOP_ALERT))->Go();
		return false;
 	}
 	return true;
}

/***********************************************************
 * Draw
 ***********************************************************/
void
HTaskView::Draw(BRect rect)
{
	BStatusBar::Draw(rect);
	if(IsFocus())
	{
		BRect rect = Bounds();
		const rgb_color old = this->HighColor();
		this->SetHighColor(ui_color(B_KEYBOARD_NAVIGATION_COLOR));
		this->StrokeRect(rect);
		this->SetHighColor(old);
	}else{
		BRect rect = Bounds();
		const rgb_color old = this->HighColor();
		this->SetHighColor(ui_color(B_PANEL_BACKGROUND_COLOR));
		this->StrokeRect(rect);
		this->SetHighColor(old);	
	}
	Flush();
}

/***********************************************************
 * MakeFocus
 ***********************************************************/
void 
HTaskView::MakeFocus(bool focused) 
{
	if (focused != IsFocus())
	{ 
		BStatusBar::MakeFocus(focused);
		Draw(Bounds()); 
		Flush();
	}

	return; 
}

/***********************************************************
 * SetQueue
 ***********************************************************/
void
HTaskView::SetQueue(uint32 queue,bool init)
{
	if(init)
		SetMaxValue(queue);
	else
		Update(fServerQueue - queue);

	fServerQueue = queue;

	if(fServerQueue == 0&&!init)
	{
		Start();
		return;
	}	
	
	BString str = _("waiting queue");
	str += ": ";
	str << queue;
	SetTrailingText(str.String());
}