#ifndef __HWINDOW_H__
#define __HWINDOW_H__

#include <Be.h>

/***** Message *****/
enum{
	M_TEST1_MESSAGE = 'MTS1',
	M_TEST2_MESSAGE = 'MTS2',
	M_TEST3_MESSAGE = 'MTS3',
	M_TEST4_MESSAGE = 'MTS4'
};

class HWindow :public BWindow {
public:
				HWindow(BRect rect ,const char* name);
virtual			~HWindow();
virtual void	MessageReceived(BMessage *message);
		
protected:
		void	SetText(const char* text);
		void	UpdateToolbarButtons(BMessage *message);
		void	InitGUI();
		void	InitMenu();
virtual	bool	QuitRequested();
	BBitmap*	GetBitmapResource(type_code type,const char* name);
	BStringView* fStringView;
};
#endif