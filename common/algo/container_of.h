#ifndef _CONTAINER_OF_H_
#define _CONTAINER_OF_H_

/**
 * container_of - cast a member of a structure out to the containing structure
 * @ptr:	the pointer to the member.
 * @type:	the type of the container struct this is embedded in.
 * @member:	the name of the member within the struct.
 *
 */
#if 0
// This container_of from kernel cannot support Non POD classes, lots of warnings
#define container_of(ptr, type, member) ({			\
	const typeof(((type *)0)->member) * __mptr = (ptr);	\
	(type *)((char *)__mptr - offsetof(type, member)); })
#endif 

#define container_of(ptr, type, member)( { \
	int iRef=0; \
	type* obj = (type*)&iRef; \
	const typeof(obj->member) * __mptr = (ptr); \
	(type *)( (char *)__mptr - ((char *)&(obj->member) - (char *)obj) ); } )

#endif

