#ifndef __HARTICLELIST_H__
#define __HARTICLELIST_H__

#include "ColumnListView.h"
#include <String.h>

class HArticleItem;

enum{
ARTICLE_REQUESTED = 'ARRQ'
};

class HArticleList :public ColumnListView {
public:
				HArticleList(BRect rect,BetterScrollView **scroll,const char* title);
	virtual		~HArticleList();
		void	AddArticle(const char* subject,const char*sender,uint32 time,uint32 parent_id,uint16 index);
		void	RemoveAll();
		void	SetCategory(const char* category) {fCategory = category;}
const char*		Category() {return fCategory.String();}

protected:
virtual void	MessageReceived(BMessage *message);
virtual void 	SelectionChanged();
virtual void 	MouseDown(BPoint point);
		void	StartBarberPole();
private:	
		BString fCategory;
		BList	fPointerList;
};
#endif