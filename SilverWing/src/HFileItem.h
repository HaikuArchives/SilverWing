#ifndef __H_FILE_ITEM_H__
#define __H_FILE_ITEM_H__

#include <String.h>

#include "HTaskWindow.h"
#include "hx_types.h"
#include "CLVEasyItem.h"

class HFileItem :public CLVEasyItem {
public:
					HFileItem(const char* name,
							const char* type,
							const char* creator,
							uint32 size,
							uint32 index,
							uint32 modified,
							bool isSuper = false);
		virtual		~HFileItem();
	static int 		CompareItems(const CLVListItem* a_Item1, const CLVListItem* a_Item2, int32 KeyColumn);
			bool	isFolder() {return fIsFolder;}
	const 	char*	Name() {return fName.String();}
	const 	char*   DecodedName();
			int32	Time() const {return fTime;}
			uint32	Size() const {return fSize;}
	const	BBitmap*	Bitmap();
			uint32  ItemIndex() {return fItemIndex;}
protected:
	virtual	void 	DrawItemColumn(BView* owner, BRect item_column_rect, int32 column_index, bool complete);

private:
			bool 	fIsFolder;
			BString  fName;
			uint32  fSize;
			uint32  fItemIndex;
			int32	fTime;
};
#endif 