#ifndef __HFONTSETTING_H__
#define __HFONTSETTING_H__

#include <View.h>

enum{
	M_FONT_CHANGED = 'MFOT',
	M_FONT_SIZE = 'MFOS'
};

class HFontSettingView :public BView {
public:
				HFontSettingView(BRect rect);
		virtual	~HFontSettingView();
const char*		FontFamily();
const char*		FontStyle(); 
		int32	FontSize();
		uint32	FontColor();
		uint32	BackColor();
		uint32	Encoding();
		uint32  NickColor();
		uint32  URLColor();

protected:
		void	InitGUI();
virtual void	MessageReceived(BMessage *message);

private:
		font_family fFamily;
		font_style	fStyle;
		
};
#endif