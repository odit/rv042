#ifndef _XT_MARK_H_target
#define _XT_MARK_H_target

/* Version 0 */
struct xt_mark_target_info {
	u_int32_t mark; /* Changed from UL 6/5/2007 based on email from Aziz */
};

/* Version 1 */
enum {
	XT_MARK_SET=0,
	XT_MARK_AND,
	XT_MARK_OR,
};

struct xt_mark_target_info_v1 {
	u_int32_t mark; /* Changed from UL 6/5/2007 based on email from Aziz */
	u_int8_t mode;
};

#endif /*_XT_MARK_H_target */
