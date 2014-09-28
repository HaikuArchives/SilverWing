#include <Window.h>
#include "HAccountList.h"
#include "HAccountItem.h"

/**************************************************************
 * Constructor.
 **************************************************************/
HAccountList::HAccountList(BRect rect,const char* name)
	:BListView(rect,name,B_SINGLE_SELECTION_LIST)
{

}

/**************************************************************
 * Destructor.
 **************************************************************/
HAccountList::~HAccountList()
{

}

/**************************************************************
 * SelectionChanged.
 **************************************************************/
void
HAccountList::SelectionChanged()
{
	int32 pos = this->CurrentSelection();
	if(pos >= 0)
	{
		HAccountItem *item = (HAccountItem*)ItemAt(pos);
		BMessage msg(M_LIST_SELECTION_CHANGED);
		msg.AddString("text",item->Text());
		Window()->PostMessage(&msg);
	}
}