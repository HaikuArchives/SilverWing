#ifndef _XMALLOC_H
#define _XMALLOC_H 1

#ifdef __cplusplus
extern "C" {
#endif

extern void *xmalloc (size_t size);
extern void *xrealloc (void *ptr, size_t size);
extern void xfree (void *ptr);
extern char *xstrdup (const char *str);

#ifdef __cplusplus
}
#endif
#endif /* !_XMALLOC_H */
