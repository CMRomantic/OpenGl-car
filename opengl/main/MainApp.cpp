#include "jni.h"
#include "config.h"
#include "JNIHelp.h"
#include "src/GLUtils.h"
#include "src/GLRender.h"
#include "FileUtils.cpp"

#ifdef CAR_LOG_TAG
#undef CAR_LOG_TAG
#endif //CAR_LOG_TAG
#define CAR_LOG_TAG  "MainApp"

#define APP_PACKAGE_CLASS_NAME "com/aonions/opengl/jni/MainApp"
#define GLSL_TRIANGLE_V_SHADER "sdcard/glsl/triangle/vShader.glsl"
#define GLSL_TRIANGLE_F_SHADER "sdcard/glsl/triangle/fShader.glsl"

namespace android {

    jstring test(JNIEnv *env, jclass type){

        std::string folder_path = "sdcard/glsl/triangle/vShader.glsl";

        std::string filename = std::string("main/glsl/triangle") + "/vShader.glsl";

        std::string content = FileUtils::readFile(folder_path.c_str());

//        static const int BUF_SZ = 1024;
//        char vData[BUF_SZ + 1] = {0};
//
//        FileUtils::readFile(folder_path.c_str(),vData, BUF_SZ);

        LOGD("read buffer:\n%s",content.c_str());


        return env->NewStringUTF(folder_path.c_str());
    }

    void onSurfaceCreated(JNIEnv *env, jclass type) {
        GLRender::OnSurfaceCreated();
        static const int BUF_SZ = 1024;
//        char vData[BUF_SZ + 1] = {0};
//        char fData[BUF_SZ + 1] = {0};
//        FileUtils::readFile(GLSL_TRIANGLE_V_SHADER, vData, BUF_SZ);
//        FileUtils::readFile(GLSL_TRIANGLE_F_SHADER, fData, BUF_SZ);
        std::string vData = FileUtils::readFile(GLSL_TRIANGLE_V_SHADER);
        std::string fData = FileUtils::readFile(GLSL_TRIANGLE_F_SHADER);
        //LOGD("read buffer:\n%s",vData.c_str());
        GLRender::getInstance()->init(vData.c_str(), fData.c_str());
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
            {"test",             "()Ljava/lang/String;", (void *) test},
            {"onSurfaceCreated", "()V",   (void *) onSurfaceCreated},
            {"onSurfaceChanged", "(II)V", (void *) onSurfaceChanged},
            {"onDrawFrame",      "()V",   (void *) onDrawFrame},
            {"onDestroy",        "()V",   (void *) onDestroy},
    };

    int register_MainApp(JNIEnv *env) {
        return jniRegisterNativeMethods(env, APP_PACKAGE_CLASS_NAME, methodsRx, NELEM(methodsRx));
    }
};