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

#include "JavaUI.h"

namespace jui_helper
{

//-------------------------------------------------
//JUI Window
//-------------------------------------------------
JUIWindow::JUIWindow() :
        suspended_( false ), windowDestroyed_( false ), jni_helper_java_class_( NULL ), jni_helper_java_ref_( NULL )
{
    views_.clear();
}

JUIWindow::~JUIWindow()
{
    ndk_helper::JNIHelper* helper = ndk_helper::JNIHelper::GetInstance();
    JNIEnv *env = helper->Lock();

    env->DeleteGlobalRef( jni_helper_java_ref_ );
    env->DeleteGlobalRef( jni_helper_java_class_ );

    helper->Unlock();
}

//-------------------------------------------------
//Signleton
//-------------------------------------------------
JUIWindow * JUIWindow::GetInstance()
{
    static JUIWindow window;
    return &window;
}

jobject JUIWindow::GetHelperClassInstance()
{
    if( JUIWindow::GetInstance()->jni_helper_java_ref_ == NULL )
        LOGE( "JUIWindow::GetHelperClass(): Null helper class instance!" );
    return JUIWindow::GetInstance()->jni_helper_java_ref_;
}

jclass JUIWindow::GetHelperClass()
{
    if( JUIWindow::GetInstance()->jni_helper_java_class_ == NULL )
        LOGE( "JUIWindow::GetHelperClass(): Null helper class!" );
    return JUIWindow::GetInstance()->jni_helper_java_class_;
}

//---------------------------------------------------------------------------
//Init
//---------------------------------------------------------------------------
void JUIWindow::Init( ANativeActivity* activity,
        const char* helper_class_name )
{
    JUIWindow& window = *GetInstance();
    LOGI( "Initialized Java UI" );
    if( window.activity_ )
    {
        LOGI( "The class has been already initialized with an activity" );
        return;
    }
    window.activity_ = activity;

    ndk_helper::JNIHelper& helper = *ndk_helper::JNIHelper::GetInstance();
    JNIEnv *env = helper.Lock();

    if( helper_class_name != NULL && window.jni_helper_java_class_ == NULL && window.jni_helper_java_ref_ == NULL )
    {
        //Instantiate JUIHelper class
        jclass cls = helper.RetrieveClass( env, helper_class_name );

        window.jni_helper_java_class_ = (jclass) env->NewGlobalRef( cls );

        jmethodID constructor = env->GetMethodID( window.jni_helper_java_class_, "<init>",
                "(Landroid/app/NativeActivity;)V" );
        window.jni_helper_java_ref_ = env->NewObject( window.jni_helper_java_class_, constructor, activity->clazz );
        window.jni_helper_java_ref_ = env->NewGlobalRef( window.jni_helper_java_ref_ );
    }

    //Create popupWindow
    jmethodID mid = env->GetMethodID( window.jni_helper_java_class_, "createPopupWindow",
            "(Landroid/app/NativeActivity;)Landroid/widget/PopupWindow;" );
    jobject obj = env->CallObjectMethod( window.jni_helper_java_ref_, mid, window.activity_->clazz );
    obj = env->NewGlobalRef( obj );

    helper.Unlock();

    if( obj == NULL )
    {
        LOGI( "Failed creating popupWindow" );
    }
    window.popupWindow_ = obj;
}

//---------------------------------------------------------------------------
//Suspend
//---------------------------------------------------------------------------
void JUIWindow::Suspend( const int32_t cmd )
{
    if( cmd == APP_CMD_TERM_WINDOW )
        windowDestroyed_ = true;

    if( suspended_ == true )
    {
        LOGI( "The window has been suspended already" );
        return;
    }

    LOGI( "Suspending JUI" );
    if( popupWindow_ )
    {
        ndk_helper::JNIHelper::GetInstance()->CallVoidMethod( jni_helper_java_ref_, "suspendPopupWindow",
                "(Landroid/widget/PopupWindow;)V", popupWindow_ );
        popupWindow_ = NULL;
    }

    if( dialog_ )
    {
        dialog_->Suspend();
    }

    suspended_ = true;
}

//---------------------------------------------------------------------------
//Resume
//---------------------------------------------------------------------------
void JUIWindow::Resume( ANativeActivity* activity,
        const int32_t cmd )
{
    if( suspended_ != true )
    {
        LOGI( "The window has not been suspended" );
        return;
    }

    /*
     * Special case handling for sleep
     * In case of the situation that
     * 1) screen has been slept
     * 2) but lockscreen has not been initiated
     * Only a message coming is:
     * APP_CMD_START & APP_CMD_TERM_RESUME
     * Usually windows should not be initialized at these message,
     * but we do it only for the situation
     */
    LOGI( "resuming %d, %d", cmd, windowDestroyed_ );
    if( windowDestroyed_ == true && cmd == APP_CMD_RESUME )
    {
        LOGI( "Won't resume JUI" );
        return;
    }

    LOGI( "Resuming JUI" );

    //Re-initialize popupWindow
    activity_ = NULL;
    Init( activity );

    ndk_helper::JNIHelper::GetInstance()->CallVoidMethod( jni_helper_java_ref_, "resumePopupWindow",
            "(Landroid/app/NativeActivity;Landroid/widget/PopupWindow;)V", activity_->clazz, popupWindow_ );

    //Restore widgets
    auto itBegin = views_.begin();
    auto itEnd = views_.end();
    while( itBegin != itEnd )
    {
        //Restore
        (*itBegin)->Restore();
        ndk_helper::JNIHelper::GetInstance()->CallVoidMethod( jni_helper_java_ref_, "addView", "(Landroid/view/View;)V",
                (*itBegin)->GetJobject() );

        itBegin++;
    }

    //Restore dialog
    if( dialog_ )
        dialog_->Resume( activity );

    suspended_ = false;
    windowDestroyed_ = false;
}

//---------------------------------------------------------------------------
//Close
//---------------------------------------------------------------------------
void JUIWindow::Close()
{
    LOGI( "Closing JUI" );
    ndk_helper::JNIHelper::GetInstance()->CallVoidMethod( jni_helper_java_ref_, "closePopupWindow",
            "(Landroid/widget/PopupWindow;)V", popupWindow_ );
    ndk_helper::JNIHelper::GetInstance()->DeleteObject( popupWindow_ );

    auto itBegin = views_.begin();
    auto itEnd = views_.end();
    while( itBegin != itEnd )
    {
        delete *itBegin;
        itBegin++;
    }

    views_.clear();

    if( dialog_ )
        dialog_->Close();

    activity_ = NULL;
    popupWindow_ = NULL;
}

//---------------------------------------------------------------------------
//Add JUIView to popup window
//---------------------------------------------------------------------------
void JUIWindow::AddView( JUIView* view )
{
    ndk_helper::JNIHelper::GetInstance()->CallVoidMethod( jni_helper_java_ref_, "addView", "(Landroid/view/View;)V",
            view->GetJobject() );
    views_.push_back( view );
}

jobject JUIWindow::CreateWidget( const char* strWidgetName,
        void* id )
{
    if( activity_ == NULL )
    {
        LOGI( "JNIHelper has not been initialized. Call init() to initialize the helper" );
        return NULL;
    }

    ndk_helper::JNIHelper* helper = ndk_helper::JNIHelper::GetInstance();
    JNIEnv *env = helper->Lock();

    //Create widget
    jstring name = env->NewStringUTF( strWidgetName );
    static jmethodID mid = NULL;
    if( mid == NULL )
    {
        mid = env->GetMethodID( jni_helper_java_class_, "createWidget", "(Ljava/lang/String;I)Landroid/view/View;" );
        if( mid == NULL )
        {
            LOGI( "method ID 'createWidget', '(Ljava/lang/String;)Landroid/view/View;' not found" );
            helper->Unlock();
            return NULL;
        }
    }

    jobject obj = env->CallObjectMethod( jni_helper_java_ref_, mid, name, (int32_t) id );
    obj = env->NewGlobalRef( obj );

    env->DeleteLocalRef( name );
    helper->Unlock();

    return obj;
}

void JUIWindow::CloseWidget( jobject obj )
{
    if( activity_ == NULL )
    {
        LOGI( "JNIHelper has not been initialized. Call init() to initialize the helper" );
        return;
    }

    ndk_helper::JNIHelper* helper = ndk_helper::JNIHelper::GetInstance();
    JNIEnv *env = helper->Lock();

    static jmethodID mid = NULL;
    if( mid == NULL )
    {
        mid = env->GetMethodID( jni_helper_java_class_, "closeWidget", "(Landroid/view/View;)V" );
        if( mid == NULL )
        {
            LOGI( "method  not found" );
            helper->Unlock();
            return;
        }
    }

    env->CallVoidMethod( jni_helper_java_ref_, mid, obj );
    env->DeleteGlobalRef( obj );

    helper->Unlock();
    return;
}

}   //namespace jui_helper
