#ifndef __ColorView_H__
#define __ColorView_H__

#include <View.h>


class ColorView :public BView{
public:
				ColorView(const char *name,BRect,rgb_color color,BHandler *target);
		virtual	~ColorView();
 		void	SetColor(rgb_color inColor) { color = inColor;}
	rgb_color Color() { return color;}
	
protected:
	 virtual	void MessageReceived(BMessage *msg);
	 virtual 	void MouseDown(BPoint point);
	 virtual	void Draw(BRect);

private:
	rgb_color color;
	BHandler *fTarget;
};
#endif