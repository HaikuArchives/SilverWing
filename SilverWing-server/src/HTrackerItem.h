#ifndef __HTRACKERITEM_H__
#define __HTRACKERITEM_H__

#include <ListItem.h>
#include <String.h>

class HTrackerItem :public BStringItem {
public:
					HTrackerItem(const char* address,
								const char* login,
								const char* password);
		virtual		~HTrackerItem();
		const char* Address() {return fAddress.String();}
		const char* Login() {return fLogin.String();}
		const char* Password() {return fPassword.String();}
		void		SetAddress(const char* address) {fAddress = address;}
		void		SetLogin(const char* login) {fLogin = login;}
		void		SetPassword(const char* password) {fPassword = password;}
protected:
	BString fAddress;
	BString fLogin;
	BString fPassword;
};
#endif