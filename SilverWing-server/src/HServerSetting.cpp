#include "HServerSetting.h"
#include "HApp.h"
#include "HPrefs.h"
#include "NumberControl.h"

#include <TextControl.h>
#include <CheckBox.h>
#include <ClassInfo.h>
#include <String.h>

/**************************************************************
 * Constructor.
 **************************************************************/
HServerSetting::HServerSetting(BRect rect,const char* name)
	:BView(rect,name,B_FOLLOW_ALL,B_WILL_DRAW)
{
	this->InitGUI();
}

/**************************************************************
 * Destructor.
 **************************************************************/
HServerSetting::~HServerSetting()
{

}

/**************************************************************
 * MessageReceived.
 **************************************************************/
void
HServerSetting::InitGUI()
{
	this->SetViewColor(216,216,216,0);
	
	BRect rect = Bounds();
	rect.top += 10;
	rect.left += 10;
	rect.right = rect.left + 240;
	rect.bottom = rect.top + 15;
	const char* address;
	((HApp*)be_app)->Prefs()->GetData("address",&address);
	fAddress = new BTextControl(rect,"address","Address:",address,NULL);
	const int divider = static_cast<int>(fAddress->StringWidth("Simultaneous Downloads:") + 5);
	fAddress->SetDivider(divider);
	this->AddChild(fAddress);
	rect.OffsetBy(0,22);
	uint32 integer;
	((HApp*)be_app)->Prefs()->GetData("port",(int32*)&integer);

	fPort = new NumberControl(rect,"port","Port:",integer,NULL);
	fPort->SetDivider(divider);	
	this->AddChild(fPort);

	rect.OffsetBy(0,22);

	((HApp*)be_app)->Prefs()->GetData("max_users",(int32*)&integer);
	fMaxUser = new NumberControl(rect,"max","Max users:",integer,NULL);
	fMaxUser->SetDivider(divider);
	this->AddChild(fMaxUser);
	rect.OffsetBy(0,22);
	
	((HApp*)be_app)->Prefs()->GetData("sim_download",(int32*)&integer);
	fSimDownloads = new NumberControl(rect,"simusers","Simultaneous Downloads:",integer,NULL);
	fSimDownloads->SetDivider(divider);
	this->AddChild(fSimDownloads);
	rect.OffsetBy(0,22);
	
	((HApp*)be_app)->Prefs()->GetData("sim_upload",(int32*)&integer);
	fSimUploads = new NumberControl(rect,"simusers","Simultaneous Uploads:",integer,NULL);
	fSimUploads->SetDivider(divider);
	this->AddChild(fSimUploads);
	
	rect.OffsetBy(0,35);
	bool enable;
	((HApp*)be_app)->Prefs()->GetData("save_log",&enable);
	fSaveLog = new BCheckBox(rect,"savelog","Save log into Log.txt",NULL);
	fSaveLog->SetValue(enable);
	this->AddChild(fSaveLog);
	
	rect.OffsetBy(0,22);
	((HApp*)be_app)->Prefs()->GetData("mutithread_news",&enable);
	fThreadedNews = new BCheckBox(rect,"threadednews","Use threaded news.",NULL);
	fThreadedNews->SetValue(enable);
	fThreadedNews->SetEnabled(false);
	this->AddChild(fThreadedNews);
	
	rect.OffsetBy(0,22);
	((HApp*)be_app)->Prefs()->GetData("sound",&enable);
	fSound = new BCheckBox(rect,"sound","Enable sound effects.",NULL);
	fSound->SetValue(enable);
	this->AddChild(fSound);
}

/**************************************************************
 * return server address.
 **************************************************************/
const char*
HServerSetting::Address()
{
	return fAddress->Text();
}

/**************************************************************
 * return server port.
 **************************************************************/
uint32
HServerSetting::Port()
{
	return fPort->Value();
}

/**************************************************************
 * return max users.
 **************************************************************/
uint32
HServerSetting::MaxUser()
{
	return fMaxUser->Value();
}

/**************************************************************
 * return sim downloads.
 **************************************************************/
uint32
HServerSetting::SimDownloads()
{
	return fSimDownloads->Value();
}

/**************************************************************
 * return sim uploads.
 **************************************************************/
uint32
HServerSetting::SimUploads()
{
	return fSimUploads->Value();
}

/**************************************************************
 * return save log to file or not.
 **************************************************************/
bool
HServerSetting::SaveLog()
{
	return fSaveLog->Value();
}

/**************************************************************
 * return enable or disable sound effects.
 **************************************************************/
bool
HServerSetting::Sound()
{
	return fSound->Value();
}