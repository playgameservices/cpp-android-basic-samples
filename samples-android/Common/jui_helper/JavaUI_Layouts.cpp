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
 * JUILinearLayout
 */
std::unordered_map<std::string, int32_t> JUILinearLayout::map_attributes_;
const AttributeType JUILinearLayout::attributes_[] = {
  { "BaselineAligned", ATTRIBUTE_PARAMETER_BOOLEAN },
  { "BaselineAlignedChildIndex", ATTRIBUTE_PARAMETER_INT },
  { "DividerPadding", ATTRIBUTE_PARAMETER_INT },
  { "Gravity", ATTRIBUTE_PARAMETER_INT },
  { "HorizontalGravity", ATTRIBUTE_PARAMETER_INT },
  { "MeasureWithLargestChildEnabled", ATTRIBUTE_PARAMETER_BOOLEAN },
  { "Orientation", ATTRIBUTE_PARAMETER_INT },
  { "ShowDividers", ATTRIBUTE_PARAMETER_INT },
  { "VerticalGravity", ATTRIBUTE_PARAMETER_INT },
  { "WeightSum", ATTRIBUTE_PARAMETER_FLOAT },
};

JUILinearLayout::JUILinearLayout() : JUIView() {
  obj_ = JUIWindow::GetInstance()->CreateWidget("JUILinearLayout", this);
  if (obj_ == NULL)
    LOGI("Class initialization failure");

  Init();
}

JUILinearLayout::JUILinearLayout(const bool b) : JUIView() {
  if (b == true)
    JUILinearLayout();
  else
    Init();
}

void JUILinearLayout::Init() {
  //setup attribute map (once)
  if (map_attributes_.size() == 0) {
    //Add base class's map
    map_attributes_.insert(JUIView::map_attributes_.begin(),
                           JUIView::map_attributes_.end());

    for (int32_t i = 0; i < sizeof(attributes_) / sizeof(attributes_[0]); ++i) {
      map_attributes_[std::string(attributes_[i].attribute_name)] =
          attributes_[i].attribute_type;
    }
  }
}

JUILinearLayout::~JUILinearLayout() {
  auto itBegin = views_.begin();
  auto itEnd = views_.end();
  while (itBegin != itEnd) {
    delete *itBegin;
    itBegin++;
  }

  views_.clear();

  JUIWindow::GetInstance()->CloseWidget(obj_);
}

void JUILinearLayout::Restore() {
  //Recreate Java Widget when the activity has been disposed
  obj_ = JUIWindow::GetInstance()->CreateWidget("JUILinearLayout", this);
  if (obj_ == NULL)
    LOGI("Class initialization failure");

  RestoreParameters(map_attributes_);

  //Restore widgets
  auto itBegin = views_.begin();
  auto itEnd = views_.end();
  while (itBegin != itEnd) {
    //Restore
    (*itBegin)->Restore();
    ndk_helper::JNIHelper::GetInstance()->CallVoidMethod(
        JUIWindow::GetHelperClassInstance(), "addView",
        "(Landroid/view/ViewGroup;Landroid/view/View;)V", GetJobject(),
        (*itBegin)->GetJobject());

    itBegin++;
  }

}

/*
 * Add JUIView to the layout
 */
void JUILinearLayout::AddView(JUIView *view) {
  ndk_helper::JNIHelper::GetInstance()->CallVoidMethod(
      JUIWindow::GetHelperClassInstance(), "addView",
      "(Landroid/view/ViewGroup;Landroid/view/View;)V", GetJobject(),
      view->GetJobject());
  views_.push_back(view);
}

/*
 * JUIRadioGroup
 */
JUIRadioGroup::JUIRadioGroup() : JUILinearLayout(false) {
  obj_ = JUIWindow::GetInstance()->CreateWidget("JUIRadioGroup", this);
  if (obj_ == NULL)
    LOGI("Class initialization failure");

}

JUIRadioGroup::~JUIRadioGroup() { JUIWindow::GetInstance()->CloseWidget(obj_); }

void JUIRadioGroup::Restore() {
  //Recreate Java Widget when the activity has been disposed
  obj_ = JUIWindow::GetInstance()->CreateWidget("JUIRadioGroup", this);
  if (obj_ == NULL)
    LOGI("Class initialization failure");

  RestoreParameters(map_attributes_);

  //Restore widgets
  auto itBegin = views_.begin();
  auto itEnd = views_.end();
  while (itBegin != itEnd) {
    //Restore
    (*itBegin)->Restore();
    ndk_helper::JNIHelper::GetInstance()->CallVoidMethod(
        JUIWindow::GetHelperClassInstance(), "addView",
        "(Landroid/view/ViewGroup;Landroid/view/View;)V", GetJobject(),
        (*itBegin)->GetJobject());

    itBegin++;
  }
}

/*
 * JUIRelativeLayout
 */
std::unordered_map<std::string, int32_t> JUIRelativeLayout::map_attributes_;
const AttributeType JUIRelativeLayout::attributes_[] = {
  { "Gravity", ATTRIBUTE_PARAMETER_INT },
  { "HorizontalGravity", ATTRIBUTE_PARAMETER_INT },
  { "IgnoreGravity", ATTRIBUTE_PARAMETER_INT },
  { "VerticalGravity", ATTRIBUTE_PARAMETER_INT },
};

JUIRelativeLayout::JUIRelativeLayout() : JUIView() {
  obj_ = JUIWindow::GetInstance()->CreateWidget("JUIRelativeLayout", this);
  if (obj_ == NULL)
    LOGI("Class initialization failure");

  Init();
}

JUIRelativeLayout::JUIRelativeLayout(const bool b) : JUIView() {
  if (b == true)
    JUIRelativeLayout();
  else
    Init();
}

void JUIRelativeLayout::Init() {
  //setup attribute map (once)
  if (map_attributes_.size() == 0) {
    //Add base class's map
    map_attributes_.insert(JUIView::map_attributes_.begin(),
                           JUIView::map_attributes_.end());

    for (int32_t i = 0; i < sizeof(attributes_) / sizeof(attributes_[0]); ++i) {
      map_attributes_[std::string(attributes_[i].attribute_name)] =
          attributes_[i].attribute_type;
    }
  }
}

JUIRelativeLayout::~JUIRelativeLayout() {
  auto itBegin = views_.begin();
  auto itEnd = views_.end();
  while (itBegin != itEnd) {
    delete *itBegin;
    itBegin++;
  }

  views_.clear();

  JUIWindow::GetInstance()->CloseWidget(obj_);
}

void JUIRelativeLayout::Restore() {
  //Recreate Java Widget when the activity has been disposed
  obj_ = JUIWindow::GetInstance()->CreateWidget("JUIRelativeLayout", this);
  if (obj_ == NULL)
    LOGI("Class initialization failure");

  RestoreParameters(map_attributes_);

  //Restore widgets
  auto itBegin = views_.begin();
  auto itEnd = views_.end();
  while (itBegin != itEnd) {
    //Restore
    (*itBegin)->Restore();
    ndk_helper::JNIHelper::GetInstance()->CallVoidMethod(
        JUIWindow::GetHelperClassInstance(), "addView",
        "(Landroid/view/ViewGroup;Landroid/view/View;)V", GetJobject(),
        (*itBegin)->GetJobject());

    itBegin++;
  }
}

/*
 * Add JUIView to the layout
 */
void JUIRelativeLayout::AddView(JUIView *view) {
  ndk_helper::JNIHelper::GetInstance()->CallVoidMethod(
      JUIWindow::GetHelperClassInstance(), "addView",
      "(Landroid/view/ViewGroup;Landroid/view/View;)V", GetJobject(),
      view->GetJobject());
  views_.push_back(view);
}

} //namespace jui_helper
