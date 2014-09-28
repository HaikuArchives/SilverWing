#ifndef __HUSERSETTING_VIEW_H__
#define __HUSERSETTING_VIEW_H__

#include <View.h>
#include <string>

class HUserList;
class HProgressWindow;

enum{
M_GATHER_ICON_MSG = 'MGIM',
M_LIST_CLICKED = 'MLIC',
M_ADD_ICON_ITEM = 'MADI',
M_END_SEARCH = 'MEND'
};

class HUserSettingView :public BView {
public:
				HUserSettingView(BRect rect);
	virtual		~HUserSettingView();
const char*		Nick();
		uint32	Icon();
		void	RemoveAll();
		void	GatherAllIcons();
		void	SetNick(const char* nick);
		void	SetIcon(int32 icon);


protected:
virtual void	MessageReceived(BMessage *message);
static	int32	ThreadFunc(void* data);
void	InitGUI();

private:
		HUserList	*fUserList;
		HProgressWindow *fProgressWin;
};
#endif