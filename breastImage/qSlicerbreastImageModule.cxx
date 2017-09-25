/*==============================================================================

  Program: 3D Slicer

  Portions (c) Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

// Qt includes
#include <QtPlugin>

// breastImage Logic includes
#include <vtkSlicerbreastImageLogic.h>

// breastImage includes
#include "qSlicerbreastImageModule.h"
#include "qSlicerbreastImageModuleWidget.h"

//-----------------------------------------------------------------------------
Q_EXPORT_PLUGIN2(qSlicerbreastImageModule, qSlicerbreastImageModule);

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_ExtensionTemplate
class qSlicerbreastImageModulePrivate
{
public:
  qSlicerbreastImageModulePrivate();
};

//-----------------------------------------------------------------------------
// qSlicerbreastImageModulePrivate methods

//-----------------------------------------------------------------------------
qSlicerbreastImageModulePrivate::qSlicerbreastImageModulePrivate()
{
}

//-----------------------------------------------------------------------------
// qSlicerbreastImageModule methods

//-----------------------------------------------------------------------------
qSlicerbreastImageModule::qSlicerbreastImageModule(QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerbreastImageModulePrivate)
{
}

//-----------------------------------------------------------------------------
qSlicerbreastImageModule::~qSlicerbreastImageModule()
{
}

//-----------------------------------------------------------------------------
QString qSlicerbreastImageModule::helpText() const
{
  return "This is a loadable module that can be bundled in an extension";
}

//-----------------------------------------------------------------------------
QString qSlicerbreastImageModule::acknowledgementText() const
{
  return "This work was partially funded by NIH grant NXNNXXNNNNNN-NNXN";
}

//-----------------------------------------------------------------------------
QStringList qSlicerbreastImageModule::contributors() const
{
  QStringList moduleContributors;
  moduleContributors << QString("John Doe (AnyWare Corp.)");
  return moduleContributors;
}

//-----------------------------------------------------------------------------
QIcon qSlicerbreastImageModule::icon() const
{
  return QIcon(":/Icons/breastImage.png");
}

//-----------------------------------------------------------------------------
QStringList qSlicerbreastImageModule::categories() const
{
  return QStringList() << "Examples";
}

//-----------------------------------------------------------------------------
QStringList qSlicerbreastImageModule::dependencies() const
{
  return QStringList();
}

//-----------------------------------------------------------------------------
void qSlicerbreastImageModule::setup()
{
  this->Superclass::setup();
}

//-----------------------------------------------------------------------------
qSlicerAbstractModuleRepresentation* qSlicerbreastImageModule
::createWidgetRepresentation()
{
  return new qSlicerbreastImageModuleWidget;
}

//-----------------------------------------------------------------------------
vtkMRMLAbstractLogic* qSlicerbreastImageModule::createLogic()
{
  return vtkSlicerbreastImageLogic::New();
}
