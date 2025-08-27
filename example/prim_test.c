#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include "tachyon.h"

void prim_test(TachyonEnv* env) {
	TachyonDatum* d1 = Tachyon_Env_PopStack(env);
	TachyonDatum* d2 = Tachyon_Env_PopStack(env);
	Tachyon_Env_PushStack(env, Tachyon_New_Bool(d2->v.intval >= d1->v.intval));
}

int main() {
	Tachyon_Initialize();
	Tachyon_RegisterPrimitive(">=", prim_test);
	TachyonEnv* env = Tachyon_Env_New();
	Tachyon_Exec(env, Tachyon_New_Int(3));
	Tachyon_Exec(env, Tachyon_New_Int(5));
	Tachyon_Exec(env, Tachyon_New_Word(">="));
	printf("------------------------\n");
	Tachyon_Exec(env, Tachyon_New_Word("dstk"));
	Tachyon_Env_Dispose(env);
	Tachyon_Teardown();
	return 0;
}

