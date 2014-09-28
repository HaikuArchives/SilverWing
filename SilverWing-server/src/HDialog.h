#ifndef __HDIALOG_H__
#define __HDIALOG_H__

#include <Window.h>
enum{
	CHANGE_TEXT = 'CHTE',
	OK_MESSAGE = 'DIOK'
};

class HDialog :public BWindow {
public:
					HDialog(BRect rect
							,const char* title
							,const char* textlabel
							,const char* buttonlabel="OK");
		virtual		~HDialog();
			void	SetParent(BWindow *win) {fParent = win;}
		
protected:
	virtual void	MessageReceived(BMessage *message);
			void	InitGUI(const char* textlabel,const char* buttonlabel);
private:
	BWindow		*fParent;
};
#endif