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

namespace jui_helper {

/*
 * JUIToast, showing java ui toast
 */
std::unordered_map<std::string, int32_t> JUIToast::map_attributes_;
const AttributeType JUIToast::attributes_[] = {
  { "Duration", ATTRIBUTE_PARAMETER_INT },
  { "Gravity", ATTRIBUTE_PARAMETER_III }, { "Margin", ATTRIBUTE_PARAMETER_FF },
  { "Text", ATTRIBUTE_PARAMETER_STRING },
};

JUIToast::JUIToast() { InitToast(); }

JUIToast::JUIToast(const char *str) {
  InitToast();
  SetAttribute("Text", str);
}

JUIToast::JUIToast(const char *str, const int32_t duration) {
  InitToast();
  SetAttribute("Text", str);
  SetAttribute("Duration", duration);

}

void JUIToast::InitToast() {
  // setup attribute map (once)
  if (map_attributes_.size() == 0) {
    for (int32_t i = 0; i < sizeof(attributes_) / sizeof(attributes_[0]); ++i) {
      map_attributes_[std::string(attributes_[i].attribute_name)] =
          attributes_[i].attribute_type;
    }
  }

  // Init toast object
  JNIEnv *env = ndk_helper::JNIHelper::GetInstance()->AttachCurrentThread();
  jclass cls = ndk_helper::JNIHelper::GetInstance()->RetrieveClass(
      env, "android/widget/Toast");

  jmethodID mid = env->GetStaticMethodID(
      cls, "makeText", "(Landroid/content/Context;Ljava/lang/"
                       "CharSequence;I)Landroid/widget/Toast;");

  jobject context = JUIWindow::GetInstance()->GetContext();
  jstring emptyString = env->NewStringUTF("");
  jobject obj = env->CallStaticObjectMethod(cls, mid, context, emptyString, 0);
  obj_ = env->NewGlobalRef(obj);
  if (obj_ == NULL)
    LOGI("Class initialization failure");

  env->DeleteLocalRef(emptyString);
  env->DeleteLocalRef(obj);
  env->DeleteLocalRef(cls);
}

JUIToast::~JUIToast() {
  if (obj_) {
    ndk_helper::JNIHelper::GetInstance()->DeleteObject(obj_);
    obj_ = NULL;
  }
}

void JUIToast::SetView(JUIView *view) {
  ndk_helper::JNIHelper::GetInstance()->CallVoidMethod(
      obj_, "show", "(Landroid/view/View;)V", view->GetJobject());
}

void JUIToast::Show() {
  ndk_helper::JNIHelper::GetInstance()->CallVoidMethod(obj_, "show", "()V");
}

void JUIToast::Cancel() {
  ndk_helper::JNIHelper::GetInstance()->CallVoidMethod(obj_, "cancel", "()V");
}

} // namespace jui_helper
