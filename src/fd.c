#include "../src/taskimpl.h"

#if defined(_WIN32) || defined(_WIN64)
#include <Winsock2.h>
#include "compat/fcntl.h"
#pragma comment(lib, "ws2_32.lib")

int gettimeofday(struct timeval *tp, struct timezone *tzp)
{
    // Note: some broken versions only have 8 trailing zero's, the correct epoch has 9 trailing zero's
    // This magic number is the number of 100 nanosecond intervals since January 1, 1601 (UTC)
    // until 00:00:00 January 1, 1970
    static const uint64_t EPOCH = ((uint64_t)116444736000000000ULL);

    SYSTEMTIME system_time;
    FILETIME file_time;
    uint64_t time;

    GetSystemTime(&system_time);
    SystemTimeToFileTime(&system_time, &file_time);
    time = ((uint64_t)file_time.dwLowDateTime);
    time += ((uint64_t)file_time.dwHighDateTime) << 32;

    tp->tv_sec = (long)((time - EPOCH) / 10000000L);
    tp->tv_usec = (long)(system_time.wMilliseconds * 1000);
    return 0;
}
#else
#include <sys/poll.h>
#include <fcntl.h>
#endif


enum
{
	MAXFD = 1024
};

static struct pollfd pollfd[MAXFD];
static Task *polltask[MAXFD];
static int npollfd;
static int startedfdtask;
static Tasklist sleeping;
static int sleepingcounted;
static uvlong nsec(void);

void fdtask(void *v)
{
	int i, ms;
	Task *t;
	uvlong now;

	tasksystem();
	taskname("fdtask");
	for (;;)
	{
		/* let everyone else run */
		while (taskyield() > 0)
			;
		/* we're the only one runnable - poll for i/o */
		errno = 0;
		taskstate("poll");
		if ((t = sleeping.head) == nil)
			ms = -1;
		else
		{
			/* sleep at most 5s */
			now = nsec();
			if (now >= t->alarmtime)
				ms = 0;
			else if (now + 5 * 1000 * 1000 * 1000LL >= t->alarmtime)
				ms = (t->alarmtime - now) / 1000000;
			else
				ms = 5000;
		}

#if defined(_WIN32) || defined(_WIN64)
        if (WSAPoll(pollfd, npollfd, ms) < 0)
#else
        if (poll(pollfd, npollfd, ms) < 0)
#endif

		{
			if (errno == EINTR)
				continue;
			fprint(2, "poll: %s\n", strerror(errno));
			taskexitall(0);
		}

		/* wake up the guys who deserve it */
		for (i = 0; i < npollfd; i++)
		{
			while (i < npollfd && pollfd[i].revents)
			{
				taskready(polltask[i]);
				--npollfd;
				pollfd[i] = pollfd[npollfd];
				polltask[i] = polltask[npollfd];
			}
		}
		now = nsec();
		while ((t = sleeping.head) && now >= t->alarmtime)
		{
			deltask(&sleeping, t);
			if (!t->system && --sleepingcounted == 0)
				taskcount--;
			taskready(t);
		}
	}
}

uint taskdelay(uint ms)
{
	uvlong when, now;
	Task *t;

	if (!startedfdtask)
	{
		startedfdtask = 1;
		taskcreate(fdtask, 0, 32768);
	}

	now = nsec();
	when = now + (uvlong)ms * 1000000;
	for (t = sleeping.head; t != nil && t->alarmtime < when; t = t->next)
		;

	if (t)
	{
		taskrunning->prev = t->prev;
		taskrunning->next = t;
	}
	else
	{
		taskrunning->prev = sleeping.tail;
		taskrunning->next = nil;
	}

	t = taskrunning;
	t->alarmtime = when;
	if (t->prev)
		t->prev->next = t;
	else
		sleeping.head = t;
	if (t->next)
		t->next->prev = t;
	else
		sleeping.tail = t;

	if (!t->system && sleepingcounted++ == 0)
		taskcount++;
	taskswitch();

	return (nsec() - now) / 1000000;
}

void fdwait(int fd, int rw)
{
	int bits;

	if (!startedfdtask)
	{
		startedfdtask = 1;
		taskcreate(fdtask, 0, 32768);
	}

	if (npollfd >= MAXFD)
	{
		fprint(2, "too many poll file descriptors\n");
		abort();
	}

	taskstate("fdwait for %s", rw == 'r' ? "read" : rw == 'w' ? "write"
															  : "error");
	bits = 0;
	switch (rw)
	{
	case 'r':
		bits |= POLLIN;
		break;
	case 'w':
		bits |= POLLOUT;
		break;
	}

	polltask[npollfd] = taskrunning;
	pollfd[npollfd].fd = fd;
	pollfd[npollfd].events = bits;
	pollfd[npollfd].revents = 0;
	npollfd++;
	taskswitch();
}

/* Like fdread but always calls fdwait before reading. */
int fdread1(int fd, void *buf, int n)
{
	int m;

	do
		fdwait(fd, 'r');
	while ((m = read(fd, buf, n)) < 0 && errno == EAGAIN);
	return m;
}

int fdread(int fd, void *buf, int n)
{
	int m;

	while ((m = read(fd, buf, n)) < 0 && errno == EAGAIN)
		fdwait(fd, 'r');
	return m;
}

int fdwrite(int fd, void *buf, int n)
{
	int m, tot;

	for (tot = 0; tot < n; tot += m)
	{
		while ((m = write(fd, (char *)buf + tot, n - tot)) < 0 && errno == EAGAIN)
			fdwait(fd, 'w');
		if (m < 0)
			return m;
		if (m == 0)
			break;
	}
	return tot;
}

// F_GETFL (void)
// 		Return (as the function result) the file access mode and
// 		the file status flags; arg is ignored.
//
// F_SETFL (int)
// 		Set the file status flags to the value specified by arg.
// 		File access mode (O_RDONLY, O_WRONLY, O_RDWR) and file
// 		creation flags (i.e., O_CREAT, O_EXCL, O_NOCTTY, O_TRUNC)
// 		in arg are ignored.  On Linux, this command can change
// 		only the O_APPEND, O_ASYNC, O_DIRECT, O_NOATIME, and
// 		O_NONBLOCK flags.  It is not possible to change the
// 		O_DSYNC and O_SYNC flags; see BUGS, below.
int fdnoblock(int fd)
{

#if defined(_WIN32) || defined(_WIN64)
    return fd;
#else
    return fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK);
#endif
}

static uvlong
nsec(void)
{
	struct timeval tv;

	if (gettimeofday(&tv, 0) < 0)
		return -1;
	return (uvlong)tv.tv_sec * 1000 * 1000 * 1000 + tv.tv_usec * 1000;
}
