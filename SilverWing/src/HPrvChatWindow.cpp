#include <ClassInfo.h>
#include <Autolock.h>

#include "HPrvChatWindow.h"
#include "HApp.h"
#include "RectUtils.h"
#include <santa/Colors.h>
#include "TextUtils.h"
#include "ResourceUtils.h"
#include "HToolbar.h"
#include "HUserItem.h"
#include "HSendMsgWindow.h"
#include "HCaption.h"
#include "HSettingWindow.h"
#include "HToolbarButton.h"
#include "HTaskWindow.h"
#include "HUserItem.h"
#include "HPrvChatList.h"
#include "CTextView.h"
#include "HotlineClient.h"
#include "MLTextControl.h"
#include "SplitPane.h"
#include "ChatLogView.h"
#include "HPrefs.h"
#include "HPrvChatCaption.h"
#include "HWindow.h"
#include "HDialog.h"
#include "HMsgWindow.h"

/***********************************************************
 * Constructor.
 ***********************************************************/
HPrvChatWindow::HPrvChatWindow(BRect rect,const char *name,uint32 pcref)
	:BWindow(rect,name,B_DOCUMENT_WINDOW,B_ASYNCHRONOUS_CONTROLS)
	,fPcref(pcref)
{
	InitGUI();
	AddShortcut('/',0,new BMessage(B_ZOOM));

	float min_width,min_height,max_width,max_height;
	GetSizeLimits(&min_width,&max_width,&min_height,&max_height);
	min_width = 300;
	min_height = 150;
	SetSizeLimits(min_width,max_width,min_height,max_height);
}

/***********************************************************
 * Destructor.
 ***********************************************************/
HPrvChatWindow::~HPrvChatWindow()
{
}

/***********************************************************
 * Set up GUIs.
 ***********************************************************/
void
HPrvChatWindow::InitGUI()
{
	BRect rect = Bounds();
	BView *bg = new BView(rect,"bg",B_FOLLOW_ALL,B_WILL_DRAW);
	bg->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	rect.right -= 20+B_V_SCROLL_BAR_WIDTH;
	rect.left = rect.right - 140;
	rect.top += 55;
	rect.bottom -= 30;
	ResourceUtils utils;
	// Userlist
	listview = new HPrvChatList(rect,"userlist");
	BScrollView *scrollView = new BScrollView("scrollview",listview,B_FOLLOW_LEFT|B_FOLLOW_BOTTOM,
													B_WILL_DRAW|B_FRAME_EVENTS,false,true,B_FANCY_BORDER);
	listview->SetInvocationMessage(new BMessage(PRVCHAT_SEND_MESSAGE));
	//ChatView
	BRect textrect = this->Bounds();
	textrect.top += 2;
	textrect.left += 2;
	textrect.right -= 2 + B_V_SCROLL_BAR_WIDTH;
	textrect.bottom -= 2 + 50;
	chatview = new ChatLogView(textrect,"chatview",B_FOLLOW_ALL,B_WILL_DRAW);
	// Font
	BFont chatfont;
	int32 font_size;
	const char* font_family;
	const char* font_style;
	uint32 indexColor;
	uint8 col;
	((HApp*)be_app)->Prefs()->GetData("font_color",(int32*)&indexColor);
	col = indexColor;
	fChatColor.blue = col;
	col = indexColor >> 8;
	fChatColor.green = col;
	col = indexColor >> 16;
	fChatColor.red = col;

	((HApp*)be_app)->Prefs()->GetData("nick_color",(int32*)&indexColor);
	col = indexColor;
	fNickColor.blue = col;
	col = indexColor >> 8;
	fNickColor.green = col;
	col = indexColor >> 16;
	fNickColor.red = col;

	((HApp*)be_app)->Prefs()->GetData("font_family",&font_family);
	((HApp*)be_app)->Prefs()->GetData("font_style",&font_style);
	((HApp*)be_app)->Prefs()->GetData("font_size",&font_size);
	chatfont.SetSize(font_size);
	chatfont.SetFamilyAndStyle(font_family,font_style);
	chatview->SetFontAndColor(&chatfont,B_FONT_ALL,&fChatColor);
	// Color
	((HApp*)be_app)->Prefs()->GetData("back_color",(int32*)&indexColor);
	rgb_color backColor;
	col = indexColor;
	backColor.blue = col;
	col = indexColor >> 8;
	backColor.green = col;
	col = indexColor >> 16;
	backColor.red = col;
	chatview->SetColorSpace(B_RGB32);
	chatview->SetViewColor(backColor);
	//
	chatview->MakeEditable(false);
	chatview->SetWordWrap(true);
	chatview->SetStylable(true);
	chatview->SetPrivateChat(fPcref);
	BScrollView* scrollView2 = new BScrollView("scrollview",chatview,B_FOLLOW_ALL,
												B_WILL_DRAW|B_FRAME_EVENTS,false,true,B_FANCY_BORDER);
	//chatbgview->AddChild(scrollView2);

	textrect.bottom = Bounds().bottom-2;
	textrect.top = textrect.bottom - 40;
	textrect.right = Bounds().right -2;
	BBox *backbox = new BBox(textrect,NULL,B_FOLLOW_ALL);
	textrect.bottom -= 1;
	textrect.top += 1;
	textrect.left += 1;
	textrect.right -= 1;
	textrect.OffsetTo(B_ORIGIN);
	textrect.OffsetBy(1,1);
	textview = new MLTextControl(textrect,"textview",PRVCHAT_INVOKE_CHAT,B_FOLLOW_ALL);
	backbox->AddChild(textview);

	BRect rightrect = bg->Bounds();
	rightrect.top += 30;
	rightrect.bottom -= B_H_SCROLL_BAR_HEIGHT;
	fHSplitter = new SplitPane(rightrect,scrollView2,backbox,B_FOLLOW_ALL);
	fHSplitter->SetBarThickness(BPoint(0,7));
	fHSplitter->SetAlignment(B_HORIZONTAL);
	fHSplitter->SetBarAlignmentLocked(true);
	fHSplitter->SetResizeViewOne(true, true);
	int32 pos;
	((HApp*)be_app)->Prefs()->GetData("prvchat_hbar_pos",&pos);
	fHSplitter->SetBarPosition(BPoint(0,pos));

	fVSplitter = new SplitPane(rightrect,fHSplitter,scrollView,B_FOLLOW_ALL);
	fVSplitter->SetAlignment(B_VERTICAL);
	((HApp*)be_app)->Prefs()->GetData("prvchat_vbar_pos",&pos);
	fVSplitter->SetBarAlignmentLocked(true);
	fVSplitter->SetBarThickness(BPoint(7,0));
	fVSplitter->SetResizeViewOne(true, true);
	fVSplitter->SetBarPosition(BPoint(pos,0));
	bg->AddChild(fVSplitter);

	BRect toolrect = Bounds();
	toolrect.bottom = toolrect.top + 30;
	toolrect.left -= 1;
	toolrect.right += 1;
	HToolbar *toolbar = new HToolbar(toolrect,B_FOLLOW_LEFT_RIGHT|B_FOLLOW_TOP);
	toolbar->AddButton("msgbtn",utils.GetBitmapResource('BBMP',"BMP:MESSAGE"),new BMessage(PRVCHAT_SEND_MESSAGE),_("Send message"));
	toolbar->AddButton("infobtn",utils.GetBitmapResource('BBMP',"BMP:INFO"),new BMessage(PRVCHAT_GET_INFO),_("Get user infomation"));
	toolbar->AddButton("topicbtn",utils.GetBitmapResource('BBMP',"BMP:TOPIC"),new BMessage(PRVCHAT_CHANGE_TOPIC),_("Change topic"));
	bg->AddChild(toolbar);
	/****************** StatusBar ***********************/
	BRect statusrect = this->Bounds();
	statusrect.bottom += 2;
	statusrect.top = statusrect.bottom - B_H_SCROLL_BAR_HEIGHT -1;
	statusrect.right -= B_V_SCROLL_BAR_WIDTH-2;
	statusrect.left--;
	BBox *box = new BBox(statusrect,"status",B_FOLLOW_LEFT_RIGHT|B_FOLLOW_BOTTOM,B_WILL_DRAW);

	statusrect.OffsetTo(B_ORIGIN);
	statusrect.top +=2;
	statusrect.bottom -= 1;
	statusrect.left += 7;
	statusrect.right -= 7;

	HPrvChatCaption *view = new HPrvChatCaption(statusrect,"info",listview);
	box->AddChild(view);
	bg->AddChild(box);
	AddChild(bg);
}

/***********************************************************
 * Destructor.
 ***********************************************************/
void
HPrvChatWindow::MessageReceived(BMessage *message)
{
	switch(message->what)
	{
	// topic changed
	case H_TOPIC_CHANGED:
	{
		uint32 pcref;
		const char* topic;
		pcref = message->FindInt32("pcref");
		if(message->FindString("text",&topic) == B_OK)
		{
			HPrvChatCaption *view = cast_as(FindView("info"),HPrvChatCaption);
			view->SetTopic(topic);
			BString title = _("Private Chat");
			title << ":" << topic;
			SetTitle(title.String());
		}
		break;
	}
	//
	case PRVCHAT_CHANGE_TOPIC:
	{
		BMessage *msg = new BMessage(H_PRVCHAT_TOPIC_CHANGE);
		msg->AddInt32("pcref",Pcref());
		HDialog *dialog = new HDialog(RectUtils().CenterRect(200,70)
								,_("Change topic")
								,msg
								,_("New topic:"));
		dialog->Show();
		break;
	}
	// user info have been changed.
	case H_CHANGE_USER:
	{
		uint16 sock,icon,color;
		message->FindInt16("sock",(int16*)&sock);
		message->FindInt16("icon",(int16*)&icon);
		message->FindInt16("color",(int16*)&color);
		const char* nick;
		if( message->FindString("nick",&nick) != B_OK)
			nick = "";
		ChangeUserItem(sock,icon,color,nick,false);
		break;
	}
	// add new user to prvchat
	case H_CHAT_USER_CHANGE:
	{
		uint16 sock,icon,color;
		message->FindInt16("sock",(int16*)&sock);
		message->FindInt16("icon",(int16*)&icon);
		message->FindInt16("color",(int16*)&color);
		const char* nick;
		if( message->FindString("nick",&nick) != B_OK)
			nick = "";
		ChangeUserItem(sock,icon,color,nick);
		break;
	}
	case PRVCHAT_INVOKE_CHAT:
	{
		//string text = textview->Text();
		int32 len = strlen(textview->Text());
		char *text = new char[len+1];
		::memset(text,0,len+1);
		::sprintf(text,"%s",textview->Text());
		TextUtils utils;
		int32 encoding;
		((HApp*)be_app)->Prefs()->GetData("encoding",(int32*)&encoding);
		if(encoding)
			utils.ConvertFromUTF8(&text,encoding-1);
		utils.ConvertReturnsToCR(text);
		if(strlen(text) != 0)
		{
			BMessage msg(PRVCHAT_PRVCHAT_SENDMSG);
			msg.AddInt32("pcref",fPcref);
			msg.AddString("text",text);
			be_app->PostMessage(&msg);
			textview->SetText("");
		}
		delete[] text;
		break;
	}
	case PRVCHAT_RECEIVE_MSG:
	{
		const char* text = message->FindString("text");
		char *chat = new char[strlen(text)+1];
		::strcpy(chat,text);
		TextUtils utils;
		utils.ConvertReturnsToLF(chat);
		int32 encoding;
		((HApp*)be_app)->Prefs()->GetData("encoding",(int32*)&encoding);
		if(encoding)
		{
			utils.ConvertToUTF8(&chat,encoding-1);
		}
		int32 len = strlen(chat);
		BString str;
		for(register int32 i = 0;i < len;i++)
		{
			str << chat[i];
			if(chat[i+1] == '\n')
			{
				InsertChatMessage(str.String());
				str = "";
			}
			if(len == i+1)
				InsertChatMessage(str.String());
		}
		//InsertChatMessage((const char*)chat);
		delete[] chat;
		break;
	}
	//****** Chat font の変更 **************//
	case M_SET_CHAT_FONT_MSG:
	{
		const char* family = message->FindString("font_family");
		const char* style = message->FindString("font_style");
		int32 size = message->FindInt32("font_size");
		uint32 font_color = message->FindInt32("font_color");
		uint32 back_color = message->FindInt32("back_color");
		uint32 nick_color = message->FindInt32("nick_color");

		rgb_color color;
		uint8 col;
		col = font_color;
		fChatColor.blue = col;
		col = font_color >> 8;
		fChatColor.green = col;
		col = font_color >> 16;
		fChatColor.red = col;

		col = nick_color;
		fNickColor.blue = col;
		col = font_color >> 8;
		fNickColor.green = col;
		col = font_color >> 16;
		fNickColor.red = col;

		BFont font;
		font.SetSize(size);
		font.SetFamilyAndStyle(family,style);
		chatview->SetFontAndColor(0,strlen(chatview->Text()),&font);
		col = back_color;
		color.blue = col;
		col = back_color >> 8;
		color.green = col;
		col = back_color >> 16;
		color.red = col;
		chatview->SetViewColor(color);
		//chatview->Draw(chatview->Bounds());
		chatview->Invalidate();
		break;
	}
	case PRVCHAT_SEND_MESSAGE:
	{
		BRect rect = RectUtils().CenterRect(SEND_MESSAGE_WIDTH,SEND_MESSAGE_HEIGHT);
		int32 sel = listview->CurrentSelection();
		if(sel < 0)
			break;
		BAutolock lock(this);
		if(lock.IsLocked())
		{
			HUserItem *item = cast_as(listview->ItemAt(sel),HUserItem);

			if(item != NULL)
			{
				uint32 sock = item->Sock();
				bool msgchat;
				((HApp*)be_app)->Prefs()->GetData("msgchat",&msgchat);
				if(msgchat)
				{
					BMessage msg(MESSAGE_CHAT_MSG);
					msg.AddInt32("sock",sock);
					be_app->PostMessage(&msg);
				}else{
					BString title = _("Send message to ");
					title << item->Nick();
					HSendMsgWindow *win = new HSendMsgWindow(rect,title.String(),sock);
					win->Show();
				}
			}
		}
		break;
	}
	case PRVCHAT_GET_INFO:
	{
		uint32 sock;
		int32 sel = listview->CurrentSelection();
		if(sel < 0)
			break;
		BAutolock lock(this);
		if(lock.IsLocked())
		{
			HUserItem *item = cast_as(listview->ItemAt(sel),HUserItem);
			if(item != NULL){
				sock = item->Sock();
				BMessage msg(MWIN_USER_INFO_MESSAGE);
				msg.AddInt32("sock",sock);
				((HApp*)be_app)->Client()->PostMessage(&msg);
				}
		}
		break;
	}
	/******* Toolbar buttons' update *********/
	case M_UPDATE_TOOLBUTTON:
	{
		const char* name = message->FindString("name");
		void *pointer;
		message->FindPointer("pointer",&pointer);
		HToolbarButton *btn = static_cast<HToolbarButton*>(pointer);
		if(btn == NULL)
			break;

		if(::strcmp(name,"infobtn") == 0
			||::strcmp(name,"msgbtn") == 0)
		{
			int32 sel = listview->CurrentSelection();
			if(sel >= 0)
			{
				btn->SetEnabled(true);
			}else{
				btn->SetEnabled(false);
			}
		}
		break;
	}
	default:
		BWindow::MessageReceived(message);
	}
}

/***********************************************************
 * Insert chat message
 ***********************************************************/
void
HPrvChatWindow::InsertChatMessage(const char* text)
{
	BAutolock lock(this);
	if(lock.IsLocked())
	{
		int i = strlen(chatview->Text());
		chatview->Select(i,i);
		HCaption *caption = cast_as(FindView("info"),HCaption);
		if(caption)
			caption->SetTime(time(NULL));
		BFont font;
		rgb_color color;
		chatview->GetFontAndColor(0,&font,&color);
		// time stamp
		int time_offset = 0;
		bool timestamp;
		((HApp*)be_app)->Prefs()->GetData("timestamp",&timestamp);
		if(timestamp)
			time_offset = 8;

		//
		if(strlen(text) >= 15 && text[14] == ':') // chat
		{
			text_run	run1;
			run1.offset = 0;
			run1.font = font;
			run1.color = fNickColor;

			text_run	run2;
			run2.offset = 15 + time_offset;
			run2.font = font;
			run2.color = fChatColor;

			text_run_array	array;
			array.count = 2;
			array.runs[0] = run1;
			array.runs[1] = run2;
			BString str = text;
			if(timestamp)
			{
				time_t timet = time(NULL);
				struct tm* t = localtime(&timet);
				char *tmp = new char[10];
				::sprintf(tmp,"[%.2d:%.2d]",t->tm_hour,t->tm_min);
				str.Insert(tmp,1);
				delete[] tmp;
			}
			chatview->Insert(str.String(),&array);
		}else{
			text_run	run;
			run.offset = 0;
			run.font = font;
			run.color = fChatColor;

			text_run_array	array;
			array.count = 1;
			array.runs[0] = run;
			chatview->Insert(text,&array);
		}


		float min,max;
		(chatview->ScrollBar(B_VERTICAL))->GetRange(&min,&max);
		(chatview->ScrollBar(B_VERTICAL))->SetValue(max);
	}
}

/***********************************************************
 * QuitRequested
 ***********************************************************/
bool
HPrvChatWindow::QuitRequested()
{
	BPoint pos = fVSplitter->GetBarPosition();
	int32 x = static_cast<int32>(pos.x);
	((HApp*)be_app)->Prefs()->SetData("prvchat_vbar_pos",x);
	pos = fHSplitter->GetBarPosition();
	x = static_cast<int32>(pos.y);
	((HApp*)be_app)->Prefs()->SetData("prvchat_hbar_pos",x);

	((HApp*)be_app)->SaveRect("prvchat_window_rect",this->Frame());
	BMessage msg(PRVCHAT_PRVCHAT_DELETE);
	msg.AddPointer("window",this);
	be_app->PostMessage(&msg);
	return true;
}

/***********************************************************
 * Add user item
 ***********************************************************/
void
HPrvChatWindow::AddUserItem(uint16 sock ,uint16 icon,uint16 color,const char* nick)
{
	BAutolock lock(this);
	if( lock.IsLocked() )
	{
		listview->AddItem(new HUserItem(sock,icon,color,nick));
	}
}

/***********************************************************
 * Change user item
 ***********************************************************/
void
HPrvChatWindow::ChangeUserItem(uint16 sock ,uint16 icon,uint16 color,const char* nick,bool add)
{
	bool find = false;
	BAutolock lock(this);
	int32 count = listview->CountItems();
	if( lock.IsLocked() )
	{
		for(int32 i = 0; i < count ; i++)
		{
			HUserItem *userItem = cast_as(listview->ItemAt(i),HUserItem);
			if(userItem == NULL)
				break;
			if( userItem->Sock() == sock)
			{
				find = true;
				userItem->ChangeUser(sock,icon,color,nick);
				listview->InvalidateItem(i);
				break;
			}
		}
		if(find == false && add == true)
		{
			listview->AddItem(new HUserItem(sock,icon,color,nick));
			be_app->PostMessage(SOUND_LOGIN_SND);
		}
	}
}

/***********************************************************
 * Remove user item
 ***********************************************************/
void
HPrvChatWindow::RemoveUserItem(uint32 sock)
{
	BAutolock lock(this);
	if( lock.IsLocked() )
	{
		for(int i = 0; i < listview->CountItems(); i++)
		{
			HUserItem *user = cast_as(listview->ItemAt(i),HUserItem);
			if(user == NULL)
				break;
			if( user->Sock() == sock)
			{
				HUserItem *item = cast_as(listview->RemoveItem(i),HUserItem);
				be_app->PostMessage(SOUND_LOGOUT_SND);
				delete item;
				break;
			}
		}
	}
}
