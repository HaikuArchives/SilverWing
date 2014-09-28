#ifndef __HSTATUSVIEW_H__
#define __HSTATUSVIEW_H__

#include <StringView.h>
#include <String.h>



class HStatusView:public BStringView {
public:
				HStatusView(BRect rect,const char* name,const char* title);
	virtual 	~HStatusView();
//virtual void	Pulse();
		void	SetNumber(uint32 num);
protected:
		uint32  fNumbers;
		BString	fTitle;
};
#endif