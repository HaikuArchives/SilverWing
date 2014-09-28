#include "URLTextView.h"
#include "NetPositiveUtils.h"
#include <String.h>
#include <iostream>
#include <ctype.h>
#include <Debug.h>

/***********************************************************
 * Constructor.
 ***********************************************************/
URLTextView::URLTextView(BRect rect,
						const char* name,
						int32 resize,
						int32 flags)
			:CTextView(rect,name,resize,flags)
{
	
}

/***********************************************************
 * Destructor.
 ***********************************************************/
URLTextView::~URLTextView()
{

}

/***********************************************************
 * MouseDown
 ***********************************************************/
void
URLTextView::MouseDown(BPoint point)
{
	int32 line = LineAt(point);
	if(line < 0)
		return;
	int32 mouse_offset = OffsetAt(point);
	int32 offset = OffsetAt(line);
	char *buf = new char[1024];
	GetText(offset,1023,buf);
	BString str = buf;
	delete[] buf;	
	int32 ret = str.FindFirst("\n");
	if(ret != B_ERROR)
		str.Truncate(ret+1,false);
	int32 start = str.FindFirst("http://");
	if(start == B_ERROR)
		start = str.FindFirst("ftp://");
	if(start == B_ERROR)
		start = str.FindFirst("file://");
	if(start != B_ERROR)
	{
		int32 len = str.Length();
		for(register int32 i = start+7;i < len ;i++)
		{
			if( //str.ByteAt(i) == ' ' || str.ByteAt(i) == '\n' 
				/*||*/len == i+1
				|| ( !isalpha(str.ByteAt(i)) 
				&& str.ByteAt(i) != '/' 
				&& str.ByteAt(i) != '.' 
				&& str.ByteAt(i) != ':'
				&& str.ByteAt(i) != '_'
				&& str.ByteAt(i) != '?'
				&& str.ByteAt(i) != '-'
				&& str.ByteAt(i) != '&'
				&& str.ByteAt(i) != '%'
				&& str.ByteAt(i) != '+'
				&& str.ByteAt(i) != '='
				&& str.ByteAt(i) != '~'
				&& !isdigit(str.ByteAt(i)) ))
			{
				BString url = "";
				BPoint offset_point = PointAt(offset+start+7+i);
			
				if( (isdigit(str.ByteAt(i)) || isalpha(str.ByteAt(i)) || str.ByteAt(i) == '/') &&
									 str.ByteAt(i) != '.' && str.ByteAt(i) != ':' )	
					i++;
	
				Highlight(offset + start,offset + i);
				Invalidate();
				str.CopyInto(url,start,i-start);
				// get font height.	
				font_height FontAttributes;
				BFont font;
				uint32 propa;
				GetFontAndColor(&font,&propa);
				font.GetHeight(&FontAttributes);
				float FontHeight = ceil(FontAttributes.ascent) + ceil(FontAttributes.descent);

				if( (offset + start < mouse_offset) &&   
					(mouse_offset < offset + i) &&
					 point.y <= offset_point.y+ FontHeight)
				{
					NetPositiveUtils utils;
					url.ReplaceAll("\n","");
					utils.ShowURL(url.String());
				}else
					CTextView::MouseDown(point);
				break;
			}
		}
	}else
		CTextView::MouseDown(point);
}


/***********************************************************
 * CopyRuns
 ***********************************************************/
void
URLTextView::CopyRuns(const text_run_array *in_runs
							,text_run_array *out_runs)
{
	
}