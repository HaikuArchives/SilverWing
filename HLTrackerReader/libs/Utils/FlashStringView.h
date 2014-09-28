#ifndef __FlashStringView_H__
#define __FlashStringView_H__

#include <StringView.h>
#include <Message.h>
#include <Window.h>

enum{
M_START_FLASH = 0x020L,
M_STOP_FLASH = 0x021L
};


class FlashStringView :public BStringView
{
public:
					FlashStringView(BRect frame, 
						const char *name, const char *text, 
         					uint32 resizingMode = B_FOLLOW_LEFT | B_FOLLOW_TOP, 
         					uint32 flags = B_WILL_DRAW|B_PULSE_NEEDED);
		virtual		~FlashStringView();
					
		virtual void	MessageReceived(BMessage* msg);
		bool			isFlash() {return fFlash;}
		virtual void	Pulse();
				void	SetFontSize(int size);
				void	SetColor(rgb_color old_color,rgb_color new_color);
protected:
			void		startFlash();
			void		stopFlash();
			bool		old;		
			bool		fFlash;
			rgb_color	fNew;
			BFont 		fFont;
};

#endif