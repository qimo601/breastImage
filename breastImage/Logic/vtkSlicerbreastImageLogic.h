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

// .NAME vtkSlicerbreastImageLogic - slicer logic class for volumes manipulation
// .SECTION Description
// This class manages the logic associated with reading, saving,
// and changing propertied of the volumes


#ifndef __vtkSlicerbreastImageLogic_h
#define __vtkSlicerbreastImageLogic_h

// Slicer includes
#include "vtkSlicerModuleLogic.h"
#include "vtkMRMLSubjectHierarchyNode.h"

// MRML includes

// QT includes
#include <QString>
#include <QMap>

// STD includes
#include <cstdlib>

#include "vtkSlicerbreastImageModuleLogicExport.h"

class vtkMRMLVolumeNode;
class vtkMRMLAnnotationROINode;

/// \ingroup Slicer_QtModules_ExtensionTemplate
class VTK_SLICER_BREASTIMAGE_MODULE_LOGIC_EXPORT vtkSlicerbreastImageLogic :
  public vtkSlicerModuleLogic
{
public:

  static vtkSlicerbreastImageLogic *New();
  vtkTypeMacro(vtkSlicerbreastImageLogic, vtkSlicerModuleLogic);
  void PrintSelf(ostream& os, vtkIndent indent);
  QMap< QString, QString> GetNodeAttribute(vtkMRMLNode *node);
  void coordinatesTransform(vtkMRMLVolumeNode* inputVolume);
  void acquireRoiLocation(vtkMRMLVolumeNode* inputVolume, vtkMRMLAnnotationROINode* inputROI);
  void writeAnnotationXML(QString dir,QString fileName, QMap<QString, QString> m_dicomInf, QMap<QString, QString> m_pacasInf, QMap<QString, QString> m_annotationInf);
  void readAnnotationXML(QString fileName, QMap<QString, QString> &m_dicomInf, QMap<QString, QString> &m_pacasInf, QMap<QString, QString> &m_annotationInf);

  //ijk
  int roiXYZIJK[3];
  int roiRadiusIJK[3];
protected:
  vtkSlicerbreastImageLogic();
  virtual ~vtkSlicerbreastImageLogic();

  virtual void SetMRMLSceneInternal(vtkMRMLScene* newScene);
  /// Register MRML Node classes to Scene. Gets called automatically when the MRMLScene is attached to this logic class.
  virtual void RegisterNodes();
  virtual void UpdateFromMRMLScene();
  virtual void OnMRMLSceneNodeAdded(vtkMRMLNode* node);
  virtual void OnMRMLSceneNodeRemoved(vtkMRMLNode* node);
private:

  vtkSlicerbreastImageLogic(const vtkSlicerbreastImageLogic&); // Not implemented
  void operator=(const vtkSlicerbreastImageLogic&); // Not implemented
};

#endif
