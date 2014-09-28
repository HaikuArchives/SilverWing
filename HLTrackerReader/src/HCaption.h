#ifndef __HCaption_H__
#define __HCaption_H__
#include <String.h>
#include <View.h>
#include <StringView.h>
#include <ListView.h>
#include <iostream>

class HCaption :public BView{
public:
					HCaption(BRect rect,
							const char* name,
							const char* cpu,
							BListView *target=NULL);
		virtual		~HCaption();
		void 		SetNumber(int32 num);


protected:
		virtual void Pulse();
private:
		BStringView *view;
		BString		fCpu;	
		BListView *fTarget;
		int32		fOld;
};
#endif