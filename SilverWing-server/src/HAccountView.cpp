#include <CheckBox.h>
#include <Alert.h>
#include <ClassInfo.h>

#include "HAccountView.h"
#include "AppUtils.h"


/**************************************************************
 * Constructor.
 **************************************************************/
HAccountView::HAccountView(BRect frame,const char* name)
	:BView(frame,name,B_FOLLOW_ALL,B_WILL_DRAW)
	,fDirty(false)
	,fAccount("")
{
	this->SetViewColor(216,216,216,0);
	BRect rect = Bounds();
	rect.top += 10;
	rect.left += 10;
	rect.right -= 10;
	rect.bottom = rect.top + 15;
	
	const int divider = static_cast<int>(this->StringWidth("Password:") + 5);
	fLogin = new BTextControl(rect,"login","Login:","",NULL);
	fLogin->SetDivider(divider);
	fLogin->SetEnabled(false);
	this->AddChild(fLogin);
	
	rect.OffsetBy(0,22);
	fPassword = new BTextControl(rect
								,"password"
								,"Password:"
								,""
								,new BMessage(M_DIRTY_MSG));
	fPassword->SetDivider(divider);
	this->AddChild(fPassword);
	
	BCheckBox *checkBox;
	rect.OffsetBy(20,32);
	checkBox = new BCheckBox(rect
							,"download"
							,"Can download files"
							,new BMessage(M_DIRTY_MSG));
	this->AddChild(checkBox);
	
	rect.OffsetBy(0,22);
	checkBox = new BCheckBox(rect
							,"upload"
							,"Can upload files"
							,new BMessage(M_DIRTY_MSG));
	this->AddChild(checkBox);
	
	rect.OffsetBy(0,22);
	checkBox = new BCheckBox(rect
							,"uploaduploads"
							,"Can upload files to Uploads folder"
							,new BMessage(M_DIRTY_MSG));
	checkBox->SetTarget(this);
	this->AddChild(checkBox);

	rect.OffsetBy(0,22);
	checkBox = new BCheckBox(rect
							,"view"
							,"Can view files"
							,new BMessage(M_DIRTY_MSG));
	checkBox->SetTarget(this);
	this->AddChild(checkBox);

	rect.OffsetBy(0,22);
	checkBox = new BCheckBox(rect
							,"delete"
							,"Can delete files"
							,new BMessage(M_DIRTY_MSG));
	checkBox->SetTarget(this);
	this->AddChild(checkBox);
	
	rect.OffsetBy(0,22);
	checkBox = new BCheckBox(rect
							,"rename"
							,"Can rename files"
							,new BMessage(M_DIRTY_MSG));
	checkBox->SetTarget(this);
	this->AddChild(checkBox);
	
	rect.OffsetBy(0,22);
	checkBox = new BCheckBox(rect
							,"move"
							,"Can move files"
							,new BMessage(M_DIRTY_MSG));
	checkBox->SetTarget(this);
	this->AddChild(checkBox);
	
	rect.OffsetBy(0,22);
	checkBox = new BCheckBox(rect
							,"createfolder"
							,"Can create folder"
							,new BMessage(M_DIRTY_MSG));
	checkBox->SetTarget(this);
	this->AddChild(checkBox);
	
	rect.OffsetBy(0,22);
	checkBox = new BCheckBox(rect
							,"getinfo"
							,"Can get user info"
							,new BMessage(M_DIRTY_MSG));
	checkBox->SetTarget(this);
	this->AddChild(checkBox);
	
	rect.OffsetBy(0,22);
	checkBox = new BCheckBox(rect
							,"kick"
							,"Can kick users"
							,new BMessage(M_DIRTY_MSG));
	checkBox->SetTarget(this);
	this->AddChild(checkBox);
	
	rect.OffsetBy(0,22);
	checkBox = new BCheckBox(rect
							,"ban"
							,"Can ban users"
							,new BMessage(M_DIRTY_MSG));
	checkBox->SetTarget(this);
	this->AddChild(checkBox);
	
	rect.OffsetBy(0,22);
	checkBox = new BCheckBox(rect
							,"readnews"
							,"Can view news"
							,new BMessage(M_DIRTY_MSG));
	checkBox->SetTarget(this);
	this->AddChild(checkBox);
	
	rect.OffsetBy(0,22);
	checkBox = new BCheckBox(rect
							,"postnews"
							,"Can post news"
							,new BMessage(M_DIRTY_MSG));
	checkBox->SetTarget(this);
	this->AddChild(checkBox);
	
	rect.OffsetBy(0,22);
	checkBox = new BCheckBox(rect
							,"prvchat"
							,"Can create private chat"
							,new BMessage(M_DIRTY_MSG));
	this->AddChild(checkBox);
	checkBox->SetTarget(this);
}

/**************************************************************
 * Destructor.
 **************************************************************/
HAccountView::~HAccountView()
{
	if(fAccount.Length() != 0)
		this->SaveAccount();
}

/**************************************************************
 * Set selected account.
 **************************************************************/
void
HAccountView::SetAccount(const char* name)
{
	if(fAccount.Length() != 0)
		this->SaveAccount();
	fDirty = false;
	
	BPath path = AppUtils().GetAppDirPath(be_app);
	path.Append("Accounts");	
	path.Append(name);
	
	BFile file(path.Path(),B_READ_ONLY);
	if(file.InitCheck() != B_OK)
	{
		(new BAlert("","Could not find such a account","OK"))->Go();
		return;
	}
	BMessage msg;
	msg.Unflatten(&file);
	
	fLogin->SetText(msg.FindString("login"));
	fPassword->SetText(msg.FindString("password"));
	this->SetValue("download",msg.FindBool("download"));
	this->SetValue("upload",msg.FindBool("upload"));
	this->SetValue("uploaduploads",msg.FindBool("uploaduploads"));
	this->SetValue("view",msg.FindBool("viewfile"));
	this->SetValue("delete",msg.FindBool("delete"));
	this->SetValue("rename",msg.FindBool("rename"));
	this->SetValue("move",msg.FindBool("move"));
	this->SetValue("createfolder",msg.FindBool("createfolder"));
	this->SetValue("getinfo",msg.FindBool("getinfo"));
	this->SetValue("kick",msg.FindBool("kick"));
	this->SetValue("ban",msg.FindBool("ban"));
	this->SetValue("readnews",msg.FindBool("readnews"));
	this->SetValue("postnews",msg.FindBool("postnews"));
	this->SetValue("prvchat",msg.FindBool("prvchat"));
	fAccount = name;
}

/**************************************************************
 * Set checkbox value.
 **************************************************************/
void
HAccountView::SetValue(const char* name,bool which)
{
	BCheckBox *checkBox = cast_as(FindView(name),BCheckBox);
	if(checkBox != NULL)
	{
		checkBox->SetValue(which);
	}
}

/**************************************************************
 * Get checkbox value.
 **************************************************************/
bool
HAccountView::GetValue(const char* name)
{
	bool result = false;
	BCheckBox *checkBox = cast_as(FindView(name),BCheckBox);
	if(checkBox != NULL)
	{
		result = checkBox->Value();
	}
	return result;
}

/**************************************************************
 * Save account.
 **************************************************************/
void
HAccountView::SaveAccount()
{
	if(!fDirty)
		return;
	BPath path = AppUtils().GetAppDirPath(be_app);
	path.Append("Accounts");
	path.Append(fAccount.String());

	BFile file(path.Path(),B_WRITE_ONLY|B_ERASE_FILE);
	if(file.InitCheck() != B_OK)
		return;
	BMessage msg;
	
	msg.AddString("login",fLogin->Text());
	msg.AddString("password",fPassword->Text());
	msg.AddBool("download",GetValue("download"));
	msg.AddBool("upload",GetValue("upload"));
	msg.AddBool("uploaduploads",GetValue("uploaduploads"));
	msg.AddBool("viewfile",GetValue("view"));
	msg.AddBool("delete",GetValue("delete"));
	msg.AddBool("rename",GetValue("rename"));
	msg.AddBool("move",GetValue("move"));
	msg.AddBool("createfolder",GetValue("createfolder"));
	msg.AddBool("getinfo",GetValue("getinfo"));
	msg.AddBool("kick",GetValue("kick"));
	msg.AddBool("ban",GetValue("ban"));
	msg.AddBool("readnews",GetValue("readnews"));
	msg.AddBool("postnews",GetValue("postnews"));
	msg.AddBool("prvchat",GetValue("prvchat"));
	
	msg.Flatten(&file);
	
}

/***********************************************************
 * MessageReceived
 ***********************************************************/
void
HAccountView::MessageReceived(BMessage *message)
{
	switch(message->what)
	{
	case M_DIRTY_MSG:
		fDirty = true;
		break;
	default:
		BView::MessageReceived(message);
	}
}