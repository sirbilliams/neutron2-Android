#include "OMediaControlSettings.h"
#include <assert.h>
#include <stdio.h>
int main() {
    using namespace omcontrols;
    Settings c;
    assert(c.map[6]==6 && c.movement==0 && c.camera==20 && c.speed==100);
    assert(c.mapping("1,0,2,3,9,10,7,8") && c.map[0]==1 && c.map[1]==0);
    assert(!c.mapping("1,1,2,3,9,10,7,8"));
    assert(!c.mapping("0,1,2,3,6,10,7,8")); // Start cannot be stolen
    assert(!c.mapping("0,1,2,3,9,10,7,8,9"));
    assert(!c.mapping("0,1,2,3,9,10,7"));
    assert(deadzone(0.1f,20)==0 && deadzone(-0.1f,20)==0);
    assert(fabsf(deadzone(0.6f,20)-0.5f)<0.0001f);
    assert(fabsf(deadzone(-0.6f,20)+0.5f)<0.0001f);
    assert(deadzone(1,35)==1 && deadzone(-1,35)==-1);
    assert(bounded("nan",100,25,250)==100);
    assert(bounded("-1",100,25,250)==100);
    assert(bounded("250",100,25,250)==250);
    puts("Controller remapping and dead-zone regressions passed");
}
