#include "bridge.h"
#include <stdio.h>
#include <string.h>
#define CHECK(x) do { if(!(x)){fprintf(stderr,"failed: %s line %d\n",#x,__LINE__);return 1;} } while(0)
int main(void){
    er_web_reset();
    CHECK(er_web_command(1,-1,1)==0);
    for(int i=0;i<100;i++){
        CHECK(er_web_command(1,-1,1)==0);
        CHECK(er_web_step(10000)==0);
    }
    CHECK(strstr(er_web_state(),"\"enabled\":1")!=NULL);
    er_web_set_fault(1,0,0);
    CHECK(er_web_step(10000)!=0);
    CHECK(strstr(er_web_state(),"\"enabled\":0")!=NULL);
    er_web_set_fault(0,0,0);
    CHECK(er_web_command(1,-1,1)!=0);
    er_web_reset();
    CHECK(er_web_command(1,-1,1)==0);
    for(int i=0;i<11;i++) CHECK(er_web_step(10000)==0);
    CHECK(er_web_step(10000)!=0);
    puts("web bridge tests passed");return 0;
}
