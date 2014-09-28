#ifndef __HFileWindow_H__
#define __HFileWindow_H__
/*********** SYSTEM HEADER ***********/
#include <Window.h>
#include <FilePanel.h>
#include <iostream>
#include <String.h>


class HFileList;
class HFileItem;
class HTaskWindow;

enum{
FILE_FILE_REQUEST = 'FFRQ',
FILE_SENDCHAT_MSG = 'FSCM',
FILE_RECEIVE_MSG = 'FRMG',
FILE_REMOVE_POINTER= 'FRPT',
FILE_ADD_POINTER= 'FAPT',
FILE_ITEM_CLICK_MSG = 'FICM',
FILE_GET_FILE_MSG= 'FGFM',
FILE_PUT_FILE_MSG= 'FPFM',
FILE_PARENT_FOLDER_MSG = 'FPAF',
FILE_REFRESH_LSIT = 'FREF',
FILE_MAKE_FOLDER = 'FMKF',
FILE_INFO_MESSAGE = 'FINF',
FILE_DELETE_FILE = 'FDEL',
};

static char dir_char = '\\';

class HFileWindow :public BWindow {
public:

				HFileWindow(BRect rect
							,const char* name
							,bool IsSilverWing = false);
		virtual	~HFileWindow();
		void 	InsertFileItem(const char* name,
						const char* type,
						const char* creator,
						uint32 size,
						bool isFolder,
						uint32 index,
						uint32 modified = 0);
		void 	SetTrans(int32 trans) {fTrans = trans;}
		int32 	Trans() const{return fTrans;}
		void 	SetParent(BLooper *looper) {fTarget = looper;}
		void 	SendDownload(const char* remotepath,const char* localpath);
		void 	SendUpload(const char* remotepath,const char* localpath,bool resume);
		void	GetSubItems(HFileItem *item);			
		void	SendMove(const char* filename,
							const char* filepath,
							const char* destpath);
		void	StartBarberPole(bool start);
protected:
	virtual void 	MessageReceived(BMessage *msg);
	virtual bool 	QuitRequested();
			void 	InitGUI(bool IsSilverWing);
		
private:
			HFileList		*fListView;
			BFilePanel		*fFilePanel;
			BString			fPath;
			int32			fTrans;
			BLooper			*fTarget;
			uint32			fItemIndex;
};
#endif		
					