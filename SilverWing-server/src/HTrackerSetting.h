#ifndef __HTRACKER_SETTING_H__
#define __HTRACKER_SETTING_H__

#include <View.h>
#include <TextControl.h>

enum{
	M_DEL_MSG = 'MDEL',
	M_ADD_MSG = 'MADT',
	M_SEL_CHANGE_MSG = 'MSCM'
};

class HTrackerItem;
class HListView;

class HTrackerSetting :public BView {
public:
				HTrackerSetting(BRect rect,const char* name);
virtual 		~HTrackerSetting();
virtual	void	MessageReceived(BMessage *message);
		void	LoadTrackers();
		void	SaveTrackers();
		void	RemoveAll();
		void	InitGUI();
		bool	FindItem(const char *name,HTrackerItem **out);
protected:
	HListView*	fListView;
	BTextControl *fAddress;
	BTextControl *fLogin;
	BTextControl *fPassword;

};
#endif
