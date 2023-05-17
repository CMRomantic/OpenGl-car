#include "jni.h"
#include "config.h"
#include "JNIHelp.h"
#include "src/GLUtils.h"
#include "src/GLRender.h"
#include "FileUtils.cpp"
#include "unordered_map"

#ifdef CAR_LOG_TAG
#undef CAR_LOG_TAG
#endif //CAR_LOG_TAG
#define CAR_LOG_TAG  "MainApp"

#define APP_PACKAGE_CLASS_NAME "com/aonions/opengl/jni/MainApp"
#define GLSL_TRIANGLE_V_SHADER "sdcard/glsl/triangle/vShader.glsl"
#define GLSL_TRIANGLE_F_SHADER "sdcard/glsl/triangle/fShader.glsl"
#define GLSL_V_SHADER_PATH  "circle/vShader.glsl"
#define GLSL_F_SHADER_PATH  "circle/fShader.glsl"

AAssetManager* manager = nullptr;

namespace android {

    jstring test(JNIEnv *env, jclass type, jobject assetManager) {
        manager = AAssetManager_fromJava(env, assetManager);
        if (manager == nullptr) {
            return env->NewStringUTF("assetManager is null.");
        }
        return env->NewStringUTF("222");
    }

    void onSurfaceCreated(JNIEnv *env, jclass type) {
        GLRender::OnSurfaceCreated();
        static const int BUF_SZ = 1024;
//        char vData[BUF_SZ + 1] = {0};
//        char fData[BUF_SZ + 1] = {0};
//        FileUtils::readFile(GLSL_TRIANGLE_V_SHADER, vData, BUF_SZ);
//        FileUtils::readFile(GLSL_TRIANGLE_F_SHADER, fData, BUF_SZ);
//        std::string vData = FileUtils::readFile(GLSL_TRIANGLE_V_SHADER);
//        std::string fData = FileUtils::readFile(GLSL_TRIANGLE_F_SHADER);

        char * vData = FileUtils::readFileAsset(GLSL_V_SHADER_PATH,manager);
        char * fData = FileUtils::readFileAsset(GLSL_F_SHADER_PATH, manager);
//
        LOGD("read buffer:\n%s",vData);
//
        GLRender::getInstance()->init(vData, fData);
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
            {"test",             "(Landroid/content/res/AssetManager;)Ljava/lang/String;", (void *) test},
            {"onSurfaceCreated", "()V",                                                    (void *) onSurfaceCreated},
            {"onSurfaceChanged", "(II)V",                                                  (void *) onSurfaceChanged},
            {"onDrawFrame",      "()V",                                                    (void *) onDrawFrame},
            {"onDestroy",        "()V",                                                    (void *) onDestroy},
    };

    int register_MainApp(JNIEnv *env) {
        return jniRegisterNativeMethods(env, APP_PACKAGE_CLASS_NAME, methodsRx, NELEM(methodsRx));
    }
};