// HLDat.h: HLDat クラスのインターフェイス
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_HLDAT_H__B4D2F403_8424_11D4_8205_444553540000__INCLUDED_)
#define AFX_HLDAT_H__B4D2F403_8424_11D4_8205_444553540000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "berw.h"
#include <String.h>
	
typedef struct Icon{
	char *name;
	u_int32_t width, height;
	u_int32_t offset, size;
	u_int32_t numcolors;
	u_int16_t number;
	u_int8_t has_trans;
	u_int8_t trans_index;
	u_int8_t trans[4];
	u_int8_t *cmap;
	u_int8_t *index;
} Icon;

class HLDat  
{
public:
	void Reset();
	void ConvertToRGB(u_int8_t **data,u_int8_t *cmap
					,int width,int height,int numcolors,u_int8_t has_trans,u_int8_t *trans);
	void SetFilePath(const char* path);
	BBitmap* GetBitmap(u_int16_t num);
	Icon* get_icon (u_int16_t num);
	void Init();
	int GetItemCount() const {return numicons;}
	HLDat();
	HLDat(const char* path);
	virtual ~HLDat();
protected:
	void read_icon (Icon *icon);
	void make_icon_numbers();
	
	int read_icon_names ( unsigned int block, unsigned int num_blocks);
private:
	BString m_path;
	unsigned int numicons;
	Icon *icons;
	//Icon **icon_number;
	BFile m_file;
	bool	fInited;
	struct icon_hdr {
	char awpx[4];
	u_int16_t ver;
	u_int16_t pad2;
	u_int32_t pad1;
	u_int32_t one;
	u_int32_t width_real;
	u_int32_t height;
	u_int32_t width_bytes, b, c, d, data_size;
	u_int8_t  r_trans, g_trans, b_trans, a_trans;
	u_int32_t numcolors;
	u_int32_t zero;
	u_int8_t  numbits, f, g, h;
	u_int16_t has_trans, z;
};

};

#endif // !defined(AFX_HLDAT_H__B4D2F403_8424_11D4_8205_444553540000__INCLUDED_)
