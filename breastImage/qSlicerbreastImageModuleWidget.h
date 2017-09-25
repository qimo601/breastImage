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

#ifndef __qSlicerbreastImageModuleWidget_h
#define __qSlicerbreastImageModuleWidget_h

// SlicerQt includes
#include "qSlicerAbstractModuleWidget.h"

//QT includes
#include <QString>
#include <QMap>

#include "qSlicerbreastImageModuleExport.h"

class qSlicerbreastImageModuleWidgetPrivate;
class vtkMRMLAnnotationROINode;
class vtkMRMLVolumeNode;
class vtkMRMLNode;

/// \ingroup Slicer_QtModules_ExtensionTemplate
class Q_SLICER_QTMODULES_BREASTIMAGE_EXPORT qSlicerbreastImageModuleWidget :
  public qSlicerAbstractModuleWidget
{
  Q_OBJECT

public:

  typedef qSlicerAbstractModuleWidget Superclass;
  qSlicerbreastImageModuleWidget(QWidget *parent=0);
  virtual ~qSlicerbreastImageModuleWidget();
  void init();
  QMap< QString, QString> m_dicomInf;
  QMap< QString, QString> m_pacasInf;
  QMap< QString, QString> m_AnnotationInf;
  QMap<QString, QString> t_dicomInf;
  QMap<QString, QString> t_pacasInf;
  QMap<QString, QString> t_annotationInf;
  int m_number;
  int m_index;
  bool m_readMode;
  QString m_view;
  QString m_modality;

public slots:
  void on_addButton_clicked();
  void on_deleteButton_clicked();
  void on_importXMLButton_clicked();
  void on_outputXMLButton_clicked();
  void on_refreshRoiButton_clicked();
  void on_refreshRulerButton_clicked();
  void on_transformButton_clicked();

protected:
  QScopedPointer<qSlicerbreastImageModuleWidgetPrivate> d_ptr;
  virtual void setup();
  virtual void enter();
  virtual void setMRMLScene(vtkMRMLScene*);
  void updateVolume(vtkMRMLVolumeNode* inputVolumeNode);

protected slots:
  void onInputNodeChanged();
  void onPacasInfChanged();
  void onEditInfTableChanged();
  void initializeNode(vtkMRMLNode*);
  void onInputVolumeAdded(vtkMRMLNode*);
  void onInputROIChanged();
  void onInputROIAdded(vtkMRMLNode*);
  void onInputRulerChanged();
  void onInputRulerAdded(vtkMRMLNode*);

private:
  Q_DECLARE_PRIVATE(qSlicerbreastImageModuleWidget);
  Q_DISABLE_COPY(qSlicerbreastImageModuleWidget);
};

#endif
