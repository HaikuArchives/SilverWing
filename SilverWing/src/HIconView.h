#ifndef __HICONVIEW_H__
#define __HICONVIEW_H__

#include <View.h>

class HIconView :public BView {
public:
					HIconView(BRect rect,uint32 icon);
		virtual		~HIconView();
		void		SetIcon(uint32 icon);
		uint32		Icon() {return fIcon;}
		void		LoadUserIcon();

protected:
	virtual void	Draw(BRect updateRect);
	
private:
		uint32		fIcon;
		BBitmap 	*fBitmap;
		bool		fBad;
};
#endif