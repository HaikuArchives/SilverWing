#ifndef __HARTICLEITEM_H__
#define __HARTICLEITEM_H__

#include <santa/CLVEasyItem.h>
#include <String.h>
#include <iostream>

class HArticleItem :public CLVEasyItem {
public:
				HArticleItem(const char* subject,const char* sender,
						uint32 time,uint32 parent_id,uint16 index);
	virtual 	~HArticleItem();
		uint32	ParentID() {return fParent_id;}
		uint16 	Index() {return fIndex;}
		uint32 	Time() {return fTime;}
const char*		Subject() {return fSubject.String();}
static	int		CompareItems(const CLVListItem *a_Item1,
							 const CLVListItem *a_Item2,
							 int32 KeyColumn);
protected:
	virtual	void 	DrawItemColumn(BView* owner, BRect item_column_rect, int32 column_index, bool complete);
private:
	BString fSubject;
	uint32 fParent_id;
	uint16 fIndex;
	uint32 fTime;

};
#endif
