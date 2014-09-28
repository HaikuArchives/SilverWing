#include <Autolock.h>
#include <Alert.h>

#include "TServer.h"
#include "HUserTimer.h"
#include "HUserItem.h"
#include "AppUtils.h"

/**************************************************************
 * Constructor.
 **************************************************************/
HUserItem::HUserItem(const char* nick
					,uint16 icon
					,uint32 trans
					,HUserTimer *timer
					,TServer *server
					,BNetEndpoint *endpoint)
		:BLocker()
		,fNick(nick)
		,fIcon(icon)
		,fTrans(trans)
		,fIndex(0)
		,fSocket(endpoint)
		,fDownloads(0)
		,fUploads(0)
		,fIdleTimer(timer)
		,fSilverWing(false)
		,fServer(server)
{
	fIdleTimer->StartTimer();
}

/**************************************************************
 * Destructor.
 **************************************************************/
HUserItem::~HUserItem()
{
	if(fSocket)
	{
		fSocket->Close();
		delete fSocket;
	}
	if(fIdleTimer)
		delete fIdleTimer;
}

/**************************************************************
 * Set up account info.
 **************************************************************/
void
HUserItem::SetAccount(const char* name)
{
	BAutolock lock(this);
	BPath path = AppUtils().GetAppDirPath(be_app);
	path.Append("Accounts");

	if(strlen(name) == 0)
	{
		path.Append("guest");
		fAccountName="guest";
	}else{
		path.Append(name);
		fAccountName=name;
	}
	BFile file(path.Path(),B_READ_ONLY);
	if(file.InitCheck() == B_OK)
	{
		BMessage msg;
		msg.Unflatten(&file);
		fCanDownload = msg.FindBool("download");
		fCanUpload = msg.FindBool("upload");
		fCanUploadUploads = msg.FindBool("uploaduploads");
		fCanViewFile = msg.FindBool("viewfile");
		fCanDeleteFile = msg.FindBool("delete");
		fCanMoveFile = msg.FindBool("move");
		fCanRenameFile = msg.FindBool("rename");
		fCanCreateFolder = msg.FindBool("createfolder");
		fCanGetInfo = msg.FindBool("getinfo");
		fCanKickUser = msg.FindBool("kick");
		fCanBanUser = msg.FindBool("ban");
		fCanReadNews = msg.FindBool("readnews");
		fCanPostNews = msg.FindBool("postnews");
		fCanCreatePrvChat = msg.FindBool("prvchat");
	}
	if(fCanKickUser||fCanBanUser)
		fColor = 2;
	else
		fColor = 0;
	return;
}

/***********************************************************
 * ResetAccout
 ***********************************************************/
void
HUserItem::ResetAccount()
{
	this->SetAccount(fAccountName.String());
}

/***********************************************************
 * Reset Idle Time
 ***********************************************************/
void
HUserItem::ResetIdleTime()
{
	if(fIdleTimer)
		fIdleTimer->ResetTimer();
	if(fColor%2 != 0)
	{
		SetColor(fColor-1);
		fServer->SendUserChange(Index(),Nick(),Icon(),Color());
	}
}