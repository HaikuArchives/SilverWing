#include "HFontSettingView.h"
#include "MenuUtils.h"
#include "ColorView.h"
#include "HApp.h"
#include "FontMenuItem.h"
#include "HPrefs.h"
#include "TextUtils.h"

#include <stdlib.h>
#include <StringView.h>
#include <String.h>
#include <ClassInfo.h>
#include <iostream>
#include <MenuField.h>

/***********************************************************
 * Constructor.
 ***********************************************************/
HFontSettingView::HFontSettingView(BRect rect)
	:BView(rect,"fontsetting",B_FOLLOW_ALL,B_WILL_DRAW)
{
	InitGUI();
}

/***********************************************************
 * Destructor.
 ***********************************************************/
HFontSettingView::~HFontSettingView()
{

}

/***********************************************************
 * Set up GUIs.
 ***********************************************************/
void
HFontSettingView::InitGUI()
{
	SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	BRect rect = Bounds();
	rect.top += 10;
	rect.left += 10;
	rect.right -= 10;
	rect.bottom = rect.top + 15;
	BMenu *menu = new BMenu("fontmenu");
	MenuUtils utils;
	utils.AddFontMenu(menu,M_FONT_CHANGED,this);
	
	//****** 自前にマークを付けているので以下のコードはいらない *********//
	//menu->SetLabelFromMarked(true);
	//menu->SetRadioMode(true);
	BMenuField *menufield = new BMenuField(rect,"font",_("Chat Font:"),menu);
	menufield->SetDivider(this->StringWidth(_("Chat Font:"))+10);
	this->AddChild(menufield);
	//****** 設定を読み込みマークを付ける ************//	
	const char* family;
	const char* style;
	((HApp*)be_app)->Prefs()->GetData("font_family",&family);
	((HApp*)be_app)->Prefs()->GetData("font_style",&style);	
	BMenuItem *item = menu->FindItem(family);
	::strcpy(fFamily,family);
	::strcpy(fStyle,style);
	if(item != NULL)
	{
		item->SetMarked(true);
		int32 index = menu->IndexOf(item);
		BMenu *subMenu = menu->SubmenuAt(index);
		item = subMenu->FindItem(style);
		if(item != NULL)
		{
			item->SetMarked(true);
		}
	}
	BString title = family;
	title << "  ";
	title << style;
	menufield->Menu()->Superitem()->SetLabel( title.String() );
	//********** Font size menu **************//
	rect.OffsetBy(0,25);
	menu = new BMenu("sizemenu");
	for(register int32 i = 6;i < 27;i++)
	{
		BString str;
		str << i; 
		utils.AddMenuItem(menu,str.String(),M_FONT_SIZE,this,this->Window());
	}
	menu->SetLabelFromMarked(true);
	menu->SetRadioMode(true);
	menufield = new BMenuField(rect,"size",_("Font Size:"),menu);
	menufield->SetDivider(this->StringWidth(_("Chat Font:"))+10);
	int32 size;
	((HApp*)be_app)->Prefs()->GetData("font_size",&size);
	BString sSize;
	sSize << size;
	item = menu->FindItem(sSize.String());
	if(item != NULL)
		item->SetMarked(true);
	this->AddChild(menufield);
	rect.OffsetBy(0,30);
	// BackGround Color ----------------------------------
	const int32 offset = static_cast<int32>(StringWidth(_("Background Color:"))+5);
	rect.right = rect.left + offset;
	BStringView *stringView = new BStringView(rect,"StringView" ,_("Font Color:"),B_WILL_DRAW,0);
	this->AddChild(stringView);
	
	rect.OffsetBy(offset,0);
	rgb_color forecolor;
	int32 indexColor;
	((HApp*)be_app)->Prefs()->GetData("font_color",&indexColor);
	forecolor.red = indexColor >> 16;
	forecolor.green = indexColor >> 8;
	forecolor.blue = indexColor;
	rect.right = rect.left + 200;
	ColorView *colorView = new ColorView("FontColor",rect,forecolor,this);
	this->AddChild(colorView);
	rect.OffsetBy(-offset,30);
	// BackGround Color ----------------------------------
	rect.right = rect.left + offset;
	stringView = new BStringView(rect,"StringView" ,_("Background Color:"),B_WILL_DRAW,0);
	this->AddChild(stringView);
	rect.OffsetBy(offset,0);
	rgb_color backcolor;
	
	rect.right = rect.left + 200;
	((HApp*)be_app)->Prefs()->GetData("back_color",&indexColor);
	backcolor.red = indexColor >> 16;
	backcolor.green = indexColor >> 8;
	backcolor.blue = indexColor;
	colorView = new ColorView("BackColor",rect,backcolor,this);
	this->AddChild(colorView);
	rect.OffsetBy(-offset,30);
	// nick color ------------------------------------------	
	rect.right = rect.left + offset;
	stringView = new BStringView(rect,"StringView" ,_("Nickname Color:"),B_WILL_DRAW,0);
	this->AddChild(stringView);
	rect.OffsetBy(offset,0);
	rgb_color nickcolor;
	
	rect.right = rect.left + 200;
	((HApp*)be_app)->Prefs()->GetData("nick_color",&indexColor);
	nickcolor.red = indexColor >> 16;
	nickcolor.green = indexColor >> 8;
	nickcolor.blue = indexColor;
	colorView = new ColorView("NickColor",rect,nickcolor,this);
	this->AddChild(colorView);
	rect.OffsetBy(-offset,30);
	// url color ------------------------------------------	
	rect.right = rect.left + offset;
	stringView = new BStringView(rect,"StringView" ,_("URL Color:"),B_WILL_DRAW,0);
	this->AddChild(stringView);
	rect.OffsetBy(offset,0);
	rgb_color urlcolor;
	
	rect.right = rect.left + 200;
	((HApp*)be_app)->Prefs()->GetData("url_color",&indexColor);
	urlcolor.red = indexColor >> 16;
	urlcolor.green = indexColor >> 8;
	urlcolor.blue = indexColor;
	colorView = new ColorView("UrlColor",rect,urlcolor,this);
	this->AddChild(colorView);
	rect.OffsetBy(-offset,30);
	// Encoding ------------------------------------------
	BMenu *encodingMenu = new BMenu("Encoding");
	const char* ENCODING[] = {
	"UTF-8",
	"ISO-8859-1","ISO-8859-2","ISO-8859-3","ISO-8859-4",
	"ISO-8859-5","ISO-8859-6","ISO-8859-7","ISO-8859-8",
	"ISO-8859-9","ISO-8859-10","MacRoman","Shift-JIS","EUC",
	"JIS","EUC-kr","KOI8-R"
	};

	for(register int i = 0;i < 17;i++)
		utils.AddMenuItem(encodingMenu,ENCODING[i],(uint32)0,this->Window(),this->Window());
	encodingMenu->SetRadioMode(true);
	encodingMenu->SetLabelFromMarked(true);
	int32 encode;
	((HApp*)be_app)->Prefs()->GetData("encoding",&encode);
	if(encode != EUC_KR_ENCODING+1)
		encodingMenu->ItemAt(encode)->SetMarked(true);
	else
		encodingMenu->ItemAt(15)->SetMarked(true);
	menufield = new BMenuField(rect,"encoding",_("Character Encoding:"),encodingMenu);
	menufield->SetDivider(this->StringWidth(_("Character Encoding:"))+10);
	this->AddChild(menufield);
}

/***********************************************************
 * MessageReceived.
 ***********************************************************/
void
HFontSettingView::MessageReceived(BMessage *message)
{
	switch(message->what)
	{
	/******** フォントが変更された時 **********/
	case k_font_menu_msg_type:
	{
		const char* family = message->FindString("family");
		const char* style = message->FindString("style");
		::strcpy(fFamily,family);
		::strcpy(fStyle,style);
		BString title = fFamily;
		title << "  ";
		title << fStyle;
		BMenuField *menufield = cast_as(FindView("font"),BMenuField);
		
		menufield->Menu()->Superitem()->SetLabel( title.String() ); 
		BMenu *menu = menufield->Menu();
		MenuUtils utils;
		utils.DeMarkAll(menu);
		BMenuItem *item = menu->FindItem(fFamily);
		if(item != NULL)
		{
			item->SetMarked(true);
			int32 index = menu->IndexOf(item);
			item = (menu->SubmenuAt(index))->FindItem(style);
			if(item != NULL)
				item->SetMarked(true);
		}
		
		break;
	}
	default:
		BView::MessageReceived(message);
	}
}

/***********************************************************
 * Return font family.
 ***********************************************************/
const char*
HFontSettingView::FontFamily()
{
	const char* font = fFamily;
	return font;
}

/***********************************************************
 * Return font style.
 ***********************************************************/
const char*
HFontSettingView::FontStyle()
{
	const char* font = fStyle;
	return font;
}

/***********************************************************
 * Return font size.
 ***********************************************************/
int32
HFontSettingView::FontSize()
{
	int32 result = 12;
	
	BMenuField *menufield = cast_as(FindView("size"),BMenuField);
	BMenuItem *item = (menufield->Menu())->FindMarked();
	if(item != NULL)
	{
		result = atol(item->Label());	
	}	
	return result;
}

/***********************************************************
 * Return font color.
 ***********************************************************/
uint32
HFontSettingView::FontColor()
{
	ColorView *colorView = cast_as(FindView("FontColor"),ColorView);
	rgb_color color = colorView->Color();
	uint32 indexColor = color.red << 16|color.green << 8|color.blue;
	return indexColor;	
}

/***********************************************************
 * Return bg color.
 ***********************************************************/
uint32
HFontSettingView::BackColor()
{
	ColorView *colorView = cast_as(FindView("BackColor"),ColorView);
	rgb_color color = colorView->Color();
	uint32 indexColor = color.red << 16|color.green << 8|color.blue;
	return indexColor;	
}

/***********************************************************
 * Return nick color.
 ***********************************************************/
uint32
HFontSettingView::NickColor()
{
	ColorView *colorView = cast_as(FindView("NickColor"),ColorView);
	rgb_color color = colorView->Color();
	uint32 indexColor = color.red << 16|color.green << 8|color.blue;
	return indexColor;	
}

/***********************************************************
 * Return encoding.
 ***********************************************************/
uint32
HFontSettingView::Encoding()
{
	BMenuField *menufield = cast_as(FindView("encoding"),BMenuField);
	BMenu *menu = menufield->Menu();
	int32 index =  menu->IndexOf(menu->FindMarked());
	
	if(index == 15 )
		index = EUC_KR_ENCODING+1;

	return index;
}

/***********************************************************
 * Return url color.
 ***********************************************************/
uint32
HFontSettingView::URLColor()
{
	ColorView *colorView = cast_as(FindView("UrlColor"),ColorView);
	rgb_color color = colorView->Color();
	uint32 indexColor = color.red << 16|color.green << 8|color.blue;
	return indexColor;	
}