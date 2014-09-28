#ifndef __HCaption_H__
#define __HCaption_H__
#include <String.h>
#include <View.h>
#include <StringView.h>
#include <ListView.h>
#include <iostream>

class HCaption :public BView{
public:
					HCaption(BRect rect,
						const char* name,
						BListView *target=NULL);
	virtual			~HCaption();
			void	SetTime(time_t time);
protected:
	void 			SetCaption(int32 num,time_t time);

	virtual void 	Pulse();
private:
	BStringView 	*view;	
	BListView 		*fTarget;
	int32			fOld;
	time_t			fTime;
};
#endif