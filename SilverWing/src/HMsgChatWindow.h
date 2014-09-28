#ifndef __MSGCHATWINDOW_H__
#define __MSGCHATWINDOW_H__

#include <Window.h>
#include <String.h>

class MLTextControl;
class ChatLogView;
class SplitPane; 

enum{
	M_CLOSE_MSG_CHAT_WINDOW = 'MCLM',
	M_LOGOUT_MESSAGE = 'MLOG'
};

class HMsgChatWindow :public BWindow {
public:
						HMsgChatWindow(BRect rect
									,const char* name
									,uint32 sock
									,uint32 icon);
		virtual			~HMsgChatWindow();
				int32	Sock() {return fSock;}
				void	InsertChatMessage(const char* text,bool byme = false);
protected:
		virtual bool	QuitRequested();
		virtual void	MessageReceived(BMessage *message);
				void	InitGUI();		
				
private:
		MLTextControl	*fChatEntry;
		ChatLogView 	*fChatView;
		SplitPane		*fHSplitter;
		int32			fSock;
		int32			fIcon;
		BString			fNick;
		rgb_color		fNickColor;
		rgb_color		fChatColor;

};
#endif