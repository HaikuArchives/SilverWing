#ifndef __MSGICONVIEW_H__
#define __MSGICONVIEW_H__

#include <View.h>
#include <String.h>
#include <Bitmap.h>

class MsgIconView :public BView {
public:
				MsgIconView(BRect rect,
							const char* nick,
							BBitmap *bitmap);
	virtual		~MsgIconView();
			void SetIcon(BBitmap *bitmap);	
protected:
	virtual void Draw(BRect updateRect);
private:
	BBitmap *fIcon;
	BString	fNick;
	bool	fBad;
};
#endif