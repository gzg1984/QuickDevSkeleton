#include "linux/module.h"


static int value = 5;
static char *s = "hello dog";
module_param(value, int, S_IWUSR | S_IRUGO);
module_param(s, charp, S_IWUSR | S_IRUGO);


MODULE_LICENSE("Gzged License");
