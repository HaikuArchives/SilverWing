#ifndef __LOCKINGLIST_H__
#define __LOCKINGLIST_H__

#include <List.h>
#include <Locker.h>

class LockingList :public BList ,public BLocker {
public:
					LockingList();
	virtual			~LockingList();

};
#endif