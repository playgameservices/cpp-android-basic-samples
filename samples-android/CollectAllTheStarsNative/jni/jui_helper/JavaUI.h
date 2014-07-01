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

#ifndef JAVAUI_H_
#define JAVAUI_H_

#include <jni.h>
#include <errno.h>
#include <time.h>
#include <vector>
#include <list>
#include <map>
#include <unordered_map>
#include <string>
#include <functional>
#include "JNIHelper.h"

#include "JavaUI_View.h"
#include "JavaUI_Toast.h"

/******************************************************************
 * jui_helper is a helper library to use Java UI easily from Native code.
 * With the helper, an application can instantiate Java controls such as Button,
 * CheckBox,
 * SeekBar from Native code and put it over NativeActivity. Also you can
 * register a native
 * callback to the controls.
 * For a sample implementation how to use jui_helper, refer JavaUI sample.
 *
 * NOTE: To use Dialog, Android Support library is required.
 * Setup support library:
 * http://developer.android.com/tools/support-library/setup.html
 */
namespace jui_helper {

/******************************************************************
 * Helper class to access Java UI from native code
 */

/*
 * Forward decls
 */
class JUIView;
class JUIWindow;
class JUIDialog;

/*
 * JUILinearLayout
 * Corresponding implementation for android.widget.LinearLayout
 */
class JUILinearLayout : public JUIView {
public:
  JUILinearLayout();
  virtual ~JUILinearLayout();

  /*
   * Add JUIView based classes to the linear layout.
   * The layout automatically deletes views in the layout.
   * Caller does not have to delete the added views.
   */
  void AddView(JUIView *view);

  /*
   * Set attributes to the component
   * For a list of attributes, refer attributes_ array
   */
  template <typename T> bool SetAttribute(const char *strAttribute, const T t) {
    return JUIBase::SetAttribute(map_attributes_, strAttribute, t);
  }

private:
  static const AttributeType attributes_[];
  void Init();

protected:
  static std::unordered_map<std::string, int32_t> map_attributes_;
  std::vector<JUIView *> views_;
  virtual void Restore();

  JUILinearLayout(const bool b);
};

/*
 * JUIRelativeLayout
 * Corresponding implementation for android.widget.RelativeLayout
 */
class JUIRelativeLayout : public JUIView {
public:
  JUIRelativeLayout();
  virtual ~JUIRelativeLayout();

  /*
   * Add JUIView based classes to the linear layout.
   * The layout automatically deletes views in the layout.
   * Caller does not have to delete the added views.
   */
  void AddView(JUIView *view);

  /*
   * Set attributes to the component
   * For a list of attributes, refer attributes_ array
   */
  template <typename T> bool SetAttribute(const char *strAttribute, const T t) {
    return JUIBase::SetAttribute(map_attributes_, strAttribute, t);
  }

private:
  static const AttributeType attributes_[];
  void Init();

protected:
  static std::unordered_map<std::string, int32_t> map_attributes_;
  std::vector<JUIView *> views_;
  virtual void Restore();

  JUIRelativeLayout(const bool b);
};

/*
 * JUIRadioGroup
 * In a radio group, you can add multiple radioButtons
 */
class JUIRadioGroup : public JUILinearLayout {
public:
  JUIRadioGroup();
  virtual ~JUIRadioGroup();

private:
protected:
  virtual void Restore();

  JUIRadioGroup(const bool b);
};

/*
 * JUIProgressBar
 * In the constructor you can specify a type of progressbar with
 * PROGRESS_BAR_STYLE parameter.
 */
class JUIProgressBar : public JUIView {
public:
  JUIProgressBar();
  /*
   * Constructer with a style
   */
  JUIProgressBar(const jui_helper::ProgressBarStyleType style);

  virtual ~JUIProgressBar();

  /*
   * Set attributes to the component
   * For a list of attributes, refer attributes_ array
   */
  template <typename T> bool SetAttribute(const char *strAttribute, const T t) {
    return JUIBase::SetAttribute(map_attributes_, strAttribute, t);
  }

private:
  const static AttributeType attributes_[];
  static std::unordered_map<std::string, int32_t> map_attributes_;
  int32_t style_;

protected:
  virtual void Restore();
};

/*
 * JUISeekBar
 */
class JUISeekBar : public JUIProgressBar {
public:
  JUISeekBar();
  virtual ~JUISeekBar();

  /*
   * Set callback to each input event of seekBar widget
   * such as start/stop/change progress
   */
  bool SetCallback(const int32_t message,
                   std::function<void(jui_helper::JUIView *, const int32_t,
                                      const int32_t, const int32_t)> callback);

  /*
   * Dispatch Widget events. This one is called from Java code through
   * Java_com_sample_helper_JUIHelper_JUICallbackHandler()
   */
  virtual void DispatchEvent(const int32_t message, const int32_t param1,
                             const int32_t param2);

  /*
   * Set attributes to the component
   * For a list of attributes, refer attributes_ array
   */
  template <typename T> bool SetAttribute(const char *strAttribute, const T t) {
    return JUIBase::SetAttribute(map_attributes_, strAttribute, t);
  }

private:
  const static AttributeType attributes_[];
  static std::unordered_map<std::string, int32_t> map_attributes_;
  std::function<void(jui_helper::JUIView *, const int32_t, const int32_t,
                     const int32_t)> stop_tracking_callback_;
  std::function<void(jui_helper::JUIView *, const int32_t, const int32_t,
                     const int32_t)> start_tracking_callback_;
  std::function<void(jui_helper::JUIView *, const int32_t, const int32_t,
                     const int32_t)> progress_changed_callback_;
  int32_t current_progress_;

protected:
  virtual void Restore();
};

//-------------------------------------------------
//JUITextView
//-------------------------------------------------
class JUITextView : public JUIView {
public:
  JUITextView();
  JUITextView(const char *str);
  virtual ~JUITextView();

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

  template <typename T, typename T2, typename T3, typename T4>
  bool SetAttribute(const char *strAttribute, T p1, T2 p2, T3 p3, T4 p4) {
    return JUIBase::SetAttribute(map_attributes_, strAttribute, p1, p2, p3, p4);
  }

  /*
   * Retriee attribute of the widget
   */
  template <typename T> bool GetAttribute(const char *strAttribute, T &value) {
    return JUIView::GetAttribute(map_attributes_, strAttribute, value);
  }

private:
  const static AttributeType attributes_[];
  void Init();

protected:
  static std::unordered_map<std::string, int32_t> map_attributes_;
  virtual void Restore();

  JUITextView(const bool b);
};

/*
 * JUIButton
 */
class JUIButton : public JUITextView {
public:
  JUIButton();
  JUIButton(const char *str);
  virtual ~JUIButton();

  /*
   * Dispatch Widget events. This one is called from Java code through
   * Java_com_sample_helper_JUIHelper_JUICallbackHandler()
   */
  virtual void DispatchEvent(const int32_t message, const int32_t param1,
                             const int32_t param2);

  /*
   * Set callback to an input event
   */
  bool SetCallback(
      std::function<void(jui_helper::JUIView *, const int32_t)> callback);

private:
  void Init();
  std::function<void(jui_helper::JUIView *, const int32_t)> onclick_callback_;

protected:
  JUIButton(const bool b);

  virtual void Restore();
};

//-------------------------------------------------
//JUICompoundButton
//-------------------------------------------------
class JUICompoundButton : public JUIButton {
public:
  JUICompoundButton();
  virtual ~JUICompoundButton();

  /*
   * Set callback to an input event
   */
  bool SetCallback(
      std::function<void(jui_helper::JUIView *, const bool)> callback);

  /*
   * Retrieve the check status of the widget
   */
  bool IsChecked() { return current_checked_; }

  /*
   * Dispatch Widget events. This one is called from Java code through
   * Java_com_sample_helper_JUIHelper_JUICallbackHandler()
   */
  virtual void DispatchEvent(const int32_t message, const int32_t param1,
                             const int32_t param2);

  /*
   * Set attributes to the component
   * For a list of attributes, refer attributes_ array
   */
  template <typename T> bool SetAttribute(const char *strAttribute, const T t) {
    return JUIBase::SetAttribute(map_attributes_, strAttribute, t);
  }

private:
  const static AttributeType attributes_[];
  std::function<void(jui_helper::JUIView *, const bool)>
      checked_changed_callback_;

protected:
  static std::unordered_map<std::string, int32_t> map_attributes_;
  bool current_checked_;

  virtual void Restore();
};

/*
 * JUICheckBox
 */
class JUICheckBox : public JUICompoundButton {
public:
  JUICheckBox();
  JUICheckBox(const char *str);
  virtual ~JUICheckBox();

private:
  void Init();

protected:
  virtual void Restore();
};

/*
 * JUISwitch
 */
class JUISwitch : public JUICompoundButton {
public:
  JUISwitch();
  JUISwitch(const char *str);
  virtual ~JUISwitch();

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

private:
  const static AttributeType attributes_[];
  static std::unordered_map<std::string, int32_t> map_attributes_;

  void Init();

protected:
  virtual void Restore();
};

/*
 * JUISwitch
 */
class JUIToggleButton : public JUICompoundButton {
public:
  JUIToggleButton();
  JUIToggleButton(const char *strOn, const char *strOff);
  virtual ~JUIToggleButton();

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

private:
  const static AttributeType attributes_[];
  static std::unordered_map<std::string, int32_t> map_attributes_;

  void Init();

protected:
  virtual void Restore();
};

/*
 * JUICheckedTextView
 */
class JUICheckedTextView : public JUICompoundButton {
public:
  JUICheckedTextView();
  JUICheckedTextView(const char *str);
  virtual ~JUICheckedTextView();

private:
  void Init();

protected:
  virtual void Restore();
};

/*
 * JUICheckedTextView
 */
class JUIRadioButton : public JUICompoundButton {
public:
  JUIRadioButton();
  JUIRadioButton(const char *str);
  virtual ~JUIRadioButton();

private:
  void Init();

protected:
  virtual void Restore();
};

/*
 * JUIWindow represents a popup window with a relative layout put on the window
 * An application can create JUIView based classes and add them to JUIWindow to
 * show Java Widget over
 * native activity.
 */
class JUIWindow {
  /*
   * These classes need to be a friend class to access protected methods
   */
  friend class JUIView;
  friend class JUIProgressBar;
  friend class JUISeekBar;
  friend class JUITextView;
  friend class JUIButton;
  friend class JUICheckBox;
  friend class JUISwitch;
  friend class JUIToggleButton;
  friend class JUIRadioButton;
  friend class JUICheckedTextView;
  friend class JUILinearLayout;
  friend class JUIRelativeLayout;
  friend class JUIRadioGroup;
  friend class JUIToast;
  friend class JUIDialog;

public:
  /*
   * Retrieve the singleton object of the helper.
   * Static member of the class

   * Methods in the class are NOT designed as thread safe.
   */
  static JUIWindow *GetInstance();

  /*
   * Retrieve an instance of JUIHelper Java class
   *
   */
  static jobject GetHelperClassInstance();

  /*
   * Retrieve JUIHelper Java class
   *
   */
  static jclass GetHelperClass();

  /*
   * Initialize window with an activity
   */
  static void Init(ANativeActivity *activity,
                   const char *helper_class_name = NULL);

  /*
   * Close the window
   */
  void Close();

  /*
   * Resume JUIWindow
   * This function needs to be invoked corresponding activity life cycle event
   */
  void Resume(ANativeActivity *activity, const int32_t command);

  /*
   * Suspend JUIWindow
   * This function needs to be invoked corresponding activity life cycle event
   */
  void Suspend(const int32_t command);

  /*
   * Add JUIView classes to the window
   */
  void AddView(JUIView *view);

  /*
   * Get context associated with the window class
   */
  jobject GetContext() {
    if (activity_ == NULL)
      return NULL;
    return activity_->clazz;
  }

  /*
   * Get mutex for JUI helpers
   */
  std::mutex &GetMutex() { return mutex_; }

private:
  JUIWindow();
  ~JUIWindow();
  JUIWindow(const JUIWindow &rhs);
  JUIWindow &operator=(const JUIWindow &rhs);

  ANativeActivity *activity_;
  jobject popupWindow_;

  std::vector<JUIView *> views_;

  bool suspended_;
  bool windowDestroyed_;

  jobject jni_helper_java_ref_;
  jclass jni_helper_java_class_;

  JUIDialog *dialog_;

  //mutex for synchronization
  mutable std::mutex mutex_;

  jobject CreateWidget(const char *strWidgetName, void *id);
  jobject CreateWidget(const char *strWidgetName, void *id,
                       const int32_t param);
  void CloseWidget(jobject obj);

  void SetDialog(JUIDialog *dialog) { dialog_ = dialog; }
};

/*
 * JUIDialog represents a dialog framgent
 * An application can create JUIView based classes and add them to JUIDialog to
 * show Java Widget over
 * native activity.
 */
class JUIDialog : public JUIBase {
  /*
   * These classes need to be a friend class to access protected methods
   */
  friend class JUIView;
  friend class JUIProgressBar;
  friend class JUISeekBar;
  friend class JUITextView;
  friend class JUIButton;
  friend class JUICheckBox;
  friend class JUISwitch;
  friend class JUIToggleButton;
  friend class JUIRadioButton;
  friend class JUICheckedTextView;
  friend class JUILinearLayout;
  friend class JUIRelativeLayout;
  friend class JUIRadioGroup;
  friend class JUIToast;
  friend class JUIWindow;

public:
  JUIDialog();
  JUIDialog(ANativeActivity *activity);
  virtual ~JUIDialog();

  /*
   * Initialize window with an activity
   */
  void Init(ANativeActivity *activity);

  /*
   * Close the dialog
   */
  void Close();

  /*
   * Show dialog
   */
  void Show();

  /*
   * Add JUIView classes to the dialog
   */
  void AddView(JUIView *view);

  /*
   * Set a callback to dialog life cycle event
   */
  bool SetCallback(const int32_t message,
                   std::function<void(jui_helper::JUIDialog *dialog,
                                      const int32_t message)> callback);

  /*
   * Dispatch Widget events. This one is called from Java code through
   * Java_com_sample_helper_JUIHelper_JUICallbackHandler()
   */
  virtual void DispatchEvent(const int32_t message, const int32_t param1,
                             const int32_t param2);

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

private:
protected:
  static const AttributeType attributes_[];
  static std::unordered_map<std::string, int32_t> map_attributes_;

  ANativeActivity *activity_;
  std::vector<JUIView *> views_;
  bool suspended_;

  std::function<void(jui_helper::JUIDialog *dialog, const int32_t message)>
      dismiss_callback_;
  std::function<void(jui_helper::JUIDialog *dialog, const int32_t message)>
      cancel_callback_;

  void CreateDialog();
  void DeleteObject();
  virtual void Suspend();
  virtual void Resume(ANativeActivity *activity);
  void RestoreParameters(std::unordered_map<std::string, int32_t> &map);
};

class JUIAlertDialog : public JUIDialog {
public:
  JUIAlertDialog(ANativeActivity *activity);
  virtual ~JUIAlertDialog();

  /*
   * Initialize window with an activity
   */
  void Init(ANativeActivity *activity);

  /*
   * Dispatch Widget events. This one is called from Java code through
   * Java_com_sample_helper_JUIHelper_JUICallbackHandler()
   */
  virtual void DispatchEvent(const int32_t message, const int32_t param1,
                             const int32_t param2);

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

  bool SetButton(
      const int32_t button, const char *message,
      std::function<void(jui_helper::JUIView *, const int32_t)> callback);

private:
  static const AttributeType attributes_[];
  static std::unordered_map<std::string, int32_t> map_attributes_;

  void CreateDialog();

  std::function<void(jui_helper::JUIView *, const int32_t)> callback_positive_;
  std::function<void(jui_helper::JUIView *, const int32_t)> callback_negative_;
  std::function<void(jui_helper::JUIView *, const int32_t)> callback_neutral_;

protected:
  virtual void Resume(ANativeActivity *activity);
};

}      //namespace ndkHelper
#endif /* JAVAUI_H_ */
