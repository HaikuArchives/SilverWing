#ifndef __H_USER_ITEM_H__
#define __H_USER_ITEM_H__
#include <ListItem.h>
#include <Bitmap.h>
#include <String.h>

class HUserItem :public BListItem {
public:
					HUserItem(uint16 sock,uint16 icon,uint16 color,const char* nick);
	virtual			~HUserItem(); 
			uint16	Sock() const {return fSock;}
			void	ChangeUser(uint16 sock,uint16 icon ,uint16 color,const char* nick);
	const char*		Nick() const; 
		BBitmap*	Bitmap()const {return fBitmap;}
			uint16	Icon()const  {return fIcon;}
			uint16  Color()const {return fColor;}
protected:
	virtual void	Update(BView *list ,const BFont *font);
	virtual void	DrawItem(BView *owner, BRect frame, bool complete = false);
			void	LoadUserBitmap(int16 icon);
private:
			char* fNick;
			BBitmap *fBitmap;
			uint16  fSock;
			uint16  fColor;
			uint16  fIcon;
			bool	fBad;
};
#endif 