#include <sys/param.h>
#include <sys/systm.h>
#include <sys/kernel.h>
#include <sys/module.h>
#include <sys/conf.h>
#include <sys/uio.h>
#include <sys/malloc.h>

#define BUFSIZE 255

static struct cdev *hello_dev;

static d_open_t    hello_open;
static d_close_t   hello_close;
static d_read_t    hello_read;
static d_write_t   hello_write;

static struct cdevsw hello_cdevsw = {
  .d_version = D_VERSION,
  .d_open = hello_open,
  .d_close = hello_close,
  .d_read = hello_read,
  .d_write = hello_write,
  .d_name = "hello",
};

typedef struct message {
	char msg[BUFSIZE + 1];
	int len;
} msg_t;

msg_t *msgt;

MALLOC_DECLARE(M_MSGT);
MALLOC_DEFINE(M_MSGT, "struct message", "buffer for hello module");


static int 
hello_open(struct cdev *dev __unused, int oflags __unused, int devtype __unused,
          struct thread *td __unused)
{
  printf("hello: Open\n");
  return (0);
}

static int
hello_close(struct cdev *dev __unused, int fflag __unused, int devtype __unused,
           struct thread *td __unused)
{
  printf("hello: Close\n");
  return (0);
}

static int
hello_read(struct cdev *dev __unused, struct uio *uio, int ioflag __unused)
{
	int error;
	char message[BUFSIZE];
	char msghello[] = "Hello, ";
	size_t length;

	snprintf(message, BUFSIZE, "%s%s", msghello, msgt->msg);
	length = sizeof(msghello) + msgt->len;

	length = MIN(uio->uio_resid,
		uio->uio_offset >= length ? 0 : length - uio->uio_offset);

	error = uiomove(message, length, uio);

	if (error != 0) {
		printf("hello: Read: uiomove failed! (%d)\n", error);
	} else {
		printf("hello: Read: uiomove success, message=\"%s\", length=%d\n",
				message, (int)length);
	}
	return (error);
}

static int
hello_write(struct cdev *dev __unused, struct uio *uio, int ioflag __unused)
{
	printf("hello: Write\n");
	size_t amt;
	int error;
	/*
	 * We either write from the beginning or are appending -- do
	 * not allow random access.
	 */
	if (uio->uio_offset != 0 && (uio->uio_offset != msgt->len))
		return (EINVAL);

	/* This is a new message, reset length */
	if (uio->uio_offset == 0)
		msgt->len = 0;

	/* Copy the string in from user memory to kernel memory */
	amt = MIN(uio->uio_resid, (BUFSIZE - msgt->len));

	error = uiomove(msgt->msg + uio->uio_offset, amt, uio);

	/* Now we need to null terminate and record the length */
	msgt->len = uio->uio_offset;
	msgt->msg[msgt->len] = 0;
	printf("%s", msgt->msg);
	if (error != 0)
		uprintf("Write failed: bad address!\n");
	return (error);
}


static int
hello_load(struct module *mod, int what, void *args){
	switch(what){
	case MOD_LOAD:
		printf("hello load\n");

		struct make_dev_args mda;
		make_dev_args_init(&mda);
		mda.mda_devsw = &hello_cdevsw;
		make_dev_s(&mda, &hello_dev, "hello");

		msgt = malloc(sizeof(msg_t), M_MSGT, M_WAITOK | M_ZERO);

		break;
	case MOD_UNLOAD:
		printf("hello unload\n");

		free(msgt, M_MSGT);
		
		destroy_dev(hello_dev);
		
		break;
	default:
		return EOPNOTSUPP;
	}
	return 0;
}

DEV_MODULE(hello, hello_load, NULL);
