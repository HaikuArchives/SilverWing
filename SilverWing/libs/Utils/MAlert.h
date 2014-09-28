#ifndef __MALERT_H__
#define __MALERT_H__

#include <Alert.h>
#include "FlashStringView.h"

class MAlert :public BAlert {
public:
						MAlert(const char *flash_text,const char *normal_text,
								const char* btn1 = "OK",
								const char* btn2 = NULL,
								const char* btn3 = NULL,
								alert_type type = B_INFO_ALERT);
						~MAlert();
protected:

};
 
#endif