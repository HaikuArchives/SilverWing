#ifndef __TEXTUTILS_H__
#define __TEXTUTILS_H__

#include <UTF8.h>

#define EUC_KR_ENCODING 20
#define SJIS_ENCODING	11
#define EUC_ENCODING	12
#define JIS_ENCODING 	13
#define KOI8R_ENCODING	16

const uint32 coding_translation_table[] = {
	B_ISO1_CONVERSION,				/* ISO 8859-1 */
	B_ISO2_CONVERSION,				/* ISO 8859-2 */
	B_ISO3_CONVERSION,				/* ISO 8859-3 */
	B_ISO4_CONVERSION,				/* ISO 8859-4 */
	B_ISO5_CONVERSION,				/* ISO 8859-5 */
	B_ISO6_CONVERSION,				/* ISO 8859-6 */
	B_ISO7_CONVERSION,				/* ISO 8859-7 */
	B_ISO8_CONVERSION,				/* ISO 8859-8 */
	B_ISO9_CONVERSION,				/* ISO 8859-9 */
	B_ISO10_CONVERSION,				/* ISO 8859-10 */
	B_MAC_ROMAN_CONVERSION,			/* Macintosh Roman */
	B_SJIS_CONVERSION,				/* Shift-JIS */
	B_EUC_CONVERSION,				/* EUC Packed Japanese */
	B_JIS_CONVERSION,				/* JIS X 0208-1990 */
	B_MS_WINDOWS_CONVERSION,		/* MS-Windows Codepage 1252 */
	B_UNICODE_CONVERSION,			/* Unicode 2.0 */
	B_KOI8R_CONVERSION,				/* KOI8-R */
	B_MS_WINDOWS_1251_CONVERSION,	/* MS-Windows Codepage 1251 */
	B_MS_DOS_866_CONVERSION,		/* MS-DOS Codepage 866 */
	B_MS_DOS_CONVERSION,			/* MS-DOS Codepage 437 */
	B_EUC_KR_CONVERSION,			/* EUC Korean */
	B_ISO13_CONVERSION,				/* ISO 8859-13 */
	B_ISO14_CONVERSION,				/* ISO 8859-14 */
	B_ISO15_CONVERSION				/* ISO 8859-15 */
};

const uint32 CODING_TYPES = 24;

enum{
	K_CR = 1,
	K_LF,
	K_CRLF
};

enum {
	NOTSET = 0,
	INPUT,
	OUTPUT,
	REPAIR
};

enum {
	NEW = 1,
	OLD,
	NEC,
	EUC,
	SJIS,
	EUCORSJIS,
	EUCKR,
	UTF8,
	ASCII
};
enum {
	NUL = 0,
	LF = 10,
	CR = 13,
	ESC = 27,
	SS2 = 142
};

class TextUtils
{
	public:
		TextUtils(void);
		
		bool	IsTopOfChar(const char* text){return ((*text & 0xc0) != 0x80);}
		bool	IsASCII(const char* text){return ((*text & 0x80) == 0);}
		bool	IsWordChar(const char* text);
		
		void	ToLowerCase(char* text, bool only1byte = false);
		void	ToLowerCase(char* text, int32 len);
		void	ToUpperCase(char* text, bool only1byte = false);
		void	ToUpperCase(char* text, int32 len);
		
		int32 CountChars(const char* text);
		int32 CountChars(const char* text, int32 len);
		
		int32 GetCodeLength(const char* text);
		
		//Return Code
		void	ConvertReturnsToLF(char* text);
		void	ConvertReturnsToLF(BString &text);
		
		void	ConvertReturnsToCR(char* text);
		void	ConvertReturnsToCR(BString &text);
		
		int32	DetectReturnCode(const char* text);
		
		void	ConvertReturnsToCRLF(char* text); // You have to prepare enough buffer
		void	ConvertReturnsToCRLF(char** text);
		void	ConvertReturnsToCRLF(BString &text);
		
		void	ConvertReturnCode(BString &text,int32 retcode);
		
		//Character Code
		int32	DetectCodeType(const char* text);
		
		int32 	DetectKoreanType(const char* text);
		
		void	JIS2EUC(char** text);
		void	EUC2JIS(char** text);
		
		void 	JIS2UTF8(char** text);
		void	UTF82JIS(char** text);
		
		void	SJIS2UTF8(char** text);
		void	UTF82SJIS(char** text);
		
		void	EUC2UTF8(char** text);
		void	UTF82EUC(char** text);
		// convert encoding with index
		void	ConvertToUTF8(char** text,int32 encoding);
		void	ConvertFromUTF8(char** text,int32 encoding);
		void	ConvertToUTF8(BString &text,int32 encoding);
		void	ConvertFromUTF8(BString &text,int32 encoding);
		
		bool	SkipESCSeq(const char* text, uint8 temp, bool *in2byte);
		
		//path tool
		char*	GetRelPath(const char* source, const char* dest);
		
		//"C" string tools
		void	Decode_C_String(char* text);
		
		//tools
		void	PrintCharCode(char* text);
		char*	DecodeTypeCode(uint32 code);
		
		void	Mime2UTF8(BString &str);
		void	UTF82Mime(BString &str,int32 encoding);

	private:
		void	p_MimeDecode(BString &str);
		char	p_Charconv(char c);
	status_t	p_ConvertToUTF8(uint32 coding,char** text);
	status_t	p_ConvertFromUTF8(uint32 coding,char** text);
	status_t	p_ConvertToUTF8(uint32 coding,BString& text);
	status_t	p_ConvertFromUTF8(uint32 coding,BString& text);
		char	p_Convtobase(char c);
		bool	p_IsASCII(char c);
		void	ToMime(BString &in, int32 encoding);
};
#endif