#ifndef __LTHREAD_H__
#define __LTHREAD_H__

#include <KernelKit.h>
#include <Locker.h>

class LThread {
public:
							LThread(const char* name,int32 priority);
				virtual	 	~LThread();
				status_t	Start();
				void		Cancel();
				status_t	WaitForExit(status_t* exitStatus);
				bool		Canceled(){return mCanceled;}
				bool		Lock(void) {return mLocker.Lock();}
				void		Unlock(void) {mLocker.Unlock();}
protected:
		virtual	int32		Main() = 0;
		bool				mCanceled;
private:
		static	int32		EntryPoint(LThread* object);
		thread_id			mID;
		BLocker				mLocker;
};
#endif
				