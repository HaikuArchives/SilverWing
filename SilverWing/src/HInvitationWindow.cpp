#include "HInvitationWindow.h"
#include "HApp.h"

HInvitationWindow::HInvitationWindow(BRect rect,const char* text)
	:BWindow(rect,_("Chat Invitation"),B_FLOATING_WINDOW_LOOK,B_NORMAL_WINDOW_FEEL,B_NOT_RESIZABLE|B_NOT_ZOOMABLE|B_NOT_CLOSABLE)
{
	BView *view = new BView(Bounds(),"bgview",B_FOLLOW_ALL,B_WILL_DRAW);
	view->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
}

HInvitationWindow::~HInvitationWindow()
{
}
