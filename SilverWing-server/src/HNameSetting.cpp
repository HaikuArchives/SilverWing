#include "HNameSetting.h"
#include "HApp.h"
#include "HPrefs.h"
#include "TextUtils.h"

#include <Menu.h>
#include <MenuItem.h>


/**************************************************************
 * Constructor.
 **************************************************************/
HNameSetting::HNameSetting(BRect frame,const char* name)
	:BView(frame,name,B_FOLLOW_ALL,B_WILL_DRAW)
{
	this->SetViewColor(216,216,216,0);
	
	BRect rect = Bounds();
	rect.top += 10;
	rect.left += 10;
	rect.right = rect.left + 300;
	rect.bottom = rect.top + 15;
	const char* server_name;
	((HApp*)be_app)->Prefs()->GetData("server_name",&server_name);
	fNameControl = new BTextControl(rect,"name","Name:",server_name,NULL);
	fNameControl->SetDivider(fNameControl->StringWidth("Description:")+4);
	this->AddChild(fNameControl);	
	
	rect.OffsetBy(0,30);
	
	const char* desc;
	((HApp*)be_app)->Prefs()->GetData("server_desc",&desc);
	fDescControl = new BTextControl(rect,"desc","Description:",desc,NULL);
	fDescControl->SetDivider(fDescControl->StringWidth("Description:")+4);
	this->AddChild(fDescControl);
	
	rect.OffsetBy(0,30);
	BMenu *menu = new BMenu("encoding");
	menu->AddItem(new BMenuItem("UTF8",NULL));
	menu->AddItem(new BMenuItem("Shift-JIS",NULL));
	menu->AddItem(new BMenuItem("EUC-KR",NULL));
	menu->SetRadioMode(true);
	menu->SetLabelFromMarked(true);
	int32 encoding;
	((HApp*)be_app)->Prefs()->GetData("encoding",&encoding);
	fEncodeMenu = new BMenuField(rect,"encoding","Encoding:",menu);
	fEncodeMenu->SetDivider(StringWidth("Encoding:")+3);
	AddChild(fEncodeMenu);
	
	BMenuItem *item = NULL;
	switch(encoding)
	{
	case -1:
		item = menu->FindItem("UTF8");
		break;
	case SJIS_ENCODING:
		item = menu->FindItem("Shift-JIS");
		break;
	case EUC_KR_ENCODING:
		item = menu->FindItem("EUC-KR");
		break;
	default:
		item = menu->FindItem("UTF8");
	}
	if(item)
		item->SetMarked(true);
}

/**************************************************************
 * Destructor.
 **************************************************************/
HNameSetting::~HNameSetting()
{
}

/**************************************************************
 * return server descriptions.
 **************************************************************/
const char*
HNameSetting::Desc()
{
	return fDescControl->Text();
}

/**************************************************************
 * return server name.
 **************************************************************/
const char*
HNameSetting::Name()
{
	return fNameControl->Text();
}

/***********************************************************
 * Encoding
 ***********************************************************/
int32
HNameSetting::Encoding()
{
	int32 result;
	BMenu *menu = fEncodeMenu->Menu();
	BMenuItem *item = menu->FindMarked();
	if(!item)
		return -1;
	int32 index = menu->IndexOf(item);
	switch(index)
	{
	case 0:
		result = -1;
		break;
	case 1:
		result = SJIS_ENCODING;
		break;
	case 2:
		result = EUC_KR_ENCODING;
		break;
	default:
		result = -1;
	}
	return result;
}