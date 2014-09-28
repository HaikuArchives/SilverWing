#ifndef __HUserList_H__
#define __HUserList_H__

#include <ListView.h>
#include <string>


class HUserItem;

enum{
USERLIST_USER_DRAG_MSG=0x10000000L
};

class HUserList :public BListView
{
public:

					HUserList(BRect frame,const char* title);
	virtual			~HUserList();
			void	AddUserItem(HUserItem* item);
			void	AddUserList(BList *list);
		HUserItem*	DeleteItem(int32 index);			
			void	RemoveAll();
			uint16	FindUserIcon(uint32 sock) const;
protected:
	virtual void	MessageReceived(BMessage *message);
	virtual void	MouseMoved(BPoint point,uint32 transit,const BMessage *message);
	virtual	bool	InitiateDrag (BPoint point, int32 index, bool wasSelected);
	virtual void	MouseDown(BPoint pos);
private:
			BList	fPointerList;
};

#endif