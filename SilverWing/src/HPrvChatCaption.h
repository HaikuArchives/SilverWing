#ifndef __HPrvChatCaption_H__
#define __HPrvChatCaption_H__
#include <String.h>
#include <View.h>
#include <StringView.h>
#include <ListView.h>
#include <iostream>

class HPrvChatCaption :public BView{
public:
					HPrvChatCaption(BRect rect,
						const char* name,
						BListView *target=NULL);
	virtual			~HPrvChatCaption();
			void	SetTime(time_t time);
			void	SetTopic(const char* topic);
	const char*		Topic() {return fTopic.String();}
protected:
			void 	SetCaption(int32 num,time_t time);
	virtual void 	Pulse();
private:
	BStringView 	*view;	
	BListView 		*fTarget;
	int32			fOld;
	time_t			fTime;
	BString			fTopic;
};
#endif