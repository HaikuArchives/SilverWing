#ifndef __HLISTVIEW_H__
#define __HLISTVIEW_H__

#include <ListView.h>

class HListView :public BListView {
public:
					HListView(BRect frame,
							const char *name,
							list_view_type type=B_SINGLE_SELECTION_LIST,
							uint32 selection_change_msg = 0,
							uint32 resizingMode=B_FOLLOW_LEFT | B_FOLLOW_TOP,
							uint32 flags = B_WILL_DRAW | B_NAVIGABLE | B_FRAME_EVENTS);
	virtual			~HListView();
protected:
 	virtual void	SelectionChanged(void);
private:
	uint32	fWhat;
};
#endif