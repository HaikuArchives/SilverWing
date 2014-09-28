#include "HListView.h"
#include <Window.h>

/**************************************************************
 * Constructor.
 **************************************************************/
HListView::HListView(BRect frame,
					const char *name,
					list_view_type type,
					uint32 selection_change_msg,
					uint32 resizingMode,
					uint32 flags)
			:BListView(frame,name,type,resizingMode,flags)
{
	fWhat = selection_change_msg;
}

/**************************************************************
 * Destructor.
 **************************************************************/
HListView::~HListView()
{
}

/**************************************************************
 * Selection changed.
 *	Send fWhat message to parent window.
 **************************************************************/
void
HListView::SelectionChanged(void)
{
	Window()->PostMessage(fWhat);
}
