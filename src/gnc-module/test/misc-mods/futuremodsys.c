/* futuremodsys.c : a gnucash module compiled with a future version of
 * the module system.  gnucash should not be able to load it.  but if
 * it doesn't motice that, the actual interface is compatible with
 * version 0 so it will load all the way before failing. */

#include <stdio.h>
#include <glib.h>

int gnc_module_system_interface = 123456;

int gnc_module_current = 0;
int gnc_module_age = 0;
int gnc_module_revision = 0;

char *
gnc_module_path(void) {
  return g_strdup("gnucash/futuremodsys");
}

char *
gnc_module_description(void) {
  return g_strdup("this is a broken future module");
}

int 
gnc_module_init(int refcount) {
  return TRUE;
}

