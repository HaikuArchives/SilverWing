/*************************************************************
*   Toolbar©
*
*   Toolbar is a usefull UI component.
*
*   @author  Atsushi Takamatsu (tak_atsu@tau.bekkoame.ne.jp)
**************************************************************/
#ifndef __HTOOLTIPSVIEW_H__
#define __HTOOLTIPSVIEW_H__

#include <TextView.h>
#include <Window.h>

class HToolTipsView :public BTextView {
public:
					HToolTipsView(BWindow *owner ,BRect rect,const char* name,BRect textrect,uint32 resize,uint32 flag);
protected:
	virtual void 	Pulse();

private:
			bool 	fShown;
			BWindow *fOwnerWindow;
};
#endif	