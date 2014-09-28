#ifndef __HUSERITEM_H__
#define __HUSERITEM_H__

#include <String.h>
#include <NetworkKit.h>
#include <iostream>
#include <Locker.h>

class HUserTimer;
class TServer;

class HUserItem :public BLocker{
public:
					HUserItem(const char* nick
							,uint16 icon
							,uint32 trans
							,HUserTimer *timer
							,TServer	*server
							,BNetEndpoint *endpoint);
	virtual			~HUserItem();
/************************************/
	BNetEndpoint*	Socket()const {return fSocket;}
	uint32			Index()const  {return fIndex;}
	uint16			Icon()const  {return fIcon;}
	const char*		Nick()const 	{return fNick.String();}
	uint32			Trans()const  {return fTrans;}
	uint16			Color()const  {return fColor;}
	const char*		Account()const {return fAccountName.String();}
	void			SetAccount(const char* name);
	void			ResetAccount();
	void			SetTrans(uint32 trans) {fTrans = trans;}
	void			SetNick(const char* nick) {fNick = nick;}
	void			SetIcon(uint16 icon) {fIcon = icon;}
	void			SetColor(uint16 color) {fColor = color;}
	void			SetIndex(uint32 index){fIndex = index;}
	void			SetSilverWing(bool enable) {fSilverWing = enable;}
	bool			IsSilverWing() const {return fSilverWing;}
	void			ResetIdleTime();
/*************************************/
	bool			CanDownload() const {return fCanDownload;}
	bool			CanUpload() const {return fCanUpload;}
	bool			CanViewFile() const {return fCanViewFile;}
	bool			CanCreateFolder() const {return fCanCreateFolder;}
	bool			CanMoveFile() const {return fCanMoveFile;}
	bool			CanDeleteFile() const {return fCanDeleteFile;}
	bool			CanRenameFile() const {return fCanRenameFile;}
	bool			CanGetInfo() const {return fCanGetInfo;}
	bool			CanReadNews() const {return fCanReadNews;}
	bool			CanPostNews() const {return fCanPostNews;}
	bool			CanCreatePrvChat() const {return fCanCreatePrvChat;}
	bool			CanKickUser() const {return fCanKickUser;}
	bool			CanBanUser() const {return fCanBanUser;}
	bool			CanUploadUploads() const {return fCanUploadUploads;}
/*********************************************************/
protected:
	/****** User Accounts ******/
	bool			fCanDownload;
	bool			fCanUpload;
	bool			fCanUploadUploads;
	bool			fCanViewFile;
	bool			fCanMoveFile;
	bool			fCanGetInfo;
	bool 			fCanDeleteFile;
	bool			fCanRenameFile;
	bool			fCanCreateFolder;
	bool			fCanReadNews;
	bool			fCanPostNews;
	bool			fCanCreatePrvChat;
	bool			fCanKickUser;
	bool			fCanBanUser;
	/******　User infomation ******/
	BString			fNick;
	BString			fPassword;
	uint16			fIcon;
	uint16  		fColor;
	uint32  		fTrans;
	uint32			fIndex;
	BNetEndpoint*	fSocket;
	BString			fAccountName;
	/********* Transfer *********/
	uint32  		fDownloads;
	uint32  		fUploads;
	/********* Idle time ********/
	HUserTimer		*fIdleTimer;
	bool			fSilverWing;	
	TServer			*fServer;
};
#endif