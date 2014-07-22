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
#include <string>
#include <functional>
#include "JNIHelper.h"

/******************************************************************
 * jui_helper is a helper library to use Java UI easily from Native code.
 * With the helper, you can instantiate Java controls such as Button, CheckBox,
 * SeekBar from Native code and put it over NativeActivity. Also you can register a native
 * callback to the controls.
 * For a sample implementation how to use jui_helper, refer JavaUI sample.
 *
 * NOTE: To use Dialog, Android Support library is required.
 * Setup support library:  http://developer.android.com/tools/support-library/setup.html
 */
namespace jui_helper
{

enum LAYOUT_PARAMETER
{
    LAYOUT_PARAMETER_UNKNOWN = -2,
    LAYOUT_PARAMETER_TRUE = -1,
    LAYOUT_PARAMETER_LEFT_OF = 0,
    LAYOUT_PARAMETER_RIGHT_OF = 1,
    LAYOUT_PARAMETER_ABOVE = 2,
    LAYOUT_PARAMETER_BELOW = 3,
    LAYOUT_PARAMETER_ALIGN_BASELINE = 4,
    LAYOUT_PARAMETER_ALIGN_LEFT = 5,
    LAYOUT_PARAMETER_ALIGN_TOP = 6,
    LAYOUT_PARAMETER_ALIGN_RIGHT = 7,
    LAYOUT_PARAMETER_ALIGN_BOTTOM = 8,
    LAYOUT_PARAMETER_ALIGN_PARENT_LEFT = 9,
    LAYOUT_PARAMETER_ALIGN_PARENT_TOP = 10,
    LAYOUT_PARAMETER_ALIGN_PARENT_RIGHT = 11,
    LAYOUT_PARAMETER_ALIGN_PARENT_BOTTOM = 12,
    LAYOUT_PARAMETER_CENTER_IN_PARENT = 13,
    LAYOUT_PARAMETER_CENTER_HORIZONTAL = 14,
    LAYOUT_PARAMETER_CENTER_VERTICAL = 15,
    LAYOUT_PARAMETER_START_OF = 16,
    LAYOUT_PARAMETER_END_OF = 17,
    LAYOUT_PARAMETER_ALIGN_START = 18,
    LAYOUT_PARAMETER_ALIGN_END = 19,
    LAYOUT_PARAMETER_ALIGN_PARENT_START = 20,
    LAYOUT_PARAMETER_ALIGN_PARENT_END = 21,
    LAYOUT_PARAMETER_COUNT = 22,
};

enum JUICALLBACK
{
    JUICALLBACK_SEEKBAR_STOP_TRACKING_TOUCH = 1,
    JUICALLBACK_SEEKBAR_START_TRACKING_TOUCH = 2,
    JUICALLBACK_SEEKBAR_PROGRESSCHANGED = 3,
    JUICALLBACK_COMPOUNDBUTTON_CHECKED = 4,
    JUICALLBACK_BUTTON_DOWN = 5,
    JUICALLBACK_BUTTON_UP = 6,
    JUICALLBACK_BUTTON_CANCELED = 7,

    JUICALLBACK_DIALOG_DISMISSED = 108,
    JUICALLBACK_DIALOG_CANCELLED = 109,
};

enum ATTRIBUTE_PARAMETER
{
    ATTRIBUTE_PARAMETER_INT,
    ATTRIBUTE_PARAMETER_FLOAT,
    ATTRIBUTE_PARAMETER_BOOLEAN,
    ATTRIBUTE_PARAMETER_STRING,
    ATTRIBUTE_PARAMETER_IF,	//parameters of int32_t, float
    ATTRIBUTE_PARAMETER_FF,	//parameters of 2 floats
    ATTRIBUTE_PARAMETER_III,	//parameters of int32_t, int32_t, int32_t
    ATTRIBUTE_PARAMETER_IIII,	//parameters of int32_t, int32_t, int32_t, int32_t
    ATTRIBUTE_PARAMETER_FFFI,	//parameters of float, float, float, int32_t
};

enum ATTRIBUTE_GRAVITY
{
    ATTRIBUTE_GRAVITY_TOP = 0x30,
    ATTRIBUTE_GRAVITY_BOTTOM = 0x50,
    ATTRIBUTE_GRAVITY_LEFT = 0x03,
    ATTRIBUTE_GRAVITY_RIGHT = 0x05,
    ATTRIBUTE_GRAVITY_CENTER_VERTICAL = 0x10,
    ATTRIBUTE_GRAVITY_FILL_VERTICAL = 0x70,
    ATTRIBUTE_GRAVITY_CENTER_HORIZONTAL = 0x01,
    ATTRIBUTE_GRAVITY_FILL_HORIZONTAL = 0x07,
    ATTRIBUTE_GRAVITY_CENTER = 0x11,
    ATTRIBUTE_GRAVITY_FILL = 0x77,
    ATTRIBUTE_GRAVITY_CLIP_VERTICAL = 0x80,
    ATTRIBUTE_GRAVITY_CLIP_HORIZONTAL = 0x08,
    ATTRIBUTE_GRAVITY_START = 0x00800003,
    ATTRIBUTE_GRAVITY_END = 0x00800005,
};

enum ATTRIBUTE_UNIT
{
    ATTRIBUTE_UNIT_PX = 0x0,
    ATTRIBUTE_UNIT_DIP = 0x1,
    ATTRIBUTE_UNIT_SP = 0x2,
    ATTRIBUTE_UNIT_PT = 0x3,
    ATTRIBUTE_UNIT_IN = 0x4,
    ATTRIBUTE_UNIT_MM = 0x5,
};

enum ATTRIBUTE_SIZE
{
    ATTRIBUTE_SIZE_MATCH_PARENT = -1, ATTRIBUTE_SIZE_WRAP_CONTENT = -2,
};

enum LAYOUT_ORIENTATION
{
    LAYOUT_ORIENTATION_HORIZONTAL = 0, LAYOUT_ORIENTATION_VERTICAL = 1,
};

enum ALERTDIALOG_BUTTON
{
    ALERTDIALOG_BUTTON_NEGATIVE = -2, ALERTDIALOG_BUTTON_NEUTRAL = -3, ALERTDIALOG_BUTTON_POSITIVE = -1,
};

struct ATTRIBUTE_PARAMETER_STORE
{
    ATTRIBUTE_PARAMETER type;
    union
    {
        int32_t i;
        float f;
        bool b;
        std::string* str;
        struct
        {
            int32_t i1;
            float f2;
        } param_if;
        struct
        {
            float f1;
            float f2;
        } param_ff;
        struct
        {
            int32_t i1;
            int32_t i2;
            int32_t i3;
        } param_iii;
        struct
        {
            float f1;
            float f2;
            float f3;
            int32_t i;
        } param_fffi;
        struct
        {
            int32_t i1;
            int32_t i2;
            int32_t i3;
            int32_t i4;
        } param_iiii;
    };
};

/******************************************************************
 * Helper class to access Java UI from native code
 */
struct ATTRIBUTE_TYPE
{
    const char* attrivute_name;
    const int32_t attirbute_type;
};

class JUIView;
class JUIWindow;
class JUIDialog;

//-------------------------------------------------
//JUIView
//-------------------------------------------------
#include "JavaUI_View.h"
#include "JavaUI_Toast.h"

//-------------------------------------------------
//JUILinearLayout
//-------------------------------------------------
class JUILinearLayout: public JUIView
{
private:
    static ATTRIBUTE_TYPE attributes_[];
    void Init();

protected:
    static std::map<std::string, int32_t> map_attributes_;
    std::vector<JUIView*> views_;
    virtual void Restore();

    JUILinearLayout( const bool b );
public:
    JUILinearLayout();
    virtual ~JUILinearLayout();

    void AddView( JUIView* view );

    template<typename T>
    bool SetAttribute( const char* strAttribute,
            const T t )
    {
        return JUIBase::SetAttribute( map_attributes_, strAttribute, t );
    }
};

//-------------------------------------------------
//JUIRelativeLayout
//-------------------------------------------------
class JUIRelativeLayout: public JUIView
{
private:
    static ATTRIBUTE_TYPE attributes_[];
    void Init();

protected:
    static std::map<std::string, int32_t> map_attributes_;
    std::vector<JUIView*> views_;
    virtual void Restore();

    JUIRelativeLayout( const bool b );
public:
    JUIRelativeLayout();
    virtual ~JUIRelativeLayout();

    void AddView( JUIView* view );

    template<typename T>
    bool SetAttribute( const char* strAttribute,
            const T t )
    {
        return JUIBase::SetAttribute( map_attributes_, strAttribute, t );
    }
};

//-------------------------------------------------
//JUIRadioGroup
//-------------------------------------------------
class JUIRadioGroup: public JUILinearLayout
{
private:
protected:
    virtual void Restore();

    JUIRadioGroup( const bool b );
public:
    JUIRadioGroup();
    virtual ~JUIRadioGroup();
};

//-------------------------------------------------
//JUISeekBar
//-------------------------------------------------
class JUISeekBar: public JUIView
{
private:
    static ATTRIBUTE_TYPE attributes_[];
    static std::map<std::string, int32_t> map_attributes_;
    std::function<void( jui_helper::JUIView*,
            const int32_t,
            const int32_t,
            const int32_t )> stop_tracking_callback_;
    std::function<void( jui_helper::JUIView*,
            const int32_t,
            const int32_t,
            const int32_t )> start_tracking_callback_;
    std::function<void( jui_helper::JUIView*,
            const int32_t,
            const int32_t,
            const int32_t )> progress_changed_callback_;
    int32_t current_progress_;
protected:
    virtual void Restore();
    virtual void DispatchEvent( const int32_t message,
            const int32_t param1,
            const int32_t param2 );
public:
    JUISeekBar();
    virtual ~JUISeekBar();
    bool SetCallback( const int32_t message,
            std::function<void( jui_helper::JUIView*,
                    const int32_t,
                    const int32_t,
                    const int32_t )> callback );

    template<typename T>
    bool SetAttribute( const char* strAttribute,
            const T t )
    {
        return JUIBase::SetAttribute( map_attributes_, strAttribute, t );
    }
};

//-------------------------------------------------
//JUITextView
//-------------------------------------------------
class JUITextView: public JUIView
{
private:
    static ATTRIBUTE_TYPE attributes_[];
    void Init();
protected:
    static std::map<std::string, int32_t> map_attributes_;
    virtual void Restore();

    JUITextView( const bool b );
public:
    JUITextView();
    JUITextView( const char* str );
    virtual ~JUITextView();

    template<typename T>
    bool SetAttribute( const char* strAttribute,
            const T t )
    {
        return JUIBase::SetAttribute( map_attributes_, strAttribute, t );
    }

    bool SetAttribute( const char* strAttribute,
            const char* str )
    {
        return JUIBase::SetAttribute( map_attributes_, strAttribute, str );
    }

    template<typename T, typename T2>
    bool SetAttribute( const char* strAttribute,
            T t,
            T2 t2 )
    {
        return JUIBase::SetAttribute( map_attributes_, strAttribute, t, t2 );
    }

    template<typename T, typename T2, typename T3, typename T4>
    bool SetAttribute( const char* strAttribute,
            T p1,
            T2 p2,
            T3 p3,
            T4 p4 )
    {
        return JUIBase::SetAttribute( map_attributes_, strAttribute, p1, p2, p3, p4 );
    }

    template<typename T>
    bool GetAttribute( const char* strAttribute,
            T& value )
    {
        return JUIView::GetAttribute( map_attributes_, strAttribute, value );
    }

};

//-------------------------------------------------
//JUIButton
//-------------------------------------------------
class JUIButton: public JUITextView
{
private:
    void Init();
    std::function<void( jui_helper::JUIView*,
            const int32_t )> onclick_callback_;
protected:
    JUIButton( const bool b );

    virtual void Restore();
    virtual void DispatchEvent( const int32_t message,
            const int32_t param1,
            const int32_t param2 );
public:
    JUIButton();
    JUIButton( const char* str );
    virtual ~JUIButton();

    bool SetCallback( std::function<void( jui_helper::JUIView*,
            const int32_t )> callback );
};

//-------------------------------------------------
//JUICompoundButton
//-------------------------------------------------
class JUICompoundButton: public JUIButton
{
private:
    static ATTRIBUTE_TYPE attributes_[];
    std::function<void( jui_helper::JUIView*,
            const bool )> checked_changed_callback_;
protected:
    static std::map<std::string, int32_t> map_attributes_;
    bool current_checked_;

    JUICompoundButton();
    virtual void Restore();
    virtual void DispatchEvent( const int32_t message,
            const int32_t param1,
            const int32_t param2 );
public:
    virtual ~JUICompoundButton();
    bool SetCallback( std::function<void( jui_helper::JUIView*,
            const bool )> callback );
    bool IsChecked()
    {
        return current_checked_;
    }

    template<typename T>
    bool SetAttribute( const char* strAttribute,
            const T t )
    {
        return JUIBase::SetAttribute( map_attributes_, strAttribute, t );
    }
};

//-------------------------------------------------
//JUICheckBox
//-------------------------------------------------
class JUICheckBox: public JUICompoundButton
{
private:
    void Init();
protected:
    virtual void Restore();
public:
    JUICheckBox();
    JUICheckBox( const char* str );
    virtual ~JUICheckBox();
};

//-------------------------------------------------
//JUISwitch
//-------------------------------------------------
class JUISwitch: public JUICompoundButton
{
private:
    static ATTRIBUTE_TYPE attributes_[];
    static std::map<std::string, int32_t> map_attributes_;

    void Init();
protected:
    virtual void Restore();
public:
    JUISwitch();
    JUISwitch( const char* str );
    virtual ~JUISwitch();

    template<typename T>
    bool SetAttribute( const char* strAttribute,
            const T t )
    {
        return JUIBase::SetAttribute( map_attributes_, strAttribute, t );
    }

    bool SetAttribute( const char* strAttribute,
            const char* str )
    {
        return JUIBase::SetAttribute( map_attributes_, strAttribute, str );
    }
};

//-------------------------------------------------
//JUIToggleButton
//-------------------------------------------------
class JUIToggleButton: public JUICompoundButton
{
private:
    static ATTRIBUTE_TYPE attributes_[];
    static std::map<std::string, int32_t> map_attributes_;

    void Init();
protected:
    virtual void Restore();
public:
    JUIToggleButton();
    JUIToggleButton( const char* strOn,
            const char* strOff );
    virtual ~JUIToggleButton();

    template<typename T>
    bool SetAttribute( const char* strAttribute,
            const T t )
    {
        return JUIBase::SetAttribute( map_attributes_, strAttribute, t );
    }

    bool SetAttribute( const char* strAttribute,
            const char* str )
    {
        return JUIBase::SetAttribute( map_attributes_, strAttribute, str );
    }
};
//-------------------------------------------------
//JUICheckedTextView
//-------------------------------------------------
class JUICheckedTextView: public JUICompoundButton
{
private:
    void Init();
protected:
    virtual void Restore();
public:
    JUICheckedTextView();
    JUICheckedTextView( const char* str );
    virtual ~JUICheckedTextView();
};

//-------------------------------------------------
//JUIRadioButton
//-------------------------------------------------
class JUIRadioButton: public JUICompoundButton
{
private:
    void Init();
protected:
    virtual void Restore();
public:
    JUIRadioButton();
    JUIRadioButton( const char* str );
    virtual ~JUIRadioButton();
};

/*
 * JUIWindow represents a popup window with relative layout.
 * An application can create JUIView based classes and add them to JUIWindow to show Java Widget over
 * native activity.
 */
class JUIWindow
{
    friend class JUIView;
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
private:
    JUIWindow();
    ~JUIWindow();
    JUIWindow( const JUIWindow& rhs );
    JUIWindow& operator=( const JUIWindow& rhs );

    ANativeActivity* activity_;
    jobject popupWindow_;

    std::vector<JUIView*> views_;

    bool suspended_;
    bool windowDestroyed_;

    jobject jni_helper_java_ref_;
    jclass jni_helper_java_class_;

    JUIDialog* dialog_;

    jobject CreateWidget( const char* strWidgetName,
            void* id );
    void CloseWidget( jobject obj );

    void SetDialog( JUIDialog* dialog )
    {
        dialog_ = dialog;
    }
public:
    /*
     * Retrieve the singleton object of the helper.
     * Static member of the class

     * Methods in the class are NOT designed as thread safe.
     */
    static JUIWindow* GetInstance();

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
    static void Init( ANativeActivity* activity,
            const char* helper_class_name = NULL );
    void Close();

    /*
     * Resume JUIWindow
     */
    void Resume( ANativeActivity* activity,
            const int32_t command );

    /*
     * Suspend JUIWindow
     */
    void Suspend( const int32_t command );

    /*
     * Add JUIView classes to the window
     */
    void AddView( JUIView* view );

    /*
     *
     */
    jobject GetContext()
    {
        if( activity_ == NULL )
            return NULL;
        return activity_->clazz;
    }
};

/*
 * JUIDialog represents a dialog framgent
 * An application can create JUIView based classes and add them to JUIDialog to show Java Widget over
 * native activity.
 */
class JUIDialog: public JUIBase
{
    friend class JUIView;
    friend class JUISeekBar;
    friend class JUITextView;
    friend class JUIButton;
    friend class JUICheckBox;
    friend class JUISwitch;
    friend class JUIToggleButton;
    friend class JUIRadioButton;
    friend class JUICheckedTextView;
    friend class JUILinearLayout;
    friend class JUIRadioGroup;
    friend class JUIToast;
    friend class JUIWindow;
private:
protected:
    static ATTRIBUTE_TYPE attributes_[];
    static std::map<std::string, int32_t> map_attributes_;

    ANativeActivity* activity_;
    std::vector<JUIView*> views_;
    bool suspended_;

    std::function<void( jui_helper::JUIDialog* dialog,
            const int32_t message )> dismiss_callback_;
    std::function<void( jui_helper::JUIDialog* dialog,
            const int32_t message )> cancel_callback_;

    void CreateDialog();
    void DeleteObject();
    virtual void Suspend();
    virtual void Resume( ANativeActivity* activity );
    void RestoreParameters( std::map<std::string, int32_t>& map );

public:
    JUIDialog();
    JUIDialog( ANativeActivity* activity );
    virtual ~JUIDialog();

    /*
     * Initialize window with an activity
     */
    void Init( ANativeActivity* activity );
    void Close();

    /*
     * Show dialog
     */
    void Show();

    /*
     * Add JUIView classes to the dialog
     */
    void AddView( JUIView* view );

    virtual void DispatchEvent( const int32_t message,
            const int32_t param1,
            const int32_t param2 );

    bool SetCallback( const int32_t message,
            std::function<void( jui_helper::JUIDialog* dialog,
                    const int32_t message )> callback );

    template<typename T>
    bool SetAttribute( const char* strAttribute,
            const T t )
    {
        return JUIBase::SetAttribute( map_attributes_, strAttribute, t );
    }

    bool SetAttribute( const char* strAttribute,
            const char* str )
    {
        return JUIBase::SetAttribute( map_attributes_, strAttribute, str );
    }

};

class JUIAlertDialog: public JUIDialog
{
private:
    static ATTRIBUTE_TYPE attributes_[];
    static std::map<std::string, int32_t> map_attributes_;

    void CreateDialog();

    std::function<void( jui_helper::JUIView*,
            const int32_t )> callback_positive_;
    std::function<void( jui_helper::JUIView*,
            const int32_t )> callback_negative_;
    std::function<void( jui_helper::JUIView*,
            const int32_t )> callback_neutral_;
protected:
    virtual void Resume( ANativeActivity* activity );
    virtual void DispatchEvent( const int32_t message,
            const int32_t param1,
            const int32_t param2 );
public:
    JUIAlertDialog( ANativeActivity* activity );
    virtual ~JUIAlertDialog();

    /*
     * Initialize window with an activity
     */
    void Init( ANativeActivity* activity );

    template<typename T>
    bool SetAttribute( const char* strAttribute,
            const T t )
    {
        return JUIBase::SetAttribute( map_attributes_, strAttribute, t );
    }

    bool SetAttribute( const char* strAttribute,
            const char* str )
    {
        return JUIBase::SetAttribute( map_attributes_, strAttribute, str );
    }

    bool SetButton( const int32_t button,
            const char* message,
            std::function<void( jui_helper::JUIView*,
                    const int32_t )> callback );
};

}   //namespace ndkHelper
#endif /* JAVAUI_H_ */
