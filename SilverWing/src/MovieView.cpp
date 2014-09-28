#include "MovieView.h"

#include <Autolock.h>
#include <Window.h>
#include <Entry.h>
#include <Screen.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


/***********************************************************
 * Constructor
 ***********************************************************/
MovieView::MovieView(BRect rect
					,const char* name
					,const char* path
					,uint32 resize
					,uint32 flags)
		:BView(rect,name,resize,flags)
		,fPath(path)
		,fMediaFile(NULL)
		,fVideoTrack(NULL)
		,fBitmap(NULL)
		,fCurTime(0)
		,fPlaying(false)
		,fLoop(false)
		
{
	SetViewColor(B_TRANSPARENT_32_BIT);
	fBitmapDepth = BScreen().ColorSpace();
	OpenFile(path);
}

/***********************************************************
 * Destructor.
 ***********************************************************/
MovieView::~MovieView()
{
	Stop();
	Reset();
}

/***********************************************************
 * GetPreferedSize
 ***********************************************************/
void
MovieView::GetPreferredSize(float	*width,
							float	*height)
{
	if (fBitmap == NULL) {
 		BView::GetPreferredSize(width, height);
	}
	else {
		BRect bitmapBounds = fBitmap->Bounds();

		*width = bitmapBounds.Width();
		*height = bitmapBounds.Height();
	}
}

/***********************************************************
 * ResizeToPreferred
 ***********************************************************/
void
MovieView::ResizeToPreferred()
{
	float width,height;
	GetPreferredSize(&width,&height);
	ResizeTo(width,height);
}

/***********************************************************
 * FrameResized
 ***********************************************************/
void
MovieView::FrameResized(float	width
						,float	height)
{
	Draw(Bounds());
}
/***********************************************************
 * Play
 ***********************************************************/
status_t
MovieView::Play()
{
	fPlaying = true;
	
	if(fVideoTrack)
	{
		status_t err = B_OK;
		fPlayerThread = spawn_thread(PlayThread, 
									 "MovieView::PlayThread",
									 B_NORMAL_PRIORITY,
									 this);
		if (fPlayerThread < B_NO_ERROR) 
		{
			err = fPlayerThread;
			fPlayerThread = B_ERROR;
			Reset();
			return (err);
		}
		
		err = resume_thread(fPlayerThread);
	
		if (err != B_NO_ERROR) {
			kill_thread(fPlayerThread);
			fPlayerThread = B_ERROR;
			Reset();

			return (err);
		}	
	}
	
	return B_OK;
}

/***********************************************************
 * Play thread
 ***********************************************************/
int32
MovieView::PlayThread(void* data)
{
	MovieView*		view = (MovieView *)data;
	BWindow*		window = view->Window();
	BBitmap*		bitmap = view->fBitmap;
	BMediaTrack*	videoTrack = view->fVideoTrack;
	int64			numFramesToSkip = 0;
	int64			numSkippedFrames = 0;
	
	int64			numFrames = videoTrack->CountFrames();
	int64			dummy;
	media_header	mh;
	
	bool			seekNeeded = true;
	
	bigtime_t		vStartTime/*, aStartTime*/, seekTime, snoozeTime, startTime;
	bigtime_t		/*curScrubbing, lastScrubbing,*/ lastTime;
	
	seekTime = 0LL;
	
	while(videoTrack->CurrentFrame() < numFrames && view->fPlaying)
	{
		startTime = system_time()-videoTrack->CurrentTime();
		// Handle seeking
		if (seekNeeded) 
		{
			if (videoTrack) 
			{
			// Seek the seekTime as close as possible
			vStartTime = seekTime;
			videoTrack->SeekToTime(&vStartTime);
					
			// Read frames until we get less than 50ms ahead.
			lastTime = vStartTime;
			do {
				bitmap->LockBits();
				status_t err = videoTrack->ReadFrames((char*)bitmap->Bits(), &dummy, &mh);
				bitmap->UnlockBits();
				if (err != B_OK) break;
				vStartTime = mh.start_time;
				if ((dummy == 0) || (vStartTime <= lastTime))
					break;
				lastTime = vStartTime;
			} while (seekTime - vStartTime > 50000);
			
			startTime = system_time() - vStartTime;
				
			// Set the current time
			view->fCurTime = seekTime;	
			seekNeeded = false;
			}
		}
		// normal play back
		if (videoTrack != NULL) 
		{
			bitmap->LockBits();
			status_t err = videoTrack->ReadFrames((char*)bitmap->Bits(), &dummy, &mh);
			bitmap->UnlockBits();
			if (err != B_OK) goto do_reset;
			if (dummy == 0)
				goto do_reset;
			vStartTime = mh.start_time;
		}

		// Estimated snoozeTime
		if (videoTrack)
			snoozeTime = vStartTime - (system_time() - startTime);
		else
			snoozeTime = 25000;

		// Handle timing issues
		if (snoozeTime > 5000LL) 
		{
			snooze(snoozeTime-1000);
		}else if (snoozeTime < -5000) {
			numSkippedFrames++;
			numFramesToSkip++;
		}
		// Set the current time
		view->fCurTime = system_time() - startTime;
		if (view->fCurTime < seekTime)
			view->fCurTime = seekTime;		
		
		// Handle the drawing : no drawing if we need to skip a frame...
		if (numSkippedFrames > 0)
			numSkippedFrames--;
		// If we can't lock the window after 50ms, better to give up for
		// that frame...
		if (window->LockWithTimeout(50000) == B_OK) 
		{
			if ((videoTrack != NULL) && !view->fUsingOverlay)
				view->DrawBitmap(bitmap, view->Bounds());
			//view->fMediaBar->SetCurTime(view->fCurTime);
			window->Unlock();
		}
		//printf("CurrentFrame: %Ld (Total: %Ld)\n",videoTrack->CurrentFrame(),videoTrack->CountFrames());
		// for loop
		if (videoTrack->CurrentFrame() >= numFrames && view->fLoop) 
		{
do_reset:
			if(view->fLoop)
			{
				seekNeeded = true;
				seekTime = 0LL;
			}else
				break;
		}
	}	
	
	view->fPlaying = false;
	view->fCurTime = 0LL;
	//printf("End thread\n");
	return B_OK;
}

/***********************************************************
 * Draw
 ***********************************************************/
void
MovieView::Draw(BRect	updateRect)
{
	if ((fBitmap != NULL) && !fUsingOverlay)
		DrawBitmap(fBitmap, Bounds());
}

/***********************************************************
 * Stop
 ***********************************************************/
status_t
MovieView::Stop()
{
	BAutolock lock(Window());
	if(!fPlaying)
		return B_OK;
	fPlaying = false;
	status_t err;
	::wait_for_thread(fPlayerThread,&err);
}

/***********************************************************
 * Reset
 ***********************************************************/
void
MovieView::Reset()
{
	delete fMediaFile;
	delete fBitmap;
	
	fMediaFile = NULL;
	fBitmap = NULL;
	fPlaying= false;
	fCurTime = 0;
}

/***********************************************************
 * Open movie file
 ***********************************************************/
status_t
MovieView::OpenFile(const char* path)
{
	if(strlen(path) == 0)
		return B_ERROR;
	BAutolock autolock(Window());
	fPath.SetTo(path);
	if(fPath.InitCheck() != B_OK)
		return B_ERROR;
	status_t	err = B_ERROR;
	entry_ref	ref;

	err = get_ref_for_path(path, &ref);
	if (err != B_NO_ERROR)
		return (err);

	fMediaFile = new BMediaFile(&ref);
	
	bool	foundTrack = false;
	int32	numTracks = fMediaFile->CountTracks();
	
	for (int32 i = 0; i < numTracks; i++) 
	{
		BMediaTrack *track = fMediaFile->TrackAt(i);
		
		if (track == NULL) 
		{
			Reset();
			return (B_ERROR);
		}else {
			bool			trackUsed = false;
			media_format	mf;

			if (track->EncodedFormat(&mf) == B_NO_ERROR) 
			{			
				switch (mf.type) 
				{
					case B_MEDIA_ENCODED_VIDEO:
printf("#################field rate %f\n", mf.u.encoded_video.output.field_rate);
						trackUsed = SetVideoTrack(path, track, &mf) == B_NO_ERROR;
						break;
					default:
						break;
				}
			}
			if (trackUsed)
				foundTrack = true;
			else {
				fMediaFile->ReleaseTrack(track);
			}
		}
	}
	printf("movie file was opend\n");
	return (B_NO_ERROR);
}

/***********************************************************
 * SetVideoTrack
 ***********************************************************/
status_t
MovieView::SetVideoTrack(const char      *path,
						BMediaTrack		*track,
						media_format	*format)
{
	if (fVideoTrack != NULL)
		// is it possible to have multiple video tracks?
		return (B_ERROR);

	fVideoTrack = track;

	BRect bitmapBounds(0.0, 
					   0.0, 
					   format->u.encoded_video.output.display.line_width - 1.0,
					   format->u.encoded_video.output.display.line_count - 1.0);

	fBitmap = new BBitmap(bitmapBounds,B_BITMAP_WILL_OVERLAY|B_BITMAP_RESERVE_OVERLAY_CHANNEL,B_YCbCr422);
	fUsingOverlay = true;
	if (fBitmap->InitCheck() != B_OK) {
		delete fBitmap;
		fBitmap = new BBitmap(bitmapBounds, fBitmapDepth);
		fUsingOverlay = false;
	};

	/* loop, asking the track for a format we can deal with */
	for(;;) {
		media_format mf, old_mf;

		BuildMediaFormat(fBitmap, &mf);

		old_mf = mf;
		fVideoTrack->DecodedFormat(&mf);
		if (old_mf.u.raw_video.display.format == mf.u.raw_video.display.format) {
			break;
		}

		printf("wanted cspace 0x%x, but it was reset to 0x%x\n",
			   fBitmapDepth, mf.u.raw_video.display.format);
		
		fBitmapDepth = mf.u.raw_video.display.format;
		delete fBitmap;
		fUsingOverlay = false;
		fBitmap = new BBitmap(bitmapBounds, fBitmapDepth);
	}

	media_header mh;
	bigtime_t time = fCurTime;	
	fVideoTrack->SeekToTime(&time);

	int64 dummyNumFrames = 0;
	fVideoTrack->ReadFrames((char *)fBitmap->Bits(), &dummyNumFrames, &mh);

	time = fCurTime;
	fVideoTrack->SeekToTime(&time);	

	if (fUsingOverlay) {
		overlay_restrictions r;
		fBitmap->GetOverlayRestrictions(&r);
		rgb_color key;
		SetViewOverlay(fBitmap,bitmapBounds,Bounds(),&key,B_FOLLOW_ALL,
			B_OVERLAY_FILTER_HORIZONTAL|B_OVERLAY_FILTER_VERTICAL);
		SetViewColor(key);
	};
	return (B_NO_ERROR);
}

/***********************************************************
 * BuildMediaFormat
 ***********************************************************/
void
MovieView::BuildMediaFormat(
	BBitmap			*bitmap,
	media_format	*format)
{
	media_raw_video_format *rvf = &format->u.raw_video;

	memset(format, 0, sizeof(*format));

	BRect bitmapBounds = bitmap->Bounds();

	rvf->last_active = (uint32)(bitmapBounds.Height() - 1.0);
	rvf->orientation = B_VIDEO_TOP_LEFT_RIGHT;
	rvf->pixel_width_aspect = 1;
	rvf->pixel_height_aspect = 3;
	rvf->display.format = bitmap->ColorSpace();
	rvf->display.line_width = (int32)bitmapBounds.Width();
	rvf->display.line_count = (int32)bitmapBounds.Height();
	rvf->display.bytes_per_row = bitmap->BytesPerRow();
}