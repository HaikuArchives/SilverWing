#ifndef __INVITATION_WINDOW_H__
#define __INVITATION_WINDOW_H__

#include <Window.h>
#include <Looper.h>

enum{
	M_JOIN_CHAT = 'MJOI',
	M_REFUSE_CHAT = 'MREF'
};

class InvitationWindow : public BWindow {
public:
				InvitationWindow(BRect rect,
							const char* name,
							const char* nick,
							uint32 pcref,
							BLooper *target);
	virtual		~InvitationWindow();

protected:
	virtual void	MessageReceived(BMessage *message);
		   void	InitGUI();
private:
	uint32 	fPcref;
	BLooper	*fTarget;
};
#endif