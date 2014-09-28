#ifndef __HFILECAPTION_H__
#define __HFILECAPTION_H__

#include <View.h>
#include <StringView.h>
#include <ListView.h>
#include <Bitmap.h>
#include <String.h>

class HFileCaption :public BView {
public:
					HFileCaption(BRect rect,const char* name = "caption",BListView *target=NULL);
	virtual			~HFileCaption();
			void	StartBarberPole();
			void	StopBarberPole();
			void	SetLabel(const char* text);
protected:
			void	SetNumber(int32 num);
	virtual void	Pulse();	
	virtual void	Draw(BRect updateRect);	
			BRect	BarberPoleInnerRect() const;	
			BRect	BarberPoleOuterRect() const;
private:
	BStringView 	*view;	
	BListView 		*fTarget;
	int32			fOld;
	int32			fLastBarberPoleOffset;
	bool 			fShowingBarberPole;
	BBitmap			*fBarberPoleBits;
};
#endif
				