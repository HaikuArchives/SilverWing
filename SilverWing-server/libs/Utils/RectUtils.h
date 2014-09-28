#ifndef __RECTUTILS_H__
#define __RECTUTILS_H__

#include <Rect.h>

class RectUtils {
public:
			BRect		CenterRect(float width,float height);
			void		SaveRectToApp(const char* name,BRect rect);
			bool		LoadRectFromApp(const char* name,BRect *rect);
};
#endif