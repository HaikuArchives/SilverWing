#ifndef __HAccountWINDOW_H__
#define __HAccountWINDOW_H__

#include <Window.h>
#include "HAccountView.h"
#include "HAccountList.h"

enum{
	M_CREATE_ACCOUNT = 'MCAT',
	M_DELETE_ACCOUNT = 'MDEL'
};

class HAccountWindow :public BWindow {
public:
				HAccountWindow(BRect rect,const char* name);
virtual		 	~HAccountWindow();
virtual void	MessageReceived(BMessage *message);
		void	InitGUI();
		void	InitMenu();
		void	RemoveAll();
		void	CreateAccount(const char* name);
		void	LoadAccounts();
protected:
virtual bool	QuitRequested();
		void	AddChangedAccount(const char* name);
		bool	FindChangedAccount(const char* name)const;
private:
	HAccountList*	fListView;
	HAccountView* 	fAccountView;
	BMessage		fChangedAccount;
};
#endif