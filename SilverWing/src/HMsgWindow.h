#ifndef __HMsgWindow_H__
#define __HMsgWindow_H__

#include <Window.h>
#include <iostream>
#include <String.h>
#include "URLTextView.h"

enum{
MESSAGE_INVOKE_CHAT = 'MEIC',
MESSAGE_SENDCHAT_MSG = 'MESM',
MESSAGE_REPLY_MSG = 'MERM',
MESSAGE_CHAT_MSG = 'MSCM'
};

class MovieView;

class HMsgWindow :public BWindow {
public:

							HMsgWindow(BRect rect
									,const char* name
									,uint32 sock
									,uint32 icon
									,const char* text);
				virtual		~HMsgWindow();
						void SetTime();
						void InsertMessage(const char* text);

protected:
				virtual void MessageReceived(BMessage *msg);
						void InitGUI(const char* text
									,const char* nick
									,uint32 icon);
						
private:
			URLTextView		*textview;
			uint32 			fSock;
			MovieView*		fMovieView;
};
#endif		
					