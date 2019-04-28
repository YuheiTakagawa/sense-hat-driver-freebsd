#include <sys/param.h>
#include <sys/systm.h>
#include <sys/kernel.h>
#include <sys/module.h>
#include <sys/conf.h>


static int
hello_load(struct module *mod, int what, void *args){
	switch(what){
	case MOD_LOAD:
		printf("hello load\n");
		break;
	case MOD_UNLOAD:
		printf("hello unload\n");
		break;
	default:
		return EOPNOTSUPP;
	}
	return 0;
}

/*
static moduledata_t hello_mod = {
	"hello",
	hello_load,
	NULL
};
*/
//DECLARE_MODULE(hello, hello_mod, SI_SUB_KLD, SI_ORDER_ANY);
DEV_MODULE(hello, hello_load, NULL);
