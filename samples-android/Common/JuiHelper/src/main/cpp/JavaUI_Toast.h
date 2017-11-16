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

#ifndef JAVAUI_TOAST_H_
#define JAVAUI_TOAST_H_

#include "JavaUI_View.h"

namespace jui_helper {

/*
 * JUIToast class
 * To show the toast, use this class like below.
 * Note that this class needs to be invoked inside UIThread
 *
 *  ndk_helper::JNIHelper::GetInstance()->RunOnUiThread( []()
 {
 jui_helper::JUIToast toast("Toast!");
 toast.Show();
 } );
 *
 */
class JUIToast : JUIBase {
  friend class JUIWindow;

public:
  JUIToast();
  JUIToast(const char *str);
  JUIToast(const char *str, const int32_t duration);
  virtual ~JUIToast();

  /*
   * Set a view in the toast
   */
  void SetView(JUIView *view);

  /*
   * Show the toast
   */
  void Show();

  /*
   * Cancel (pending) toast
   */
  void Cancel();

  /*
   * Set attributes to the component
   * For a list of attributes, refer attributes_ array
   */
  template <typename T> bool SetAttribute(const char *strAttribute, const T t) {
    return JUIBase::SetAttribute(map_attributes_, strAttribute, t);
  }

  bool SetAttribute(const char *strAttribute, const char *str) {
    return JUIBase::SetAttribute(map_attributes_, strAttribute, str);
  }

  template <typename T, typename T2>
  bool SetAttribute(const char *strAttribute, T t, T2 t2) {
    return JUIBase::SetAttribute(map_attributes_, strAttribute, t, t2);
  }

  template <typename T, typename T2, typename T3>
  bool SetAttribute(const char *strAttribute, T p1, T2 p2, T3 p3) {
    return JUIBase::SetAttribute(map_attributes_, strAttribute, p1, p2, p3);
  }

  /*
   * Get attribute
   */
  template <typename T> bool GetAttribute(const char *strAttribute, T &value) {
    return JUIBase::GetAttribute(map_attributes_, strAttribute, value);
  }

private:
  static const AttributeType attributes_[];

  void InitToast();

protected:
  static std::unordered_map<std::string, int32_t> map_attributes_;

};

} //namespace ndkHelper

#endif /* JAVAUI_TOAST_H_ */
