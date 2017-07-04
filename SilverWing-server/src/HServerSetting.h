#ifndef __HSERVER_SETTING_H__
#define __HSERVER_SETTING_H__

#include <View.h>
#include <CheckBox.h>
#include <TextControl.h>

class NumberControl;

class HServerSetting :public BView {
public:
					HServerSetting(BRect rect,const char* name);
	virtual 		~HServerSetting();
			void	InitGUI();
	const char*		Address();
	uint32			Port();
	uint32			MaxUser();
	uint32 			SimDownloads();
	uint32			SimUploads();
	bool			SaveLog();
	bool			Sound();
protected:
	BTextControl*	fAddress;
	NumberControl*	fPort;
	NumberControl*	fMaxUser;
	NumberControl*	fSimDownloads;
	NumberControl*	fSimUploads;
	BCheckBox*		fSaveLog;
	BCheckBox*		fThreadedNews;
	BCheckBox*		fSound;
};
#endif
