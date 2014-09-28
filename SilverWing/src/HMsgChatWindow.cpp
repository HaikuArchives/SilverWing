#include "HMsgChatWindow.h"
#include "MLTextControl.h"
#include "ChatLogView.h"
#include "HApp.h"
#include "HPrefs.h"
#include "SplitPane.h"
#include "MsgIconView.h"
#include "HSendMsgWindow.h"
#include "TextUtils.h"
#include "HotlineClient.h"
#include "HCaption.h"

#include <Message.h>
#include <ScrollView.h>
#include <Box.h>
#include <ClassInfo.h>

/***********************************************************
 * Constructor
 ***********************************************************/
HMsgChatWindow::HMsgChatWindow(BRect rect
							,const char* name
							,uint32 sock
							,uint32 icon)
			:BWindow(rect,name,B_DOCUMENT_WINDOW,0)
			,fSock(sock)
			,fIcon(icon)
			,fNick(name)
{
	BString title = _("Message Chat:");
	title += " ";
	title += name;
	SetTitle(title.String());
	InitGUI();
	AddShortcut('/',0,new BMessage(B_ZOOM));
}

/***********************************************************
 * Destructor
 ***********************************************************/
HMsgChatWindow::~HMsgChatWindow()
{
}

/***********************************************************
 * InitGUI
 ***********************************************************/
void
HMsgChatWindow::InitGUI()
{
	BRect rect(Bounds());
	
	BView *view = new BView(rect,"bg",B_FOLLOW_ALL,B_WILL_DRAW);
	view->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	AddChild(view);
	
	rect = Bounds();
	rect.top += 5;
	rect.left += 5;
	rect.right -= 5;
	rect.bottom = rect.top + 20;
	BBitmap *bitmap = ((HApp*)be_app)->GetIcon(fIcon);
	MsgIconView *iconView = new MsgIconView(rect,fNick.String(),bitmap);
	view->AddChild(iconView);
	
	BRect textrect = this->Bounds();
	textrect.top += 2;
	textrect.left += 2;
	textrect.right -= 2 + B_V_SCROLL_BAR_WIDTH;
	textrect.bottom -= 2 + 50;
	fChatView = new ChatLogView(textrect,"chatview",B_FOLLOW_ALL,B_WILL_DRAW);
	fChatView->SetStylable(true);
	
	// フォントの設定を読み込む
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
	//chatfont.SetFace(chatfont.Face()|B_UNDERSCORE_FACE);
	fChatView->SetFontAndColor(&chatfont,B_FONT_ALL,&fChatColor);
	// 色の設定
	((HApp*)be_app)->Prefs()->GetData("back_color",(int32*)&indexColor);
	rgb_color backColor;
	col = indexColor;
	backColor.blue = col;
	col = indexColor >> 8;
	backColor.green = col;
	col = indexColor >> 16;
	backColor.red = col;
	fChatView->SetColorSpace(B_RGB32);
	fChatView->SetViewColor(backColor);
	//
	fChatView->MakeEditable(false);
	
	BScrollView* scrollView2 = new BScrollView("scrollview",fChatView,B_FOLLOW_ALL,
												B_WILL_DRAW|B_FRAME_EVENTS,false,true,B_FANCY_BORDER);
//	chatbgview->AddChild(scrollView2);
		
	
	textrect.bottom = Bounds().bottom-2;
	textrect.top = textrect.bottom - 27;
	textrect.right = Bounds().right -2;
	BBox *backbox = new BBox(textrect,NULL,B_FOLLOW_TOP_BOTTOM|B_FOLLOW_LEFT_RIGHT);
	textrect.bottom -= 1;
	textrect.top += 1;
	textrect.left += 1;
	textrect.right -= 1;
	textrect.OffsetTo(B_ORIGIN);
	textrect.OffsetBy(1,1);
	
	fChatEntry = new MLTextControl(textrect,"textview",SENDMSG_SEND_MSG,B_FOLLOW_TOP_BOTTOM|B_FOLLOW_LEFT_RIGHT);
	backbox->AddChild(fChatEntry);
	BRect rightrect = view->Bounds();
	rightrect.top +=  25;
	rightrect.bottom -= B_H_SCROLL_BAR_HEIGHT;
	fHSplitter = new SplitPane(rightrect,scrollView2,backbox,B_FOLLOW_ALL);
	fHSplitter->SetBarThickness(BPoint(0,7));
	fHSplitter->SetAlignment(B_HORIZONTAL);
	fHSplitter->SetBarAlignmentLocked(true);
	fHSplitter->SetBarPosition(BPoint(0,Bounds().Width() - 195));
	fHSplitter->SetResizeViewOne(true, true);
	view->AddChild(fHSplitter);
	fChatEntry->MakeFocus(true);
	
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

	HCaption *caption = new HCaption(statusrect,"info",NULL);
	box->AddChild(caption);
	view->AddChild(box);	
}

/***********************************************************
 * MessageReceived
 ***********************************************************/
void
HMsgChatWindow::MessageReceived(BMessage *message)
{
	switch(message->what)
	{
	case SENDMSG_SEND_MSG:
		{
			BString text = fChatEntry->Text();
			if(text.Length() > 0)
			{
				TextUtils utils;
				
				utils.ConvertReturnCode(text,K_CR);
				int32 encoding;
				((HApp*)be_app)->Prefs()->GetData("encoding",(int32*)&encoding);
				if(encoding)
					utils.ConvertFromUTF8(text,encoding-1);
				
				message->AddString("text",text);
				message->AddInt32("sock",(int32)fSock);
				be_app->PostMessage(message);
				
				fChatEntry->SetText("");
				InsertChatMessage(text.String(),true);
			}
			break;
		}
	/*********** 受信 ************/ 
	case H_RCV_MSG:
		{
			const char* text;
			if(message->FindString("text",&text) == B_OK)
			{
				InsertChatMessage(text);	
			}
			break;
		}
	case M_LOGOUT_MESSAGE:
		{
			const char* nick;
			if(message->FindString("nick",&nick) == B_OK)
			{
				BString text = "<< ";
				text << nick << " " << _("logout") << " >>";
				InsertChatMessage(text.String(),true);
			}
			break;
		}
	default:
		BWindow::MessageReceived(message);
	}
}

/***********************************************************
 * InsertChatMsg
 ***********************************************************/
void
HMsgChatWindow::InsertChatMessage(const char* text,bool byme)
{
	BFont font;
	rgb_color color;
	fChatView->GetFontAndColor(0,&font,&color);
	
	HCaption *caption = cast_as(FindView("info"),HCaption);
	if(caption)
		caption->SetTime(time(NULL));
	
	text_run	run;
	run.offset = 0;
	run.font = font;
	if(byme)
		run.color = fNickColor;
	else
		run.color = fChatColor;
	
	text_run_array	array;
	array.count = 1;
	array.runs[0] = run;
	
	TextUtils utils;
	BString str = text;
	int32 encoding;
	utils.ConvertReturnsToLF(str);
	((HApp*)be_app)->Prefs()->GetData("encoding",(int32*)&encoding);
	if(encoding)
		utils.ConvertToUTF8(str,encoding-1);
	
	fChatView->Insert(str.String(),&array);
	fChatView->Insert("\n",&array);	

	be_app->PostMessage(SOUND_CHAT_SND);
	float min,max;
	(fChatView->ScrollBar(B_VERTICAL))->GetRange(&min,&max);
	(fChatView->ScrollBar(B_VERTICAL))->SetValue(max);
}

/***********************************************************
 * QuitRequested
 ***********************************************************/
bool
HMsgChatWindow::QuitRequested()
{
	BMessage msg(M_CLOSE_MSG_CHAT_WINDOW);
	msg.AddPointer("pointer",this);
	be_app->PostMessage(&msg);
	return BWindow::QuitRequested();
}