#ifndef __MLTEXTCONTROL_H__
#define __MLTEXTCONTROL_H__

#include "CTextView.h"
#include <String.h>

class MLTextControl :public CTextView {
public:
				MLTextControl(BRect rect,const char* name,uint32 what,uint32 resize);
	virtual		~MLTextControl();

protected:
virtual void	KeyDown(const char* bytes,int32 numBytes);
virtual void	Draw(BRect updateRect);
virtual bool	AcceptsDrop(const BMessage *message);
virtual void	MessageReceived(BMessage *message);
		void	WhenDropped(const BMessage *message);
const char*	HistoryAt(int32 index);
	const char* FindNick(const char* nick);
private:
	   void	AddHistory(const char* text);
	   void	EmptyHistory();
	uint32 fWhat;
	int32 	fHistoryIndex;
	BList	fHistory;
	bool	fCashed;
	BString fCash;
};
#endif