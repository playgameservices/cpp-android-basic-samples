/*
  * Copyright 2013 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <fstream>
#include <iostream>
#include <assert.h>

#include "JNIHelper.h"

namespace ndk_helper
{

#define NATIVEACTIVITY_CLASS_NAME "android/app/NativeActivity"

//---------------------------------------------------------------------------
//JNI Helper functions
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
//Singleton
//---------------------------------------------------------------------------
JNIHelper* JNIHelper::GetInstance()
{
    static JNIHelper helper;
    return &helper;
}

//---------------------------------------------------------------------------
//Ctor
//---------------------------------------------------------------------------
JNIHelper::JNIHelper() :
        lock_count_( 0 )
{
    pthread_mutexattr_t attr;
    pthread_mutexattr_init( &attr );
    pthread_mutexattr_settype( &attr, PTHREAD_MUTEX_RECURSIVE );

    pthread_mutex_init( &mutex_, &attr );
    pthread_mutexattr_destroy( &attr );
}

//---------------------------------------------------------------------------
//Dtor
//---------------------------------------------------------------------------
JNIHelper::~JNIHelper()
{
    pthread_mutex_lock( &mutex_ );

    JNIEnv *env;
    activity_->vm->AttachCurrentThread( &env, NULL );

    env->DeleteGlobalRef( jni_helper_java_ref_ );
    env->DeleteGlobalRef( jni_helper_java_class_ );

    activity_->vm->DetachCurrentThread();

    pthread_mutex_destroy( &mutex_ );
}

//---------------------------------------------------------------------------
//Init
//---------------------------------------------------------------------------
void JNIHelper::Init( ANativeActivity* activity,
        const char* helper_class_name )
{
    JNIHelper& helper = *GetInstance();

    helper.activity_ = activity;

    JNIEnv *env = helper.Lock();

    //Retrieve app bundle id
    jclass android_content_Context = env->GetObjectClass( helper.activity_->clazz );
    jmethodID midGetPackageName = env->GetMethodID( android_content_Context, "getPackageName", "()Ljava/lang/String;" );

    jstring packageName = (jstring) env->CallObjectMethod( helper.activity_->clazz, midGetPackageName );
    const char* appname = env->GetStringUTFChars( packageName, NULL );
    helper.app_bunlde_name_ = std::string( appname );

    //Instantiate JNIHelper class
    jclass cls = helper.RetrieveClass( env, helper_class_name );
    helper.jni_helper_java_class_ = (jclass) env->NewGlobalRef( cls );

    jmethodID constructor = env->GetMethodID( helper.jni_helper_java_class_, "<init>",
            "(Landroid/app/NativeActivity;)V" );

    helper.jni_helper_java_ref_ = env->NewObject( helper.jni_helper_java_class_, constructor, activity->clazz );
    helper.jni_helper_java_ref_ = env->NewGlobalRef( helper.jni_helper_java_ref_ );

    //Get app label
    jstring labelName = (jstring) helper.CallObjectMethod( "getApplicationName", "()Ljava/lang/String;" );
    const char* label = env->GetStringUTFChars( labelName, NULL );
    helper.app_label_ = std::string( label );

    env->ReleaseStringUTFChars( packageName, appname );
    env->ReleaseStringUTFChars( labelName, label );
    helper.Unlock();
}

void JNIHelper::Init( ANativeActivity* activity,
        const char* helper_class_name,
        const char* native_soname )
{
    Init( activity, helper_class_name );
    if( native_soname )
    {
        JNIHelper& helper = *GetInstance();
        JNIEnv *env = helper.Lock();

        //Setup soname
        jstring soname = env->NewStringUTF( native_soname );

        jmethodID mid = env->GetMethodID( helper.jni_helper_java_class_, "loadLibrary", "(Ljava/lang/String;)V" );
        env->CallVoidMethod( helper.jni_helper_java_ref_, mid, soname );

        env->DeleteLocalRef( soname );
        helper.Unlock();
    }
}

//---------------------------------------------------------------------------
//readFile
//---------------------------------------------------------------------------
bool JNIHelper::ReadFile( const char* fileName,
        std::vector<uint8_t>* buffer_ref )
{
    if( activity_ == NULL )
    {
        LOGI( "JNIHelper has not been initialized.Call init() to initialize the helper" );
        return false;
    }

    //First, try reading from externalFileDir;
    JNIEnv *env = Lock();

    jstring str_path = GetExternalFilesDirJString( env );
    const char* path = env->GetStringUTFChars( str_path, NULL );
    std::string s( path );

    if( fileName[ 0 ] != '/' )
    {
        s.append( "/" );
    }
    s.append( fileName );
    std::ifstream f( s.c_str(), std::ios::binary );

    env->ReleaseStringUTFChars( str_path, path );
    env->DeleteLocalRef( str_path );
    activity_->vm->DetachCurrentThread();

    if( f )
    {
        LOGI( "reading:%s", s.c_str() );
        f.seekg( 0, std::ifstream::end );
        int32_t fileSize = f.tellg();
        f.seekg( 0, std::ifstream::beg );
        buffer_ref->reserve( fileSize );
        buffer_ref->assign( std::istreambuf_iterator<char>( f ), std::istreambuf_iterator<char>() );
        f.close();
        Unlock();
        return true;
    }
    else
    {
        //Fallback to assetManager
        AAssetManager* assetManager = activity_->assetManager;
        AAsset* assetFile = AAssetManager_open( assetManager, fileName, AASSET_MODE_BUFFER );
        if( !assetFile )
        {
            Unlock();
            return false;
        }
        uint8_t* data = (uint8_t*) AAsset_getBuffer( assetFile );
        int32_t size = AAsset_getLength( assetFile );
        if( data == NULL )
        {
            AAsset_close( assetFile );

            LOGI( "Failed to load:%s", fileName );
            Unlock();
            return false;
        }

        buffer_ref->reserve( size );
        buffer_ref->assign( data, data + size );

        AAsset_close( assetFile );
        Unlock();
        return true;
    }
}

std::string JNIHelper::GetExternalFilesDir()
{
    if( activity_ == NULL )
    {
        LOGI( "JNIHelper has not been initialized. Call init() to initialize the helper" );
        return std::string( "" );
    }

    //First, try reading from externalFileDir;
    JNIEnv *env = Lock();

    jstring strPath = GetExternalFilesDirJString( env );
    const char* path = env->GetStringUTFChars( strPath, NULL );
    std::string s( path );

    env->ReleaseStringUTFChars( strPath, path );
    env->DeleteLocalRef( strPath );
    Unlock();
    return s;
}

uint32_t JNIHelper::LoadTexture( const char* file_name )
{
    if( activity_ == NULL )
    {
        LOGI( "JNIHelper has not been initialized. Call init() to initialize the helper" );
        return 0;
    }

    JNIEnv *env = Lock();
    jstring name = env->NewStringUTF( file_name );

    GLuint tex;
    glGenTextures( 1, &tex );
    glBindTexture( GL_TEXTURE_2D, tex );

    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

    jmethodID mid = env->GetMethodID( jni_helper_java_class_, "loadTexture", "(Ljava/lang/String;)Z" );
    jboolean ret = env->CallBooleanMethod( jni_helper_java_ref_, mid, name );
    if( !ret )
    {
        glDeleteTextures( 1, &tex );
        tex = -1;
        LOGI( "Texture load failed %s", file_name );
    }

    //Generate mipmap
    glGenerateMipmap (GL_TEXTURE_2D);

    env->DeleteLocalRef( name );
    Unlock();
    return tex;

}

std::string JNIHelper::ConvertString( const char* str,
        const char* encode )
{
    if( activity_ == NULL )
    {
        LOGI( "JNIHelper has not been initialized. Call init() to initialize the helper" );
        return std::string( "" );
    }

    JNIEnv *env = Lock();

    int32_t iLength = strlen( (const char*) str );

    jbyteArray array = env->NewByteArray( iLength );
    env->SetByteArrayRegion( array, 0, iLength, (const signed char*) str );

    jstring strEncode = env->NewStringUTF( encode );

    jclass cls = env->FindClass( "java/lang/String" );
    jmethodID ctor = env->GetMethodID( cls, "<init>", "([BLjava/lang/String;)V" );
    jstring object = (jstring) env->NewObject( cls, ctor, array, strEncode );

    const char *cparam = env->GetStringUTFChars( object, NULL );

    std::string s = std::string( cparam );

    env->ReleaseStringUTFChars( object, cparam );
    env->DeleteLocalRef( strEncode );
    env->DeleteLocalRef( object );

    Unlock();
    return s;
}

//---------------------------------------------------------------------------
//Audio helpers
//---------------------------------------------------------------------------
int32_t JNIHelper::GetNativeAudioBufferSize()
{
    if( activity_ == NULL )
    {
        LOGI( "JNIHelper has not been initialized. Call init() to initialize the helper" );
        return 0;
    }

    JNIEnv *env = Lock();
    jmethodID mid = env->GetMethodID( jni_helper_java_class_, "getNativeAudioBufferSize", "()I" );
    int32_t i = env->CallIntMethod( jni_helper_java_ref_, mid );
    Unlock();
    return i;
}

int32_t JNIHelper::GetNativeAudioSampleRate()
{
    if( activity_ == NULL )
    {
        LOGI( "JNIHelper has not been initialized. Call init() to initialize the helper" );
        return 0;
    }

    JNIEnv *env = Lock();
    jmethodID mid = env->GetMethodID( jni_helper_java_class_, "getNativeAudioSampleRate", "()I" );
    int32_t i = env->CallIntMethod( jni_helper_java_ref_, mid );
    Unlock();
    return i;
}

//---------------------------------------------------------------------------
//Misc implementations
//---------------------------------------------------------------------------
jclass JNIHelper::RetrieveClass( JNIEnv *jni,
        const char* class_name )
{
    jclass activity_class = jni->FindClass( NATIVEACTIVITY_CLASS_NAME );
    jmethodID get_class_loader = jni->GetMethodID( activity_class, "getClassLoader", "()Ljava/lang/ClassLoader;" );
    jobject cls = jni->CallObjectMethod( activity_->clazz, get_class_loader );
    jclass class_loader = jni->FindClass( "java/lang/ClassLoader" );
    jmethodID find_class = jni->GetMethodID( class_loader, "loadClass", "(Ljava/lang/String;)Ljava/lang/Class;" );

    jstring str_class_name = jni->NewStringUTF( class_name );
    jclass class_retrieved = (jclass) jni->CallObjectMethod( cls, find_class, str_class_name );
    jni->DeleteLocalRef( str_class_name );
    return class_retrieved;
}

jstring JNIHelper::GetExternalFilesDirJString( JNIEnv *env )
{
    if( activity_ == NULL )
    {
        LOGI( "JNIHelper has not been initialized. Call init() to initialize the helper" );
        return NULL;
    }

    // Invoking getExternalFilesDir() java API
    jclass cls_Env = env->FindClass( NATIVEACTIVITY_CLASS_NAME );
    jmethodID mid = env->GetMethodID( cls_Env, "getExternalFilesDir", "(Ljava/lang/String;)Ljava/io/File;" );
    jobject obj_File = env->CallObjectMethod( activity_->clazz, mid, NULL );
    jclass cls_File = env->FindClass( "java/io/File" );
    jmethodID mid_getPath = env->GetMethodID( cls_File, "getPath", "()Ljava/lang/String;" );
    jstring obj_Path = (jstring) env->CallObjectMethod( obj_File, mid_getPath );

    return obj_Path;
}

void JNIHelper::DeleteObject( jobject obj )
{
    if( obj == NULL )
    {
        LOGI( "obj can not be NULL" );
        return;
    }

    JNIEnv *env = Lock();
    env->DeleteGlobalRef( obj );
    Unlock();
}

jobject JNIHelper::CallObjectMethod( const char* strMethodName,
        const char* strSignature,
        ... )
{
    if( activity_ == NULL )
    {
        LOGI( "JNIHelper has not been initialized. Call init() to initialize the helper" );
        return NULL;
    }

    JNIEnv *env = Lock();

    //Create widget
    jmethodID mid = env->GetMethodID( jni_helper_java_class_, strMethodName, strSignature );
    if( mid == NULL )
    {
        LOGI( "method ID %s, '%s' not found", strMethodName, strSignature );
        Unlock();
        return NULL;
    }

    va_list args;
    va_start( args, strSignature );
    jobject obj = env->CallObjectMethodV( jni_helper_java_ref_, mid, args );
    va_end( args );

    Unlock();
    return obj;
}

void JNIHelper::CallVoidMethod( const char* strMethodName,
        const char* strSignature,
        ... )
{
    if( activity_ == NULL )
    {
        LOGI( "JNIHelper has not been initialized. Call init() to initialize the helper" );
        return;
    }

    JNIEnv *env = Lock();

    //Create widget
    jmethodID mid = env->GetMethodID( jni_helper_java_class_, strMethodName, strSignature );
    if( mid == NULL )
    {
        LOGI( "method ID %s, '%s' not found", strMethodName, strSignature );
        Unlock();
        return;
    }
    va_list args;
    va_start( args, strSignature );
    env->CallVoidMethodV( jni_helper_java_ref_, mid, args );
    va_end( args );

    Unlock();

    return;
}

jobject JNIHelper::CallObjectMethod( jobject object,
        const char* strMethodName,
        const char* strSignature,
        ... )
{
    if( activity_ == NULL )
    {
        LOGI( "JNIHelper has not been initialized. Call init() to initialize the helper" );
        return NULL;
    }

    JNIEnv *env = Lock();

    //Create widget
    jclass cls = env->GetObjectClass( object );
    jmethodID mid = env->GetMethodID( cls, strMethodName, strSignature );
    if( mid == NULL )
    {
        LOGI( "method ID %s, '%s' not found", strMethodName, strSignature );
        Unlock();
        return NULL;
    }

    va_list args;
    va_start( args, strSignature );
    jobject obj = env->CallObjectMethodV( object, mid, args );
    va_end( args );

    Unlock();
    return obj;
}

void JNIHelper::CallVoidMethod( jobject object,
        const char* strMethodName,
        const char* strSignature,
        ... )
{
    if( activity_ == NULL )
    {
        LOGI( "JNIHelper has not been initialized. Call init() to initialize the helper" );
        return;
    }

    JNIEnv *env = Lock();

    //Create widget
    jclass cls = env->GetObjectClass( object );
    jmethodID mid = env->GetMethodID( cls, strMethodName, strSignature );
    if( mid == NULL )
    {
        LOGI( "method ID %s, '%s' not found", strMethodName, strSignature );
        Unlock();
        return;
    }

    va_list args;
    va_start( args, strSignature );
    env->CallVoidMethodV( object, mid, args );
    va_end( args );

    Unlock();
    return;
}

float JNIHelper::CallFloatMethod( jobject object,
        const char* strMethodName,
        const char* strSignature,
        ... )
{
    float f = 0.f;
    if( activity_ == NULL )
    {
        LOGI( "JNIHelper has not been initialized. Call init() to initialize the helper" );
        return f;
    }

    JNIEnv *env = Lock();

    //Create widget
    jclass cls = env->GetObjectClass( object );
    jmethodID mid = env->GetMethodID( cls, strMethodName, strSignature );
    if( mid == NULL )
    {
        LOGI( "method ID %s, '%s' not found", strMethodName, strSignature );
        Unlock();
        return f;
    }
    va_list args;
    va_start( args, strSignature );
    f = env->CallFloatMethodV( object, mid, args );
    va_end( args );

    Unlock();
    return f;
}

int32_t JNIHelper::CallIntMethod( jobject object,
        const char* strMethodName,
        const char* strSignature,
        ... )
{
    int32_t i = 0;
    if( activity_ == NULL )
    {
        LOGI( "JNIHelper has not been initialized. Call init() to initialize the helper" );
        return i;
    }

    JNIEnv *env = Lock();

    //Create widget
    jclass cls = env->GetObjectClass( object );
    jmethodID mid = env->GetMethodID( cls, strMethodName, strSignature );
    if( mid == NULL )
    {
        LOGI( "method ID %s, '%s' not found", strMethodName, strSignature );
        Unlock();
        return i;
    }
    va_list args;
    va_start( args, strSignature );
    i = env->CallIntMethodV( object, mid, args );
    va_end( args );

    Unlock();
    return i;
}

bool JNIHelper::CallBooleanMethod( jobject object,
        const char* strMethodName,
        const char* strSignature,
        ... )
{
    bool b;
    if( activity_ == NULL )
    {
        LOGI( "JNIHelper has not been initialized. Call init() to initialize the helper" );
        return false;
    }

    JNIEnv *env = Lock();

    //Create widget
    jclass cls = env->GetObjectClass( object );
    jmethodID mid = env->GetMethodID( cls, strMethodName, strSignature );
    if( mid == NULL )
    {
        LOGI( "method ID %s, '%s' not found", strMethodName, strSignature );
        Unlock();
        return false;
    }
    va_list args;
    va_start( args, strSignature );
    b = env->CallBooleanMethodV( object, mid, args );
    va_end( args );
    Unlock();
    return b;
}

jobject JNIHelper::CreateObject( const char* class_name )
{
    JNIEnv *env = Lock();

    jclass cls = env->FindClass( class_name );
    jmethodID constructor = env->GetMethodID( cls, "<init>", "()V" );

    jobject obj = env->NewObject( cls, constructor );
    obj = env->NewGlobalRef( obj );

    Unlock();
    return obj;
}

void JNIHelper::RunOnUiThread( std::function<void()> callback )
{
    JNIEnv *env = Lock();
    static jmethodID mid = NULL;
    if( mid == NULL )
        mid = env->GetMethodID( jni_helper_java_class_, "runOnUIThread", "(J)V" );
    //
    //Allocate temporary function object to be pased around
    std::function < void() > *pCallback = new std::function<void()>( callback );
    env->CallVoidMethod( jni_helper_java_ref_, mid, (int64_t) pCallback );
    Unlock();
}

//This JNI function is invoked from UIThread asynchronously
extern "C"
{
JNIEXPORT
void Java_com_sample_helper_NDKHelper_RunOnUiThreadHandler( JNIEnv* env,
        int64_t pointer )
{
    std::function < void() > *pCallback = (std::function<void()>*) pointer;
    JNIHelper::GetInstance()->Lock( false );
    (*pCallback)();
    JNIHelper::GetInstance()->Unlock();

    //Deleting temporary object
    delete pCallback;
}
}


} //namespace ndkHelper
