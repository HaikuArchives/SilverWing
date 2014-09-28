#ifndef __HINFOWINDOW_H__
#define _HINFOWINDOW_H__

#include <Window.h>

class HInfoWindow :public	BWindow {
public:			
							HInfoWindow(BRect rect,const char* name,const char*text);
				virtual		~HInfoWindow();
				
protected:
					void	InitGUI(const char* text);
};
#endif