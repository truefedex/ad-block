/* Copyright (c) 2015 Brian R. Bondy. Distributed under the MPL2 license.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <jni.h>
#include <android/log.h>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <stdexcept>

#include "../../ad_block_client.h"

#define  LOG_TAG    "ad_block_client"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

struct ClientData {
    AdBlockClient *client;
    std::string raw_data;
    std::vector<char> parsed_data;
};

std::string getFileContents(const char *filename) {
    std::ifstream in(filename, std::ios::in);
    if (in) {
        std::ostringstream contents;
        contents << in.rdbuf();
        in.close();
        return(contents.str());
    }
    throw std::runtime_error(std::string("File read error: ") + std::to_string(errno));
}

std::vector<char> getFileContent(const char * path) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<char> buffer(size);
    if (file.read(buffer.data(), size)) {
        return buffer;
    }
    throw std::runtime_error(std::string("File read error: ") + std::to_string(errno));
}

void writeFile(const char *filename, const char *buffer, int length) {
    std::ofstream outFile(filename, std::ios::out | std::ios::binary);
    if (outFile) {
        outFile.write(buffer, length);
        outFile.close();
        return;
    }
    throw std::runtime_error(std::string("File write error: ") + std::to_string(errno));
}

extern "C" {

static jfieldID cached_abc_native_this_field_id;

JNIEXPORT jint JNI_OnLoad(JavaVM* vm, void* reserved) {
    JNIEnv * env;
    if (vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) != JNI_OK) {
        return JNI_ERR;
    }

    jclass abc_class = env->FindClass("com/brave/adblock/AdBlockClient");
    cached_abc_native_this_field_id = env->GetFieldID(abc_class, "nativeThis", "J");

    return JNI_VERSION_1_6;
}

JNIEXPORT void JNICALL
Java_com_brave_adblock_AdBlockClient_init(JNIEnv *env, jobject thiz) {
    auto clientData = new ClientData();
    clientData->client = new AdBlockClient();
    env->SetLongField(thiz, cached_abc_native_this_field_id, reinterpret_cast<jlong>(clientData));
}

JNIEXPORT void JNICALL
Java_com_brave_adblock_AdBlockClient_deinit(JNIEnv *env, jobject thiz) {
    auto clientData = reinterpret_cast<ClientData *>(env->GetLongField(thiz,
                                                                      cached_abc_native_this_field_id));
    delete clientData->client;
    delete clientData;
}

JNIEXPORT jboolean JNICALL
Java_com_brave_adblock_AdBlockClient_parse(JNIEnv *env, jobject thiz, jstring input) {
    auto clientData = reinterpret_cast<ClientData *>(env->GetLongField(thiz,
                                                                      cached_abc_native_this_field_id));
    const char *native_input = env->GetStringUTFChars(input, 0);
    clientData->raw_data = native_input;
    env->ReleaseStringUTFChars(input, native_input);
    return clientData->client->parse(clientData->raw_data.c_str());
}

JNIEXPORT jboolean JNICALL
Java_com_brave_adblock_AdBlockClient_parseFile(JNIEnv *env, jobject thiz, jstring file_name) {
    auto clientData = reinterpret_cast<ClientData *>(env->GetLongField(thiz,
                                                                      cached_abc_native_this_field_id));
    const char *native_file_name = env->GetStringUTFChars(file_name, 0);
    clientData->raw_data = getFileContents(native_file_name);
    env->ReleaseStringUTFChars(file_name, native_file_name);
    return clientData->client->parse(clientData->raw_data.c_str());
}

JNIEXPORT jboolean JNICALL
Java_com_brave_adblock_AdBlockClient_serialize(JNIEnv *env, jobject thiz, jstring jfile_name) {
    auto clientData = reinterpret_cast<ClientData *>(env->GetLongField(thiz,
                                                                      cached_abc_native_this_field_id));
    const char *native_file_name = env->GetStringUTFChars(jfile_name, nullptr);
    std::string file_name(native_file_name);
    env->ReleaseStringUTFChars(jfile_name, native_file_name);
    int size;
    char *buffer = clientData->client->serialize(&size);
    try {
        writeFile(file_name.c_str(), buffer, size);
    } catch (const std::exception& e) {
        LOGE("Error while writing serialized data to file: %s", e.what());
        delete[] buffer;
        return JNI_FALSE;
    }

    delete[] buffer;
    return JNI_TRUE;
}

JNIEXPORT jboolean JNICALL
Java_com_brave_adblock_AdBlockClient_deserialize(JNIEnv *env, jobject thiz, jstring jfile_name) {
    auto clientData = reinterpret_cast<ClientData *>(env->GetLongField(thiz,
                                                                      cached_abc_native_this_field_id));
    const char *native_file_name = env->GetStringUTFChars(jfile_name, nullptr);
    std::string file_name(native_file_name);
    env->ReleaseStringUTFChars(jfile_name, native_file_name);
    try {
        clientData->parsed_data = getFileContent(file_name.c_str());
    } catch (const std::exception& e) {
        LOGE("Error while reading serialized data from file: %s", e.what());
        return JNI_FALSE;
    }

    return clientData->client->deserialize(clientData->parsed_data.data());
}

JNIEXPORT jboolean JNICALL
Java_com_brave_adblock_AdBlockClient_matches(JNIEnv *env, jobject thiz, jstring jurl_to_check,
                                             jint filter_option,
                                             jstring jcontext_domain) {
    auto clientData = reinterpret_cast<ClientData *>(env->GetLongField(thiz,
                                                                      cached_abc_native_this_field_id));
    const char *url = env->GetStringUTFChars(jurl_to_check, nullptr);
    const char *context_domain = env->GetStringUTFChars(jcontext_domain, nullptr);
    auto result = clientData->client->matches(url, FilterOption(filter_option), context_domain);
    env->ReleaseStringUTFChars(jcontext_domain, context_domain);
    env->ReleaseStringUTFChars(jurl_to_check, url);
    return result ? JNI_TRUE : JNI_FALSE;
}

}
