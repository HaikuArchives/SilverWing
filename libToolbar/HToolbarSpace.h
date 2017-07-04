/*************************************************************
*   Toolbar©
*
*   Toolbar is a usefull UI component.
*
*   @author  Atsushi Takamatsu (tak_atsu@tau.bekkoame.ne.jp)
**************************************************************/
#ifndef __HTOOLBAR_SPACE_H__
#define __HTOOLBAR_SPACE_H__

#include <View.h>
#include <Bitmap.h>

class HToolbarSpace :public BView {
public:
						HToolbarSpace(BRect rect);
						~HToolbarSpace();
protected:
				void	Init();
		virtual void	Draw(BRect rect);
private:
	BBitmap *fBitmap;
};
#endif