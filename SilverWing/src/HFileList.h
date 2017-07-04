#ifndef __HFILELIST_H__
#define __HFILELIST_H__

#include <santa/ColumnListView.h>
#include <String.h>

class HFileItem;

class HFileList : public ColumnListView {
public:
				HFileList(BRect rect
						,BetterScrollView **scroll
						,const char* title
						,bool IsSilverWing);
	virtual		~HFileList();
		void	RemoveAll();
		void	AddFileItem(HFileItem* item);
		void	AddFileItemUnder(HFileItem *item,HFileItem *parent);
		void	RemoveFileItem(HFileItem *item);
	HFileItem*	FindItem(uint32 index);
	HFileItem*	FindItem(const BPoint point);
		void	GetItemPath(HFileItem *item,BString &path);
		bool	FindSameItem(const char* name,HFileItem *parent);
		bool	FindSameItem(const char* name);
		int32	RemoveChildItems(CLVListItem* item);

virtual void 	Expand(CLVListItem* item);
virtual void 	Collapse(CLVListItem* item);

protected:
virtual	bool	InitiateDrag (BPoint point, int32 index, bool wasSelected);
virtual void	MessageReceived(BMessage *message);
virtual void	MouseDown(BPoint pos);
virtual void	MouseMoved(BPoint point,uint32 transit,const BMessage *message);

private:
		BList	fPointerList;
		int32 	fDraggedItemIndex;
		typedef ColumnListView _inherited;
};
#endif
