//
// Created by cmeng on 2023/4/26.
//
#include "jni.h"
#include <android/asset_manager_jni.h>
#include <cstdio>
#include <string>
#include <fstream>
#include "LogUtils.h"

using namespace std;

class FileUtils {
public:
    static int readFile(const char *path, char *data, int length) {
        FILE *p = nullptr;
        size_t ret = 0;
        p = fopen(path, "r");

        if (p == nullptr) {
            LOGE("file open failed error,%s", path);
            return -1;
        }

        ret = fread(data, 1, length, p);

        if (ret <= 0) {
            fclose(p);
            LOGE("read file path buffer is error");
            return -1;
        }
        fclose(p);
        return (int) ret;
    };

    static char *readFileAsset(const char *file, AAssetManager *manager) {
        AAsset *pAsset = nullptr;
        char *pBuffer = nullptr;
        off_t size = -1;

        if (manager == nullptr) {
            LOGE("AAssetManager is null!");
            return nullptr;
        }
        pAsset = AAssetManager_open(manager, file, AASSET_MODE_UNKNOWN);

        size = AAsset_getLength(pAsset);
        LOGI("after AAssetManager_open");
        pBuffer = (char *) malloc(size + 1);
        pBuffer[size] = '\0';
        AAsset_read(pAsset, pBuffer, size);
        AAsset_close(pAsset);
        return pBuffer;
    }

    static string readFile(const char *filename) {
        std::string content;
        std::ifstream input(filename);
        if (input.is_open()) {
            input.seekg(0, std::ios::end);
            content.reserve(input.tellg());
            input.seekg(0, std::ios::beg);
            content.assign((std::istreambuf_iterator<char>(input)),
                           std::istreambuf_iterator<char>());
        } else {
            LOGE("read file error:%s", filename);
        }
        return content;
    }

    static string readShaderFromFile(const string &filePath) {
        ifstream inputFileStream(filePath, ios::in | ios::binary);
        if (inputFileStream) {
            string shaderCode;
            inputFileStream.seekg(0, ios::end);
            shaderCode.resize(inputFileStream.tellg());
            inputFileStream.seekg(0, ios::beg);
            inputFileStream.read(&shaderCode[0], shaderCode.size());
            inputFileStream.close();
            if (shaderCode.size() >= 3 && shaderCode[0] == '\xEF' && shaderCode[1] == '\xBB' &&
                shaderCode[2] == '\xBF') {
                // 删除UTF-8编码的BOM
                shaderCode.erase(0, 3);
            }
            return shaderCode;
        } else {
            LOGE("Error: Failed to open file");
            return "";
        }
    }

    static string generateUUID() {
        static const int BUF_SZ = 37;
        char buf[BUF_SZ + 1] = {0};
        readFile("/proc/sys/kernel/random/uuid", buf, BUF_SZ);
        buf[BUF_SZ - 1] = 0;
        return string{buf};
    };
};