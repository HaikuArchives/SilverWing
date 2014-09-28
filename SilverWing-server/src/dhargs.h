/**************************************************************
 * This file is from hxd.
 **************************************************************/
#ifndef _DHARG_H
#define _DHARG_H 1

static __inline__ void
memory_copy (void *__dst, void *__src, unsigned int len)
{
	u_int8_t *dst = (uint8*) __dst, *src = (uint8*) __src;

	for (; len; len--)
		*dst++ = *src++;
}

#define SIZEOF_HL_HDR SIZEOF_HX_HDR
#define SIZEOF_HL_DATA_HDR SIZEOF_HX_DATA_HDR
#define hl_data_hdr hx_data_hdr

#if 1
/* data must be accessed from locations that are aligned on multiples of the data size */
#define dh_start(_start, _len)												\
{															\
	static u_int8_t _wtfbuf[0xffff + SIZEOF_HL_DATA_HDR];								\
	struct hl_data_hdr *dh;												\
	u_int32_t _pos, _max;												\
	dh = (struct hl_data_hdr *)_wtfbuf;										\
	memory_copy(dh, (_start), SIZEOF_HL_DATA_HDR);									\
	for (_pos = 0, _max = (_len);											\
	     _pos + SIZEOF_HL_DATA_HDR < _max && (u_int16_t)ntohs(dh->len) <= (_max - _pos) - SIZEOF_HL_DATA_HDR;	\
	     memory_copy(_wtfbuf, (_start) + _pos, SIZEOF_HL_DATA_HDR)) {						\
		memory_copy(dh->data, (_start) + _pos + SIZEOF_HL_DATA_HDR, ntohs(dh->len));				\
		_pos += SIZEOF_HL_DATA_HDR + ntohs(dh->len);

#define dh_start_news(_start, _len)												\
{															\
	static u_int8_t _wtfbuf[0xffff + SIZEOF_HL_DATA_HDR];								\
	struct hx_news_data_hdr *dh;												\
	u_int32_t _pos, _max;												\
	dh = (struct hx_news_data_hdr *)_wtfbuf;										\
	memory_copy(dh, (_start), SIZEOF_HL_DATA_HDR);									\
	for (_pos = 0, _max = (_len);											\
	     _pos + SIZEOF_HL_DATA_HDR < _max && (u_int16_t)ntohs(dh->len) <= (_max - _pos) - SIZEOF_HL_DATA_HDR;	\
	     memory_copy(_wtfbuf, (_start) + _pos, SIZEOF_HL_DATA_HDR)) {						\
		memory_copy(dh->data, (_start) + _pos + SIZEOF_HL_DATA_HDR+1, ntohs(dh->len)-1); \
		memory_copy(&(dh->d_type),(_start) + _pos+	SIZEOF_HL_DATA_HDR,1);			\
		_pos += SIZEOF_HL_DATA_HDR + ntohs(dh->len);

		
#define L32NTOH(_word, _addr) \
	do { u_int32_t _x; memory_copy(&_x, (_addr), 4); _word = ntohl(_x); } while (0)
#define S32HTON(_word, _addr) \
	do { u_int32_t _x; _x = htonl(_word); memory_copy((_addr), &_x, 4); } while (0)
#define L16NTOH(_word, _addr) \
	do { u_int16_t _x; memory_copy(&_x, (_addr), 2); _word = ntohs(_x); } while (0)
#define S16HTON(_word, _addr) \
	do { u_int16_t _x; _x = htons(_word); memory_copy((_addr), &_x, 2); } while (0)
#else
#define dh_start(_start, _len)												\
{															\
	struct hl_data_hdr *dh = (struct hl_data_hdr *)(_start);							\
	u_int32_t _pos, _max;												\
	for (_pos = 0, _max = (_len);											\
	     _pos + SIZEOF_HL_DATA_HDR < _max && (u_int16_t)ntohs(dh->len) <= (_max - _pos) - SIZEOF_HL_DATA_HDR;	\
	     _pos += SIZEOF_HL_DATA_HDR + ntohs(dh->len),								\
	     dh = (struct hl_data_hdr *)(((u_int8_t *)dh) + SIZEOF_HL_DATA_HDR + ntohs(dh->len))) {

#define L32NTOH(_word, _addr) \
	_word = ntohl(*((u_int32_t *)_addr))
#define S32HTON(_word, _addr) \
	*((u_int32_t *)_addr) = htonl(_word)
#define L16NTOH(_word, _addr) \
	_word = ntohs(*((u_int16_t *)_addr))
#define S16HTON(_word, _addr) \
	*((u_int16_t *)_addr) = htons(_word)
#endif

#define dh_getint(_word)						\
do {									\
	if (ntohs(dh->len) >= 4)					\
		_word = (u_int32_t)ntohl(*((u_int32_t *)dh->data));	\
	else /* if (ntohs(dh->len) == 2) */				\
		_word = (u_int32_t)ntohs(*((u_int16_t *)dh->data));	\
} while (0)

#define dh_end()	\
	}		\
}

#endif /* !_DHARG_H */
