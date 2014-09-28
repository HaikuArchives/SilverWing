#ifndef __HPREFERENCE_WINDOW_H__
#define __HPREFERENCE_WINDOW_H__

#include <Window.h>
class HUserSettingView;
class HOtherSettingView;
class HFontSettingView;
class HFireWallView;

enum{
	M_APPLY_MESSAGE = 'MAPL',
	M_SETTING_CHANGED = 'MSTC',
	M_SET_CHAT_FONT_MSG = 'MSEF'
};

class HSettingWindow :public BWindow {
public:
				HSettingWindow(BRect rect);
	virtual		~HSettingWindow();

protected:
		void	InitGUI();
virtual void	MessageReceived(BMessage *message);
virtual bool	QuitRequested();

private:
		HUserSettingView *usersetting;
		HOtherSettingView* othersetting;
		HFontSettingView*	fontsetting;
		HFireWallView*		firewallsetting;
};
#endif