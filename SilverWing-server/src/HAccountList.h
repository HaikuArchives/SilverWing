#ifndef __HACCOUNT_LIST_H__
#define __HACCOUNT_LIST_H__

#include <ListView.h>

enum{
	M_LIST_SELECTION_CHANGED = 'MLSC'
};

class HAccountList :public BListView {
public:
				HAccountList(BRect rect,const char* name);
	virtual 	~HAccountList();
virtual	void	SelectionChanged();
};
#endif
