#ifndef __HLISTVIEW_H__
#define __HLISTVIEW_H__

#include <santa/ColumnListView.h>
#include "LockingList.h"
#include <String.h>

class HTrackerItem;
class LockingList;
class WatcherLooper;

enum{
M_CONNECT_NEW_WINDOW = 'MCNW'
};

class HListView :public ColumnListView {
public:
					HListView(BRect frame,CLVContainerView** ContainerView,const char* name);
		virtual		~HListView();
			void	RemoveAll();
			void	AddServerUnder(HTrackerItem *newItem,HTrackerItem *parent);
			void	AddServer(HTrackerItem* item);
			void	Add(HTrackerItem *item);
			void	RemoveServer(HTrackerItem* item);
	virtual void 	Expand(CLVListItem* item);
	virtual void 	Collapse(CLVListItem* item);
	virtual void	Pulse();
			void	SetKeyword(const char* keyword) {fKeyword = keyword;}
	const char*		Keyword() {return fKeyword.String();}
			void	RemoveAllChildren();
			int32	RemoveChildren(HTrackerItem* parent);
			void	StartWatching(bool start);
			void	EmptyQueue();
			void	AddQueue(HTrackerItem *item);
protected:
	virtual bool	InitiateDrag (BPoint point, int32 index, bool wasSelected);
	virtual void	MouseDown(BPoint pos);

private:
	static int32	Watcher(void *data);
		BList			fItemList;
		LockingList		fQueueList;
		LockingList 	fAllList;
//		WatcherLooper 	*fWatcher;
		bool			fKeepWatching;
		BString			fKeyword;
		thread_id		fWatcher;
};
#endif
