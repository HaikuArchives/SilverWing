#ifndef __HACCOUNT_ITEM_H__
#define __HACCOUNT_ITEM_H__

#include <ListItem.h>
#include <String.h>
#include <Bitmap.h>

class HAccountItem :public BListItem {
public:
				HAccountItem(const char* name);
virtual 		~HAccountItem();
virtual	void	DrawItem(BView *owner, BRect frame, bool complete);
virtual void	Update(BView *list ,const BFont *font);
const char*		Text() {return fName.String();}

protected:
	BString fName;
	BBitmap *fBitmap;
};
#endif