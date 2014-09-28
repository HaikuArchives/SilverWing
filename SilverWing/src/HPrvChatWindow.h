#ifndef __HPrvChatWindow_H__
#define __HPrvChatWindow_H__

#include <Window.h>
#include <string>
#include <iostream>


class SplitPane;
class ChatLogView;
class HUserItem;
class MLTextControl;
class HPrvChatList;

enum{
PRVCHAT_PRVCHAT_CREATE = 'PCCT',
PRVCHAT_PRVCHAT_DELETE = 'PCDL',
PRVCHAT_PRVCHAT_SENDMSG = 'PCSM',
PRVCHAT_PRVCHAT_RSVMSG = 'PCRM',
PRVCHAT_INVOKE_CHAT = 'PCIC',
PRVCHAT_RECEIVE_MSG = 'PCRE',
PRVCHAT_SEND_MESSAGE = 'PRSS',
PRVCHAT_GET_INFO = 'PRGI',
PRVCHAT_CHANGE_TOPIC = 'PCHT',
};

class HPrvChatWindow :public BWindow {
public:

							HPrvChatWindow(BRect rect,const char* name,uint32 pcref);
				virtual		~HPrvChatWindow();
						void InsertChatMessage(const char* text);
					uint32    Pcref() {return fPcref;}
						void AddUserItem(uint16 sock,
										uint16 icon,
										uint16 color,
										const char* nick);
						void ChangeUserItem(uint16 sock ,
										uint16 icon,
										uint16 color,
										const char* nick,
										bool add = true);
						void RemoveUserItem(uint32 sock);
						void ChangeTopic(const char* topic);

protected:
				virtual void MessageReceived(BMessage *msg);
				virtual bool QuitRequested();
						void InitGUI();
						
private:
			ChatLogView*	chatview;
			MLTextControl* 	textview;
			SplitPane 		*fVSplitter;
			SplitPane 		*fHSplitter;
			HPrvChatList	*listview;
			uint32 			fPcref;
			rgb_color		fChatColor;
			rgb_color		fNickColor;
};
#endif		
					