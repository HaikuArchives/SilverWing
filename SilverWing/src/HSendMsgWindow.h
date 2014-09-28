#ifndef __HSendMsgWindow_H__
#define __HSendMsgWindow_H__

#include <Window.h>
#include <iostream>
#include "CTextView.h"
//#include "ToolTip.h"
enum{
SENDMSG_INVOKE_CHAT = 'SMIC',
SENDMSG_SEND_MSG = 'SMSM',
SENDMSG_RECEIVE_MSG = 'SMRM'
};

#define SEND_MESSAGE_WIDTH 350
#define SEND_MESSAGE_HEIGHT 250

class HSendMsgWindow :public BWindow {
public:

							HSendMsgWindow(BRect rect,const char* name,uint32	sock);
				virtual		~HSendMsgWindow();
						void InsertMessage(const char* text);
						void SetParentMessage(const char* text);
protected:
				virtual void MessageReceived(BMessage *msg);
						void InitGUI();

private:
			CTextView		*textview;
			uint32			fSock;
			//ToolTip *fToolTip;
};
#endif		
					