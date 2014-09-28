#ifndef __CHATLOGVIEW_H__
#define __CHATLOGVIEW_H__

#include "URLTextView.h"

class ChatLogView :public URLTextView {
public:
					ChatLogView(BRect frame,
								const char* name,
								int32 resize,
								int32 flags);
	virtual			~ChatLogView();
			void	SetPrivateChat(uint32 pcref) {fPcref = pcref;}
protected:
	virtual	void	MessageReceived(BMessage *message);
			void	WhenDropped(const BMessage *message);
	virtual void	InsertText(const char* text
								,int32 len
								,int32 offset
								,const text_run_array *runs);

private:
			uint32  fPcref;
};
#endif