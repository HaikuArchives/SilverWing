#ifndef __HPrvChatList_H__
#define __HPrvChatList_H__

#include <ListView.h>
#include <string>

enum{
PRVLIST_CHAR_INVITE= 'PLCI'
};

class HPrvChatList :public BListView
{
public:

					HPrvChatList(BRect frame,const char* title);
		virtual		~HPrvChatList();
protected:
	virtual void	MessageReceived(BMessage *message);
	virtual	bool	InitiateDrag (BPoint point, int32 index, bool wasSelected);
			void	WhenDropped(BMessage *msg);
	virtual void	MouseMoved(BPoint point,uint32 transit,const BMessage *message);
	virtual void	MouseDown(BPoint pos);
};

#endif