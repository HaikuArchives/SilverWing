#ifndef __HCATEGORYLIST_H__
#define __HCATEGORYLIST_H__

#include <santa/ColumnListView.h>


class HCategoryItem;

enum{
	H_CATEGORY_SEL_CHANGED = 'HCSC'
};

class HCategoryList :public ColumnListView {
public:
				HCategoryList(BRect rect,BetterScrollView **scroll,const char* title);
virtual			~HCategoryList();
		void	AddCategory(const char* name,int16 type,int16 posted,uint32 index);
virtual void 	Expand(CLVListItem* item);
virtual void 	Collapse(CLVListItem* item);
		void	RemoveAll();
		void	RemoveCategoryItem(HCategoryItem *item);
protected:
virtual void 	MouseDown(BPoint point);
virtual void 	SelectionChanged();
virtual void	MessageReceived(BMessage *message);
		int32	RemoveChildItems(CLVListItem *item);
		void	StartBarberPole(const char* label);
private:
		BList	fPointerList;
};
#endif
