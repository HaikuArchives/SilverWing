#include <ClassInfo.h>
#include <CheckBox.h>
#include <stdlib.h>
#include <TextControl.h>

#include "HFireWallView.h"
#include "HApp.h"
#include "HPrefs.h"

/***********************************************************
 * Constructor.
 ***********************************************************/
HFireWallView::HFireWallView(BRect rect)
	:BView(rect,"firewall",B_FOLLOW_ALL,B_WILL_DRAW)
{
	const char* server;
	const char* user;
	const char* pass;
	int32 port;
	((HApp*)be_app)->Prefs()->GetData("firewall",&server);
	((HApp*)be_app)->Prefs()->GetData("firewall_port",&port);
	((HApp*)be_app)->Prefs()->GetData("firewall_user",&user);
	((HApp*)be_app)->Prefs()->GetData("firewall_password",&pass);
	((HApp*)be_app)->Prefs()->GetData("sock5",&fUseSock5);
	((HApp*)be_app)->Prefs()->GetData("auth",&fAuth);
	fServer = server;
	fPort = port;
	fUser = user;
	fPassword = pass;
	InitGUI();
}

/***********************************************************
 * Destructor.
 ***********************************************************/
HFireWallView::~HFireWallView()
{
	
}

/***********************************************************
 * Set up GUIs.
 ***********************************************************/
void
HFireWallView::InitGUI()
{
	this->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	BRect rect = Bounds();
	rect.top += 10;
	rect.left += 10;
	rect.right = rect.left + 200;
	rect.bottom = rect.top + 15;
	BCheckBox *check = new BCheckBox(rect,"sock5",_("Use SOCKS5 firewall"),new BMessage(M_USE_SOCK5));
	check->SetValue(fUseSock5);
	this->AddChild(check);
	rect.OffsetBy(0,20);
	
	BTextControl *control = new BTextControl(rect,"server",_("Server:"),fServer.String(),NULL);
	control->SetDivider(this->StringWidth(_("Username:"))+2);
	if(!fUseSock5)
		control->SetEnabled(false);
	this->AddChild(control);
	rect.OffsetBy(0,20);
	BString str;
	str << fPort;
	control = new BTextControl(rect,"port",_("Port:"),str.String(),NULL);
	control->SetDivider(this->StringWidth(_("Username:"))+2);
	if(!fUseSock5)
		control->SetEnabled(false);
	this->AddChild(control);
	
	rect.OffsetBy(0,30);
	check = new BCheckBox(rect,"auth",_("Authentication Required"),new BMessage(M_AUTH_REQ));
	if(!fUseSock5)
		check->SetEnabled(false);
	this->AddChild(check);
	
	rect.OffsetBy(0,20);
	const float kDivider = StringWidth(_("Username:"))+2;
	control = new BTextControl(rect,"username",_("Username:"),fUser.String(),NULL);
	control->SetDivider( kDivider );
	if(!fAuth)
		control->SetEnabled(false);
	this->AddChild(control);
	rect.OffsetBy(0,20);
	control = new BTextControl(rect,"password",_("Password:"),fPassword.String(),NULL);
	control->SetDivider( kDivider );
	control->TextView()->HideTyping(true);
	if(!fAuth)
		control->SetEnabled(false);
	this->AddChild(control);
}

/***********************************************************
 * MessageReceived.
 ***********************************************************/
void
HFireWallView::MessageReceived(BMessage *message)
{
	switch(message->what)
	{
	case M_AUTH_REQ:
	{
		BCheckBox *checkbox = cast_as(FindView("auth"),BCheckBox);
		fAuth = checkbox->Value();
		BTextControl *control;
		
		control = cast_as(FindView("username"),BTextControl);
		control->SetEnabled(fAuth);
		control = cast_as(FindView("password"),BTextControl);
		control->SetEnabled(fAuth);
		break;
	}
	case M_USE_SOCK5:
	{
		BCheckBox *checkbox = cast_as(FindView("sock5"),BCheckBox);
		fUseSock5 = checkbox->Value();
		BTextControl *control;

		BCheckBox *authcheck = cast_as(FindView("auth"),BCheckBox);
		if(!fUseSock5)
			authcheck->SetValue(fUseSock5);
		authcheck->SetEnabled(fUseSock5);
		Window()->PostMessage(M_AUTH_REQ);
		control = cast_as(FindView("server"),BTextControl);
		control->SetEnabled(fUseSock5);
		control = cast_as(FindView("port"),BTextControl);
		control->SetEnabled(fUseSock5);
		
		break;
	}
	default:
		BView::MessageReceived(message);
	}
}

/***********************************************************
 * Return server address.
 ***********************************************************/
const char*
HFireWallView::Server()
{
	BTextControl *control;
	control = cast_as(FindView("server"),BTextControl);
	return control->Text();
}

/***********************************************************
 * Return user name.
 ***********************************************************/
const char*
HFireWallView::Username()
{
	BTextControl *control;
	control = cast_as(FindView("username"),BTextControl);
	return control->Text();
}

/***********************************************************
 * Return password.
 ***********************************************************/
const char*
HFireWallView::Password()
{
	BTextControl *control;
	control = cast_as(FindView("password"),BTextControl);
	return control->Text();
}

/***********************************************************
 * Return port.
 ***********************************************************/
uint32
HFireWallView::Port()
{
	BTextControl *control;
	control = cast_as(FindView("port"),BTextControl);
	return atol(control->Text());
}