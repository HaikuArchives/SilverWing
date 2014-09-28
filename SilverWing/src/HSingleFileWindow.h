#ifndef __SINGLEFILEWINDOW_H__
#define __SINGLEFILEWINDOW_H__

#include <Window.h>
#include <FilePanel.h>
#include <String.h>

class HFileList;

class HSingleFileWindow :public BWindow {
public:
					HSingleFileWindow(BRect rect
									,const char *name
									,bool IsSilverWing = false);
	virtual			~HSingleFileWindow();
			void 	InsertFileItem(const char* name,
							const char* type,
							const char* creator,
							uint32 size,
							bool isFolder,
							uint32 index,
							uint32 modified = 0);
			void 	SendDownload(const char* remotepath
								,const char* localpath);
			void 	SendUpload(const char* remotepath
								,const char* localpath
								,bool resume);
			void	SendMove(const char* filename,
							const char* filepath,
							const char* destpath);
			void 	SetParent(BLooper *looper) {fTarget = looper;}
protected:
	virtual void 	MessageReceived(BMessage *msg);
	virtual bool 	QuitRequested();
			void	StartBarberPole(bool start);
			void 	InitGUI(bool IsSilverWing);
private:
			HFileList		*fListView;
			BFilePanel		*fFilePanel;
			BString			fPath;
			BLooper			*fTarget;
};
#endif