#include "er/ota.h"
#include <stdio.h>
#define CHECK(x) do { if (!(x)) { fprintf(stderr, "FAIL:%d: %s\n", __LINE__, #x); return 1; } } while (0)
static bool verify(const er_ota_manifest_t*m,void*u){(void)u;return m->signature[0]==0x42;}
int main(void){er_ota_t o;er_ota_manifest_t m={.version=2,.image_size=10};m.signature[0]=0x42;er_ota_init(&o,1,verify,0);CHECK(er_ota_begin(&o,1,&m)==ER_OTA_OK);CHECK(er_ota_write(&o,1,0,5)==ER_OTA_OK);CHECK(er_ota_write(&o,1,5,5)==ER_OTA_OK);CHECK(er_ota_finalize(&o,1)==ER_OTA_OK);CHECK(er_ota_select_boot(&o,1)==ER_OTA_OK);CHECK(er_ota_confirm_running(&o,2)==ER_OTA_OK);CHECK(o.active_slot==1);printf("ota tests passed\n");return 0;}
