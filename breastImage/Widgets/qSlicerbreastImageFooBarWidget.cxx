/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// FooBar Widgets includes
#include "qSlicerbreastImageFooBarWidget.h"
#include "ui_qSlicerbreastImageFooBarWidget.h"

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_breastImage
class qSlicerbreastImageFooBarWidgetPrivate
  : public Ui_qSlicerbreastImageFooBarWidget
{
  Q_DECLARE_PUBLIC(qSlicerbreastImageFooBarWidget);
protected:
  qSlicerbreastImageFooBarWidget* const q_ptr;

public:
  qSlicerbreastImageFooBarWidgetPrivate(
    qSlicerbreastImageFooBarWidget& object);
  virtual void setupUi(qSlicerbreastImageFooBarWidget*);
};

// --------------------------------------------------------------------------
qSlicerbreastImageFooBarWidgetPrivate
::qSlicerbreastImageFooBarWidgetPrivate(
  qSlicerbreastImageFooBarWidget& object)
  : q_ptr(&object)
{
}

// --------------------------------------------------------------------------
void qSlicerbreastImageFooBarWidgetPrivate
::setupUi(qSlicerbreastImageFooBarWidget* widget)
{
  this->Ui_qSlicerbreastImageFooBarWidget::setupUi(widget);
}

//-----------------------------------------------------------------------------
// qSlicerbreastImageFooBarWidget methods

//-----------------------------------------------------------------------------
qSlicerbreastImageFooBarWidget
::qSlicerbreastImageFooBarWidget(QWidget* parentWidget)
  : Superclass( parentWidget )
  , d_ptr( new qSlicerbreastImageFooBarWidgetPrivate(*this) )
{
  Q_D(qSlicerbreastImageFooBarWidget);
  d->setupUi(this);
}

//-----------------------------------------------------------------------------
qSlicerbreastImageFooBarWidget
::~qSlicerbreastImageFooBarWidget()
{
}
