#define	setcontext(u)	setmcontext(&(u)->uc_mcontext)
#define	getcontext(u)	getmcontext(&(u)->uc_mcontext)

typedef struct mcontext {
	int gregs[16];
} mcontext_t;

typedef struct ucontext {
	/*
	 * Keep the order of the first two fields. Also,
	 * keep them the first two fields in the structure.
	 * This way we can have a union with struct
	 * sigcontext and ucontext_t. This allows us to
	 * support them both at the same time.
	 * note: the union is not defined, though.
	 */
	sigset_t	uc_sigmask;
	mcontext_t	uc_mcontext;

	struct __ucontext *uc_link;
	stack_t		uc_stack;
	int		__spare__[8];
} ucontext_t;

extern	int getmcontext(mcontext_t*);
extern	void setmcontext(const mcontext_t*);
