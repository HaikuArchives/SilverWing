#include "ChatLogView.h"
#include "HWindow.h"
#include "HPrvChatWindow.h"
#include "HApp.h"
#include "HPrefs.h"

#include <Message.h>
#include <String.h>
#include <Application.h>

/***********************************************************
 * Constructor
 ***********************************************************/
ChatLogView::ChatLogView(BRect frame,
						const char* name,
						int32 resize,
						int32 flags)
			:URLTextView(frame,name,resize,flags)
			,fPcref(0)
{

}

/***********************************************************
 * Destructor
 ***********************************************************/
ChatLogView::~ChatLogView()
{
}

/***********************************************************
 * MessageReceived
 ***********************************************************/
void
ChatLogView::MessageReceived(BMessage *message)
{
	const char* nick;
	if(message->WasDropped() && message->FindString("nick",&nick) == B_OK)
	{
		this->WhenDropped(message);
	}else
		URLTextView::MessageReceived(message);
}


/***********************************************************
 * WhenDropped
 ***********************************************************/
void
ChatLogView::WhenDropped(const BMessage *message)
{
	const char* nick;
	const char* greeting[] = {"Hi","Howdy","Hello","Hey"};
	uint8 i = 4;
	unsigned int utime;
	
	long ltime = time(NULL);
	utime = (unsigned int)( ltime/2 );
	srand(utime);
	
	while(i >= 4)
	{
		i = rand();
	}
	
	if(message->FindString("nick",&nick) == B_OK)
	{
		BString str;
		str << greeting[i] << " " << nick << ".";
		if(fPcref == 0)
		{
			BMessage msg(MWIN_INVOKE_CHAT);
			msg.AddString("text",str);
			be_app->PostMessage(&msg);
		}else{
			BMessage msg(PRVCHAT_PRVCHAT_SENDMSG);
			msg.AddInt32("pcref",fPcref);
			msg.AddString("text",str);
			be_app->PostMessage(&msg);
		}
	}
}

/***********************************************************
 * InsertText
 ***********************************************************/
void
ChatLogView::InsertText(const char* text
						,int32 inlen
						,int32 offset
						,const text_run_array *runs)
{
	int32 sel = strlen(Text());
	URLTextView::InsertText(text,inlen,offset,runs);
	/********** find url ***********/
	BString str(text);
	int32 len = str.Length();
	char* tmp_url = str.LockBuffer(len+1);
	bool hit = false;
	int32 last_offset = -1;
	BFont font;
	rgb_color color;
	this->GetFontAndColor(0,&font,&color);
	
	rgb_color urlcolor;
	uint32 indexColor;
	uint8 col;
	
	((HApp*)be_app)->Prefs()->GetData("url_color",(int32*)&indexColor);
	col = indexColor;
	urlcolor.blue = col;
	col = indexColor >> 8;
	urlcolor.green = col;
	col = indexColor >> 16;
	urlcolor.red = col;
	
	for(register int32 i = 14;i < len;i++)
	{
		if( (::strncmp(&tmp_url[i],"http://",7) == 0||
			::strncmp(&tmp_url[i],"file://",7) == 0 ||
			::strncmp(&tmp_url[i],"ftp://",6) == 0) && !hit)
		{
			last_offset = i;
			hit = true;
		}else if( (!isalpha(tmp_url[i]) 
				&& tmp_url[i] != '/' 
				&& tmp_url[i] != '.' 
				&& tmp_url[i] != ':'
				&& tmp_url[i] != '_'
				&& tmp_url[i] != '?'
				&& tmp_url[i] != '-'
				&& tmp_url[i] != '&'
				&& tmp_url[i] != '%'
				&& tmp_url[i] != '+'
				&& tmp_url[i] != '='
				&& tmp_url[i] != '~'
				&& !isdigit(tmp_url[i]) )&& hit){

			this->SetFontAndColor(sel+last_offset,sel+i,&font,B_FONT_ALL,&urlcolor);
			hit = false;
		}
	}
	if(hit)
		this->SetFontAndColor(sel+last_offset,sel+str.Length(),&font,B_FONT_ALL,&urlcolor);

	str.UnlockBuffer(len+1);
}