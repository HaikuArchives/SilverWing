#include "HTrackerItem.h"

/**************************************************************
 * Constructor.
 **************************************************************/
HTrackerItem::HTrackerItem(const char* address,
							const char* login,
							const char* password)
			:BStringItem(address)
{
	fAddress = address;
	fLogin = login;
	fPassword = password;
}

/**************************************************************
 * Destructor.
 **************************************************************/
HTrackerItem::~HTrackerItem()
{
}