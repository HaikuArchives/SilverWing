#ifndef __HARTICLEVIEW_H__
#define __HARTICLEVIEW_H__

#include <View.h>

class HArticleView :public BView {
public:
					HArticleView(BRect rect,const char* name);
		virtual 	~HArticleView();
		void		InitGUI();
		void		SetSender(const char* sender);
		void		SetArticle(const char* text);
		void		SetDate(const char* date);
		void		SetSubject(const char* subject);
		void		ResetAll();
protected:
virtual void		MessageReceived(BMessage* message);	
};
#endif