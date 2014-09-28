/*--------------------------------------------------------------------*\
  File:      FontMenuItem.h
  Creator:   Matt Bogosian <mattb@be.com>
  Copyright: (c)1997, Matt Bogosian. All rights reserved.
  Description: Header file for FontMenuItem.cpp.
  ID:        $Id$
  Conventions:
      #defines - all uppercase letters with words separated by
          underscores.
          (E.G., #define MY_DEFINE 5).
      New data types (classes, structs, typedefs, etc.) - begin with
          an uppercase letter followed by lowercase words separated by
          uppercase letters.
          (E.G., typedef int MyTypedef;).
      Global constants (declared with const) - begin with "k_"
          followed by lowercase words separated by underscores.
          (E.G., const int k_my_constant = 5;).
      Global variables - begin with "g_" followed by lowercase words
          separated by underscores.
          (E.G., int g_my_global;).
      Local variables - begin with a lowercase letter followed by
          lowercase words separated by underscores.
          (E.G., int my_local;).
      Argument variables - begin with "a_" followed by lowercase words
          separated by underscores.
          (E.G., ...int a_my_argument, ...).
      Member variables - begin with "m_" followed by lowercase words
          separated by underscores.
          (E.G., int m_my_member;).
      Functions - begin with a lowercase letter followed by lowercase
          words separated by uppercase letters.
          (E.G., void myFunction(void);).
      Member Functions - begin with an uppercase letter followed by
          lowercase words separated by uppercase letters.
          (E.G., void MyClass::MyFunction(void);).
\*--------------------------------------------------------------------*/


/*--------------------------------------------------------------------*\
  =-=-=-=-=-=-=-=-=-=-=-=-=- Included Files -=-=-=-=-=-=-=-=-=-=-=-=-=
\*--------------------------------------------------------------------*/

#include <MenuItem.h>


#ifndef FONT_MENU_ITEM_H
#define FONT_MENU_ITEM_H


/*--------------------------------------------------------------------*\
  =-=-=-=-=-=-=-= Definitions, Enums, Typedefs, Consts =-=-=-=-=-=-=-=
\*--------------------------------------------------------------------*/

const uint32 k_font_menu_msg_type = 'FSel';
const char * const k_font_menu_msg_fam = "family";
const char * const k_font_menu_msg_stl = "style";


/*--------------------------------------------------------------------*\
  =-=-=-=-=-=-=-=-=-=-=-=-= Structs, Classes =-=-=-=-=-=-=-=-=-=-=-=-=
\*--------------------------------------------------------------------*/

class FontMenu;

/*--------------------------------------------------------------------*\
  Class name:       FontMenuItem
  Inherits from:    public BMenuItem
  New data members: private char *m_family - the menu item's font
                        family.
                    private char *m_style - the menu item's font
                        style.
                    private bool m_use_font - flag to use the menu
                        item's font family and style as the display
                        font.
  Description: Class to implement a menu item which displays in a
      given font family, style and size.
\*--------------------------------------------------------------------*/

class FontMenuItem : public BMenuItem
{
	public:
	
		// Public member functions
		
/*--------------------------------------------------------------------*\
  Function name: FontMenuItem
  Member of:     public FontMenuItem
  Defined in:    FontMenuItem.cpp
  Arguments:     const font_family &a_family - the menu item's font
                     family.
                 const font_style &a_style - the menu item's font
                     style.
                 const bool a_use_font - flag to indicate that the
                     menu item should display in the font which it
                     represents. Default: true.
  Returns:       n/a
  Description: Class constructor.
\*--------------------------------------------------------------------*/
		
		FontMenuItem(const font_family &a_family, const font_style &a_style, const bool a_use_font = true);
		
/*--------------------------------------------------------------------*\
  Function name: FontMenuItem
  Member of:     public FontMenuItem
  Defined in:    FontMenuItem.cpp
  Arguments:     BMenu *a_menu - .
                 const font_family &a_family - the menu item's font
                     family.
                 const font_style &a_style - the menu item's font
                     style.
                 const bool a_use_font - flag to indicate that the
                     menu item should display in the font which it
                     represents. Default: true.
  Returns:       n/a
  Description: Class constructor.
\*--------------------------------------------------------------------*/
		
		FontMenuItem(BMenu *a_menu, const font_family &a_family, const font_style &a_style, const bool a_use_font = true);
		
/*--------------------------------------------------------------------*\
  Function name: virtual ~FontMenuItem
  Member of:     public FontMenuItem
  Defined in:    FontMenuItem.cpp
  Arguments:     none
  Returns:       n/a
  Description: Class destructor.
\*--------------------------------------------------------------------*/
		
		virtual ~FontMenuItem(void);
		
/*--------------------------------------------------------------------*\
  Function name: virtual DrawContent
  Member of:     public FontMenuItem
  Defined in:    FontMenuItem.cpp
  Arguments:     none
  Returns:       none
  Description: Derived hook function.
\*--------------------------------------------------------------------*/
		
		virtual void DrawContent(void);
		
/*--------------------------------------------------------------------*\
  Function name: FamilyAndStyle
  Member of:     public FontMenuItem
  Defined in:    FontMenuItem.cpp
  Arguments:     font_family &a_family - an array to store a copy of
                     the menu item's font family.
                 font_style &a_style - an array to store a copy of the
                     menu item's font style.
  Returns:       none
  Description: Function to get copies of the menu item's font family
      and style.
\*--------------------------------------------------------------------*/
		
		void FamilyAndStyle(font_family &a_family, font_style &a_style);
		
/*--------------------------------------------------------------------*\
  Function name: virtual GetContentSize
  Member of:     public FontMenuItem
  Defined in:    FontMenuItem.cpp
  Arguments:     float *a_width - the width the menu item should be.
                 float *a_height - the height the menu item should be.
  Returns:       none
  Description: Derived hook function.
\*--------------------------------------------------------------------*/
		
		virtual void GetContentSize(float *a_width, float *a_height);
	
	private:
	
		// Private static data members
		font_family m_family;
		font_style m_style;
		bool m_use_font;
		
		// Private member functions
		
/*--------------------------------------------------------------------*\
  Function name: Init
  Member of:     public FontMenuItem
  Defined in:    FontMenuItem.cpp
  Arguments:     const font_family &a_family - the menu item's font
                     family.
                 const font_style &a_style - the menu item's font
                     style.
  Returns:       none
  Description: Function to initialize the menu item.
\*--------------------------------------------------------------------*/
		
		void Init(const font_family &a_family, const font_style &a_style);
};

#endif    // FONT_MENU_ITEM_H
