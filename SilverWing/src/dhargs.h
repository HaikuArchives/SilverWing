#ifndef _DHARG_H
#define _DHARG_H 1

static __inline__ void
memory_copy (void *__dst, void *__src, unsigned int len)
{
	uint8 *dst = (uint8*) __dst, *src = (uint8*) __src;

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
	static uint8 _wtfbuf[0xffff + SIZEOF_HL_DATA_HDR];								\
	struct hl_data_hdr *dh;												\
	uint32 _pos, _max;												\
	dh = (struct hl_data_hdr *)_wtfbuf;										\
	memory_copy(dh, (_start), SIZEOF_HL_DATA_HDR);									\
	for (_pos = 0, _max = (_len);											\
	     _pos + SIZEOF_HL_DATA_HDR < _max && (uint16)ntohs(dh->len) <= (_max - _pos) - SIZEOF_HL_DATA_HDR;	\
	     memory_copy(_wtfbuf, (_start) + _pos, SIZEOF_HL_DATA_HDR)) {						\
		memory_copy(dh->data, (_start) + _pos + SIZEOF_HL_DATA_HDR, ntohs(dh->len));				\
		_pos += SIZEOF_HL_DATA_HDR + ntohs(dh->len);

#define dh_start_news(_start, _len)												\
{															\
	static uint8 _wtfbuf[0xffff + SIZEOF_HL_DATA_HDR];								\
	struct hx_news_data_hdr *dh;												\
	uint32 _pos, _max;												\
	dh = (struct hx_news_data_hdr *)_wtfbuf;										\
	memory_copy(dh, (_start), SIZEOF_HL_DATA_HDR);									\
	for (_pos = 0, _max = (_len);											\
	     _pos + SIZEOF_HL_DATA_HDR < _max && (uint16)ntohs(dh->len) <= (_max - _pos) - SIZEOF_HL_DATA_HDR;	\
	     memory_copy(_wtfbuf, (_start) + _pos, SIZEOF_HL_DATA_HDR)) {						\
		memory_copy(dh->data, (_start) + _pos + SIZEOF_HL_DATA_HDR+1, ntohs(dh->len)-1); \
		memory_copy(&(dh->d_type),(_start) + _pos+	SIZEOF_HL_DATA_HDR,1);			\
		_pos += SIZEOF_HL_DATA_HDR + ntohs(dh->len);

		
#define L32NTOH(_word, _addr) \
	do { uint32 _x; memory_copy(&_x, (_addr), 4); _word = ntohl(_x); } while (0)
#define S32HTON(_word, _addr) \
	do { uint32 _x; _x = htonl(_word); memory_copy((_addr), &_x, 4); } while (0)
#define L16NTOH(_word, _addr) \
	do { uint16 _x; memory_copy(&_x, (_addr), 2); _word = ntohs(_x); } while (0)
#define S16HTON(_word, _addr) \
	do { uint16 _x; _x = htons(_word); memory_copy((_addr), &_x, 2); } while (0)
#else
#define dh_start(_start, _len)												\
{															\
	struct hl_data_hdr *dh = (struct hl_data_hdr *)(_start);							\
	uint32 _pos, _max;												\
	for (_pos = 0, _max = (_len);											\
	     _pos + SIZEOF_HL_DATA_HDR < _max && (uint16)ntohs(dh->len) <= (_max - _pos) - SIZEOF_HL_DATA_HDR;	\
	     _pos += SIZEOF_HL_DATA_HDR + ntohs(dh->len),								\
	     dh = (struct hl_data_hdr *)(((uint8 *)dh) + SIZEOF_HL_DATA_HDR + ntohs(dh->len))) {

#define L32NTOH(_word, _addr) \
	_word = ntohl(*((uint32 *)_addr))
#define S32HTON(_word, _addr) \
	*((uint32 *)_addr) = htonl(_word)
#define L16NTOH(_word, _addr) \
	_word = ntohs(*((uint16 *)_addr))
#define S16HTON(_word, _addr) \
	*((uint16 *)_addr) = htons(_word)
#endif

#define dh_getint(_word)						\
do {									\
	if (ntohs(dh->len) >= 4)					\
		_word = (uint32)ntohl(*((uint32 *)dh->data));	\
	else /* if (ntohs(dh->len) == 2) */				\
		_word = (uint32)ntohs(*((uint16 *)dh->data));	\
} while (0)

#define dh_end()	\
	}		\
}

#endif /* !_DHARG_H */
