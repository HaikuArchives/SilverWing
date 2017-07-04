#include <Window.h>
#include "HApp.h"
#include "MLTextControl.h"
#include <santa/Colors.h>
#include "HUserList.h"
#include "HUserItem.h"
#include <iostream>
#include <ClassInfo.h>

/***********************************************************
 * Constructor.
 ***********************************************************/
MLTextControl::MLTextControl(BRect rect,const char* name,uint32 what,uint32 resize)
	:CTextView(rect,name,resize,B_WILL_DRAW|B_NAVIGABLE)
	,fWhat(what)
	,fHistoryIndex(0)
	,fCashed(false)
	,fCash("")
{
	this->SetWordWrap(true);
}

/***********************************************************
 * Destructor.
 ***********************************************************/
MLTextControl::~MLTextControl()
{
	EmptyHistory();
}

/***********************************************************
 * Key down
 ***********************************************************/
void
MLTextControl::KeyDown(const char* bytes,int32 numBytes)
{
	int32 modifier = 0;
	Window()->CurrentMessage()->FindInt32("modifiers",&modifier);

	if(bytes[0] == B_RETURN && modifier != 257)
	{
		if(strlen(Text()) > 0)
		{
			Window()->PostMessage(fWhat);
			AddHistory(this->Text());
			fCashed = false;
			fCash = "";
		}
	}else if(bytes[0] == B_UP_ARROW){
		if(!fCashed)
		{
			fCash = Text();
			fCashed = true;
		}

		const char* str = HistoryAt(fHistoryIndex-1);
		if(str)
		{
			this->SetText(str);
			fHistoryIndex--;
		}
	}else if(bytes[0] == B_DOWN_ARROW){
		if(fHistoryIndex+1 == fHistory.CountItems())
		{
			SetText(fCash.String());
			int32 len = fCash.Length();
			Select(len,len);
			fHistoryIndex++;
			return;
		}
		const char* str = HistoryAt(fHistoryIndex+1);
		if(str)
		{
			this->SetText(str);
			fHistoryIndex++;
		}
	}else if(bytes[0] == B_TAB && strlen(Text()) != 0){
		const char* text = Text();
		int32 start,end;
		GetSelection(&start,&end);
		if(start == 0)
			CTextView::KeyDown(bytes,numBytes);
		start--;
		BString nick = "";
		for(;;)
		{
			if(text[start] == B_SPACE || start < 0)
				break;
			nick.Insert(text[start--],1,0);
		}
		const char* result = FindNick(nick.String());
		if(result)
		{
			Insert(&result[nick.Length()]);
			fCashed = false;
			fCash = "";
		}
	}else{
		fCashed = false;
		fCash = "";
		CTextView::KeyDown(bytes,numBytes);
	}
}

/***********************************************************
 * Draw
 ***********************************************************/
void
MLTextControl::Draw(BRect updateRect)
{
	CTextView::Draw(updateRect);
}

/***********************************************************
 * Accept Drop
 ***********************************************************/
bool
MLTextControl::AcceptsDrop(const BMessage *message)
{
	const char* nick;
	if(message->FindString("nick",&nick) == B_OK)
		return true;
	else
		return CTextView::AcceptsDrop(message);
}

/***********************************************************
 * MessageReceived
 ***********************************************************/
void
MLTextControl::MessageReceived(BMessage *message)
{
	const char* nick;
	if(message->WasDropped() && message->FindString("nick",&nick) == B_OK)
	{
		this->WhenDropped(message);
	}else
		CTextView::MessageReceived(message);
}

/***********************************************************
 * WhenDropped
 ***********************************************************/
void
MLTextControl::WhenDropped(const BMessage *message)
{
	const char *nick;
	BPoint point;
	if(message->FindString("nick",&nick) == B_OK)
	{
		message->FindPoint("_drop_point_", &point);
		this->ConvertFromScreen(&point);

		const int32 offset = this->OffsetAt(point);
		this->Insert(offset,nick,strlen(nick));
		// this->Select(offset, offset + strlen(nick));
	}
}

/***********************************************************
 * Add history
 ***********************************************************/
void
MLTextControl::AddHistory(const char* text)
{
	char *hist = new char[strlen(text)+1];
	::strcpy(hist,text);
	fHistory.AddItem(hist);
	fHistoryIndex = fHistory.CountItems();
}

/***********************************************************
 * Empty history
 ***********************************************************/
void
MLTextControl::EmptyHistory()
{
	int32 count = fHistory.CountItems();
	while(count>0)
	{
		delete fHistory.RemoveItem(--count);
	}
}

/***********************************************************
 * Get history
 ***********************************************************/
const char*
MLTextControl::HistoryAt(int32 index)
{
	if(index < 0)
		return NULL;
	else if(index > fHistory.CountItems())
		return NULL;
	return (const char*)fHistory.ItemAt(index);
}

/***********************************************************
 * FindNick
 ***********************************************************/
const char*
MLTextControl::FindNick(const char* nick)
{
	if( strlen(nick) == 0)
		return NULL;
	bool found = false;
	int32 index = 0;
	BListView *listview = cast_as(Window()->FindView("userlist"),BListView);
	if(!listview)
		return NULL;

	HUserItem **items = (HUserItem**)listview->Items();
	int32 count = listview->CountItems();

	for(register int32 i = 0;i < count;i++)
	{
		BString str = items[i]->Nick();
		if( str.Compare(nick,strlen(nick)) == 0 && !found)
		{
			found = true;
			index = i;
		}else if(str.Compare(nick,strlen(nick)) == 0 && found){
			found = false;
			break;
		}
	}

	return found ? items[index]->Nick():NULL;
}
