#ifndef __HAccountVIEW_H__
#define __HAccountVIEW_H__

#include <View.h>
#include <String.h>
#include <TextControl.h>


enum{
	M_DIRTY_MSG = 'MDRT'
};

class HAccountView :public BView {
public:
				HAccountView(BRect rect,const char* name);
virtual 		~HAccountView();
		bool	IsDirty()const {return fDirty;}
		void	SetAccount(const char* name);
		void	SetValue(const char* name,bool which);
		bool	GetValue(const char* name);			
		void	SaveAccount();
const char*		Account() const {return fAccount.String();}
virtual void	MessageReceived(BMessage *message);
protected:
		bool	fDirty;
BTextControl* 	fLogin;
BTextControl*	fPassword;
BString			fAccount;
};
#endif
