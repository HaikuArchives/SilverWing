#ifndef __APP_UTILS_H__
#define __APP_UTILS_H__

#include <Application.h>
#include <Bitmap.h>
#include <Path.h>
#include <Roster.h>
#include <AppFileInfo.h>

class AppUtils {
public:
					AppUtils();
					~AppUtils();
					
			BPath   GetAppPath(BApplication *app = be_app);
			BPath	GetAppDirPath(BApplication *app = be_app);
		BBitmap*	GetAppBitmap(BApplication *app = be_app);
protected:

};
#endif