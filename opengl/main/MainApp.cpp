#include "jni.h"
#include "config.h"
#include "JNIHelp.h"

#ifdef CAR_LOG_TAG
#undef CAR_LOG_TAG
#endif //CAR_LOG_TAG
#define CAR_LOG_TAG  "MainApp"

#define APP_PACKAGE_CLASS_NAME "com/aonions/opengl/jni/MainApp"

namespace android {

    jstring test(JNIEnv *env, jclass type){
        return env->NewStringUTF("test-jni");
    }

    //------------------------------------jni loaded----------------------------------------------------------

    static const JNINativeMethod methodsRx[] = {
    	{"test", "()Ljava/lang/String;", (void*)test },
    };

    int register_MainApp(JNIEnv *env){
    	return jniRegisterNativeMethods(env, APP_PACKAGE_CLASS_NAME , methodsRx, NELEM(methodsRx) );
    }
};
