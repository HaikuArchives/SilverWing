#ifndef __HNAME_SETTING_H__
#define __HNAME_SETTING_H__

#include <View.h>
#include <TextControl.h>
#include <MenuField.h>

class HNameSetting :public BView {
public:
				HNameSetting(BRect rect,const char* name);
virtual 		~HNameSetting();
const char*		Desc();
const char*		Name();
		int32	Encoding();

protected:
	BTextControl *fNameControl;
	BTextControl *fDescControl;
	BMenuField 	 *fEncodeMenu;
};
#endif