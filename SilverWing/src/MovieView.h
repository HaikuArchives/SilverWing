#ifndef __MOVIEVIEW_H__
#define __MOVIEVIEW_H__

#include <View.h>
#include <MediaFile.h>
#include <MediaTrack.h>
#include <Path.h>
#include <Bitmap.h>

class MovieView :public BView {
public:
						MovieView(BRect rect
								,const char* name
								,const char* path = NULL
								,uint32 resize = B_FOLLOW_TOP|B_FOLLOW_LEFT
								,uint32 flags = B_WILL_DRAW);
		virtual			~MovieView();
		virtual void	GetPreferredSize(float *width, float *height);
				void	ResizeToPreferred();
			status_t	Play();
				bool	IsPlaying() const{return fPlaying;}
			status_t	Stop();
				void	Loop(bool loop) {fLoop = loop;}
				bool	IsLoop()const {return fLoop;}
				
protected:
		virtual void	FrameResized(float	width
									,float	height);
		virtual void	Draw(BRect updateRect);
			status_t	OpenFile(const char* path);
			status_t	SetVideoTrack(const char      *path
									,BMediaTrack		*track
									,media_format	*format);
				void	Reset();
				void	BuildMediaFormat(BBitmap			*bitmap,
										media_format	*format);
private:
		static	int32	PlayThread(void *data);
		color_space		fBitmapDepth;
			thread_id	fPlayerThread;
				BPath	fPath;
			BMediaFile*	fMediaFile;
			BMediaTrack*fVideoTrack;
			BBitmap*	fBitmap;
			bigtime_t	fCurTime;
				bool	fUsingOverlay;
				bool	fPlaying;
				bool	fLoop;
};
#endif