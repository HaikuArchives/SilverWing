#ifndef __HBITMAPVIEW_H__
#define __HBITMAPVIEW_H__

#include <View.h>
#include <Bitmap.h>

class HBitmapView :public BView {
public:
					HBitmapView(BRect rect,const char* name,uint32 resizing_mode,BBitmap *bitmap);
	virtual 		~HBitmapView();

protected:
	virtual void	Draw(BRect rect);

private:
		BBitmap *fBitmap;
};
#endif