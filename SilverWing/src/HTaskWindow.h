#ifndef __HTASKWINDOW_H__
#define __HTASKWINDOW_H__

#include <Window.h>

class HTaskView;

#define VIEW_HEIGHT 30

enum{
	T_NORMAL_TYPE = 1,
	T_FILE_TRANS_TYPE = 2
};

enum{
	M_REMOVE_TASK = 'MRET',
	M_ADD_TASK = 'MADD',
	M_UPDATE_TASK = 'MUPD',
	M_REMOVE_ALL_TASK = 'MALL',
	M_SET_MAX_TASK = 'MSMT',
	M_REMOVE_TASK_FROM_BUTTON = 'MREB',
	M_DOWNLOAD_TASK = 'MDOT',
	M_UPLOAD_TASK = 'MUPT',
	M_TRANS_COMPLETE = 'MTRC',
	M_RECEIVE_UPDATE_QUEUE = 'MQUE'
};

class HTaskWindow :public BWindow {
public:
					HTaskWindow(BRect rect);
		virtual		~HTaskWindow();

			void	AddTask(const char* name,uint32 task);
			void	UpdateTask(uint32 task,float percent);
			void	RemoveTask(uint32 task);
			void	AddView(HTaskView *view);
			void	RemoveView(HTaskView* view);
			void	RemoveAllView();
		HTaskView*	LookupTaskView(uint32 task);
		HTaskView*	NextFileTrans(HTaskView* view);
			int32	CountFileTrans();
			void	ShowHide(bool show);
		HTaskView*  FindFileTrans(uint32 ref);
protected:
			void	UpdateScrollBar();
	virtual bool	QuitRequested();
	virtual void	MessageReceived(BMessage *message);
};
#endif