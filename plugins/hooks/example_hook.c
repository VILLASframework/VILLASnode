#include <stddef.h>
#include <villas/log.h>
#include <villas/plugin.h>

struct hook;
struct path;
struct sample;

static int hook_example(struct path *p, struct hook *h, int when, struct sample *smps[], size_t cnt)
{
	info("Hello world from example hook!");
	
	return 0;
}

static struct plugin p = {
	.name		= "example",
	.description	= "This is just a simple example hook",
	.type		= LOADABLE_TYPE_HOOK,
	.hook		= {
		.priority = 99,
		.history = 0,
		.cb	= hook_example,
		.type	= HOOK_PATH_START
	}
};

REGISTER_PLUGIN(&p)