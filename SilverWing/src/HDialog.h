#ifndef __HDIALOG_H__
#define __HDIALOG_H__

#include <Window.h>
enum{
	CHANGE_TEXT = 'CHTE',
	OK_MESSAGE = 'DIOK'
};

enum{
	CREATE_BUNDLE = 0x001,
	CREATE_FOLDER = 0x002,
	CREATE_CATEGORY = 0x003
};

class HDialog :public BWindow {
public:
				HDialog(BRect rect
						,const char* title
						,BMessage *message
						,const char* textlabel
						,const char* buttonlabel="OK");
		virtual	~HDialog();

protected:
virtual void	MessageReceived(BMessage *message);
		void	InitGUI(const char* textlabel,const char* buttonlabel);
private:
		BMessage *fMessage;
};
#endif