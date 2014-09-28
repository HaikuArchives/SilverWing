#ifndef __HCATEGORYITEM_H__
#define __HCATEGORYITEM_H__

#include "CLVEasyItem.h"
#include <String.h>
#include <iostream>

class HCategoryItem :public CLVEasyItem {
public:
				HCategoryItem(const char* title,uint16 type,uint16 posted,uint32 index,bool superitem = false);
	virtual 	~HCategoryItem();
		uint32	Index() {return fIndex;}
		void	SetPath(const char* text) {fPath = text;}
const char*		Path() {return fPath.String();}
		uint32	Type() {return fType;}
protected:
	virtual	void 	DrawItemColumn(BView* owner, BRect item_column_rect, int32 column_index, bool complete);
private:
	BString fTitle;
	uint16 fType;
	uint16 fPosted;
	uint32 fIndex;
	BString fPath;
};
#endif