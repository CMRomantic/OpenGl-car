#include "jni.h"
#include "config.h"
#include "JNIHelp.h"
#include "src/GLUtils.h"
#include "src/GLRender.h"

#ifdef CAR_LOG_TAG
#undef CAR_LOG_TAG
#endif //CAR_LOG_TAG
#define CAR_LOG_TAG  "MainApp"

#define APP_PACKAGE_CLASS_NAME "com/aonions/opengl/jni/MainApp"

namespace android {

    void initGl(JNIEnv *env, jclass type) {
        GLUtils::initGl();
    }

    void onSurfaceCreated(JNIEnv *env, jclass type) {
        GLRender::OnSurfaceCreated();
    }

    void onSurfaceChanged(JNIEnv *env, jclass type, jint width, jint height) {
        GLRender::OnSurfaceChanged(width, height);
    }

    void onDrawFrame(JNIEnv *env, jclass type) {
        GLRender::getInstance()->OnDrawFrame();
    }

    void onDestroy(JNIEnv *env, jclass type) {
        GLRender::getInstance()->destroy();
    }

    //------------------------------------jni loaded----------------------------------------------------------

    static const JNINativeMethod methodsRx[] = {
            /*{"test",             "()Ljava/lang/String;", (void *) test},*/
            {"initGl",           "()V",                  (void *) initGl},
            {"onSurfaceCreated", "()V",                  (void *) onSurfaceCreated},
            {"onSurfaceChanged", "(II)V",                (void *) onSurfaceChanged},
            {"onDrawFrame",      "()V",                  (void *) onDrawFrame},
            {"onDestroy",        "()V",                  (void *) onDestroy},
    };

    int register_MainApp(JNIEnv *env) {
        return jniRegisterNativeMethods(env, APP_PACKAGE_CLASS_NAME, methodsRx, NELEM(methodsRx));
    }
};