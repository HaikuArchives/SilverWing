#ifndef __HAgreeWindow_H__
#define __HAgreeWindow_H__

#include <Window.h>
#include <iostream>
#include "URLTextView.h"


class HAgreeWindow :public BWindow {
public:

							HAgreeWindow(BRect rect,const char* name,const char* text);
				virtual		~HAgreeWindow();
protected:
				virtual void MessageReceived(BMessage *msg);
				virtual bool QuitRequested();
						void InitGUI(const char* text);
						void InsertAgreeMessage(const char* text);
private:
			URLTextView		*agreeview;
};
#endif		
					