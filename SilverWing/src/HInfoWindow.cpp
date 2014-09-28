#include <ScrollView.h>
#include <Message.h>

#include "HInfoWindow.h"
#include "CTextView.h"

/***********************************************************
 * Constructor.
 ***********************************************************/
HInfoWindow::HInfoWindow(BRect rect,const char* name,const char*text)
			:BWindow(rect,name,B_DOCUMENT_WINDOW,0)
{
	InitGUI(text);
}

/***********************************************************
 * Destructor.
 ***********************************************************/
HInfoWindow::~HInfoWindow()
{
}

/***********************************************************
 * Set up GUIs.
 ***********************************************************/
void
HInfoWindow::InitGUI(const char* text)
{
	BRect rect = Bounds();

	BView *bg = new BView(rect,"bg",B_FOLLOW_ALL,0);
	bg->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	rect.right -= B_V_SCROLL_BAR_WIDTH;
	rect.bottom -= B_H_SCROLL_BAR_HEIGHT;	
	BRect textrect = rect;
	textrect.OffsetTo(B_ORIGIN);
	CTextView* infoview = new CTextView(rect,"infoview",B_FOLLOW_ALL,B_WILL_DRAW);
	infoview->MakeEditable(false);
	infoview->SetFontAndColor(be_fixed_font);
	infoview->SetWordWrap(true);
	infoview->SetFontAndColor(be_plain_font);
	BScrollView *scrollView = new BScrollView("scrollview",infoview,B_FOLLOW_ALL,
													B_WILL_DRAW,true,true,B_FANCY_BORDER);
	bg->AddChild(scrollView);
	infoview->SetText(text);
	AddChild(bg);
}