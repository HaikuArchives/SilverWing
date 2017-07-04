#ifndef __HTRACKERITEM_H__
#define __HTRACKERITEM_H__

#include <String.h>
#include <iostream>
#include <santa/CLVEasyItem.h>
#include "HTrackerConnection.h"

class HTrackerItem;

class HTrackerItem : public CLVEasyItem {
public:
			 	HTrackerItem(const char* name
			 				,const char* address
			 				,uint16 port
			 				,uint16 users
			 				,const char* desc
			 				,bool isTracker = false
			 				,HTrackerItem* parent= NULL);
	virtual	 	~HTrackerItem();
static int 		CompareItems(const CLVListItem* a_Item1, const CLVListItem* a_Item2, int32 KeyColumn);
const char*		Name() {return GetColumnContentText(2);}
const char*		Description() {return GetColumnContentText(6);}
const char*		Address() {return GetColumnContentText(3);}
const char*		Port() {return GetColumnContentText(4);}
		bool	isTracker() {return fIsTracker;}
		void	Search();
		void	StopSearch();
		void	SetShow(bool shown) {fIsShown = shown;}
		bool	IsShown() const {return fIsShown;}
HTrackerItem*	ParentItem()const {return fParentItem;}
protected:
virtual	void 	DrawItemColumn(BView* owner, BRect item_column_rect, int32 column_index, bool complete);
static	int32	ThreadFunc(void* data);
private:
	bool		fIsTracker;
	bool		fIsShown;
	HTrackerItem *fParentItem;
	HTrackerConnection *fConnection;
};
#endif
