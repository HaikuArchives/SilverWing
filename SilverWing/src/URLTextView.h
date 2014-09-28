#ifndef __URLTEXTVIEW_H__
#define __URLTEXTVIEW_H__

#include "CTextView.h"

class URLTextView : public CTextView {
public:
					URLTextView(BRect frame,
								const char* name,
								int32 resize,
								int32 flags);
	virtual			~URLTextView();
protected:
	virtual void 	MouseDown(BPoint point);
			void	CopyRuns(const text_run_array *in_runs
							,text_run_array *out_runs);
};
#endif