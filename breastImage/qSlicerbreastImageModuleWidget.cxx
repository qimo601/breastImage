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
#include <QDebug>
#include <QCoreApplication>
#include <QMessageBox>
#include <QFileDialog>

//QT GUI includes
#include <QtGui/QComboBox>
#include <QtGui/QTableWidget>
#include <QtGui/QSpinBox>
#include <QDateTime>

//vtk includes
#include "vtkMRMLScene.h"
#include "vtkImageData.h"
#include <vtkNew.h>
#include <vtkMatrix4x4.h>
#include <vtkSmartPointer.h>

//vtkSlicerbreastImageLogic includes
#include "vtkSlicerbreastImageLogic.h"

// MRMLLogic includes
#include <vtkMRMLApplicationLogic.h>

// SlicerQt includes
#include "qSlicerbreastImageModuleWidget.h"
#include <qSlicerAbstractCoreModule.h>
#include "ui_qSlicerbreastImageModuleWidget.h"

//slicer node includes
#include <vtkMRMLSubjectHierarchyNode.h>
#include <vtkMRMLNode.h>
#include <vtkMRMLVolumeNode.h>
#include <vtkMRMLAnnotationROINode.h>
#include <vtkMRMLAnnotationRulerNode.h>
#include <vtkMRMLSelectionNode.h>

// qMRML includes
#include <qMRMLNodeFactory.h>

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_ExtensionTemplate
class qSlicerbreastImageModuleWidgetPrivate: public Ui_qSlicerbreastImageModuleWidget
{
	Q_DECLARE_PUBLIC(qSlicerbreastImageModuleWidget);
public:
	qSlicerbreastImageModuleWidgetPrivate(qSlicerbreastImageModuleWidget& object);
    vtkSlicerbreastImageLogic* logic() const;

protected:
	qSlicerbreastImageModuleWidget* const q_ptr;
};

//-----------------------------------------------------------------------------
// qSlicerbreastImageModuleWidgetPrivate methods

//-----------------------------------------------------------------------------
qSlicerbreastImageModuleWidgetPrivate::qSlicerbreastImageModuleWidgetPrivate(qSlicerbreastImageModuleWidget& object) : q_ptr(&object)
{

}

//-----------------------------------------------------------------------------
vtkSlicerbreastImageLogic* qSlicerbreastImageModuleWidgetPrivate::logic() const
{
	Q_Q(const qSlicerbreastImageModuleWidget);
	return vtkSlicerbreastImageLogic::SafeDownCast(q->logic());
}

//-----------------------------------------------------------------------------
// qSlicerbreastImageModuleWidget methods

//-----------------------------------------------------------------------------
qSlicerbreastImageModuleWidget::qSlicerbreastImageModuleWidget(QWidget* _parent)
  : Superclass( _parent )
  , d_ptr(new qSlicerbreastImageModuleWidgetPrivate(*this))
{

}

//-----------------------------------------------------------------------------
qSlicerbreastImageModuleWidget::~qSlicerbreastImageModuleWidget()
{

}

//-----------------------------------------------------------------------------
void qSlicerbreastImageModuleWidget::setup()
{
  Q_D(qSlicerbreastImageModuleWidget);
  d->setupUi(this);
  this->Superclass::setup();
  QObject::connect(this, SIGNAL(mrmlSceneChanged(vtkMRMLScene*)), d->inputPatientNodeComboBox, SLOT(setMRMLScene(vtkMRMLScene*)));
  QObject::connect(this, SIGNAL(mrmlSceneChanged(vtkMRMLScene*)), d->inputStudyNodeComboBox, SLOT(setMRMLScene(vtkMRMLScene*)));
  QObject::connect(this, SIGNAL(mrmlSceneChanged(vtkMRMLScene*)), d->inputImageNodeComboBox, SLOT(setMRMLScene(vtkMRMLScene*)));
  QObject::connect(this, SIGNAL(mrmlSceneChanged(vtkMRMLScene*)), d->inputEditVolumeNodeComboBox, SLOT(setMRMLScene(vtkMRMLScene*)));
  QObject::connect(this, SIGNAL(mrmlSceneChanged(vtkMRMLScene*)), d->inputEditROINodeComboBox, SLOT(setMRMLScene(vtkMRMLScene*)));
  QObject::connect(d->inputEditROINodeComboBox->nodeFactory(), SIGNAL(nodeInitialized(vtkMRMLNode*)), this, SLOT(initializeNode(vtkMRMLNode*)));
  QObject::connect(d->inputEditRulerNodeComboBox->nodeFactory(), SIGNAL(nodeInitialized(vtkMRMLNode*)), this, SLOT(initializeNode(vtkMRMLNode*)));
  QObject::connect(this, SIGNAL(mrmlSceneChanged(vtkMRMLScene*)), d->inputEditRulerNodeComboBox, SLOT(setMRMLScene(vtkMRMLScene*)));
  QObject::connect(d->inputPatientNodeComboBox, SIGNAL(currentNodeChanged(vtkMRMLNode*)), this, SLOT(onInputNodeChanged()));
  QObject::connect(d->inputStudyNodeComboBox, SIGNAL(currentNodeChanged(vtkMRMLNode*)), this, SLOT(onInputNodeChanged()));
  QObject::connect(d->inputImageNodeComboBox, SIGNAL(currentNodeChanged(vtkMRMLNode*)), this, SLOT(onInputNodeChanged()));
  QObject::connect(d->inputEditVolumeNodeComboBox, SIGNAL(currentNodeChanged(vtkMRMLNode*)), this, SLOT(onInputNodeChanged()));
  QObject::connect(d->inputEditVolumeNodeComboBox, SIGNAL(nodeAdded(vtkMRMLNode*)), this, SLOT(onInputVolumeAdded(vtkMRMLNode*)));
  QObject::connect(d->inputEditROINodeComboBox, SIGNAL(currentNodeChanged(vtkMRMLNode*)), this, SLOT(onInputROIChanged()));
  QObject::connect(d->inputEditROINodeComboBox, SIGNAL(nodeAdded(vtkMRMLNode*)), this, SLOT(onInputROIAdded(vtkMRMLNode*)));
  QObject::connect(d->inputEditRulerNodeComboBox, SIGNAL(currentNodeChanged(vtkMRMLNode*)), this, SLOT(onInputRulerChanged()));
  QObject::connect(d->inputEditRulerNodeComboBox, SIGNAL(nodeAdded(vtkMRMLNode*)), this, SLOT(onInputRulerAdded(vtkMRMLNode*)));


  this->init();
}

//-----------------------------------------------------------------------------
void qSlicerbreastImageModuleWidget::enter()
{
	this->onInputNodeChanged();
	this->onInputROIChanged();
	this->onInputRulerChanged();
	this->onPacasInfChanged();
	this->onEditInfTableChanged();
	this->Superclass::enter();
}

//-----------------------------------------------------------------------------
void qSlicerbreastImageModuleWidget::setMRMLScene(vtkMRMLScene* scene)
{
	this->Superclass::setMRMLScene(scene);
	if (scene == NULL)
	{
		return;
	}

	// observe close event so can re-add a parameters node if necessary
	qvtkReconnect(this->mrmlScene(), vtkMRMLScene::EndCloseEvent,
		this, SLOT(onEndCloseEvent()));
}

//-----------------------------------------------------------------------------
void qSlicerbreastImageModuleWidget::initializeNode(vtkMRMLNode *n)
{
	vtkMRMLScene* scene = qobject_cast<qMRMLNodeFactory*>(this->sender())->mrmlScene();
	vtkMRMLAnnotationROINode::SafeDownCast(n)->Initialize(scene);
	vtkMRMLAnnotationRulerNode::SafeDownCast(n)->Initialize(scene);
}

//-----------------------------------------------------------------------------
void qSlicerbreastImageModuleWidget::onInputNodeChanged()
{
	Q_D(const qSlicerbreastImageModuleWidget);
	vtkSlicerbreastImageLogic *logic = d->logic();
	vtkSmartPointer<vtkMRMLNode> inputPatientNode = vtkMRMLNode::SafeDownCast(d->inputPatientNodeComboBox->currentNode());
	vtkSmartPointer<vtkMRMLNode> inputStudyNode = vtkMRMLNode::SafeDownCast(d->inputStudyNodeComboBox->currentNode());
	vtkSmartPointer<vtkMRMLNode> inputImageNode = vtkMRMLNode::SafeDownCast(d->inputImageNodeComboBox->currentNode());
	vtkSmartPointer<vtkMRMLVolumeNode> inputVolumeNode = vtkMRMLVolumeNode::SafeDownCast(d->inputEditVolumeNodeComboBox->currentNode());
	if (inputPatientNode)//Dicom patient information
	{
		QMap< QString, QString> m_map;
		QString attributeValue;
		m_map = logic->GetNodeAttribute(inputPatientNode);
		attributeValue = m_map.value("DICOM.PatientID");
		m_dicomInf["PatientID"]= attributeValue;
		d->patientInfTableWidget->setItem(0, 0, new QTableWidgetItem(attributeValue));
		attributeValue = m_map.value("DICOM.PatientBirthDate");
		m_dicomInf["PatientBirthDate"] = attributeValue;
		d->patientInfTableWidget->setItem(1, 0, new QTableWidgetItem(attributeValue));
	}
	if (inputStudyNode)//Dicom study information
	{
		QMap< QString, QString> m_map;
		QString attributeValue;
		m_map = logic->GetNodeAttribute(inputStudyNode);
		attributeValue = m_map.value("StudyID");
		m_dicomInf["StudyID"] = attributeValue;
		d->patientInfTableWidget->setItem(2, 0, new QTableWidgetItem(attributeValue));
		attributeValue = m_map.value("DICOM.StudyDate");
		m_dicomInf["StudyDate"] = attributeValue;
		d->patientInfTableWidget->setItem(3, 0, new QTableWidgetItem(attributeValue));
	}
	if (inputImageNode)//Dicom series information
	{
		QMap< QString, QString> m_map;
		QString attributeValue;
		m_map = logic->GetNodeAttribute(inputImageNode);
		attributeValue = m_map.value("DICOM.SeriesNumber");
		m_dicomInf["SeriesNumber"] = attributeValue;
		d->patientInfTableWidget->setItem(4, 0, new QTableWidgetItem(attributeValue));
		
		if (inputVolumeNode)//image information
		{
			QString nodeName = inputVolumeNode->GetName();
			if (nodeName == QString(attributeValue+": L MLO"))
			{
				m_view = "L_MLO";
				//m_modality = "2D";
			}
			else if (nodeName == QString(attributeValue + ": L MLO Breast Tomosynthesis Image"))
			{
				m_view = "L_MLO";
				//m_modality = "3D";
			}
			else if (nodeName == QString(attributeValue + ": L CC"))
			{
				m_view = "L_CC";
				//m_modality = "2D";
			}
			else if (nodeName == QString(attributeValue + ": L CC Breast Tomosynthesis Image"))
			{
				m_view = "L_CC";
				//m_modality = "3D";
			}
			else if (nodeName == QString(attributeValue + ": R CC"))
			{
				m_view = "R_CC";
				//m_modality = "2D";
			}
			else if (nodeName == QString(attributeValue + ": R CC Breast Tomosynthesis Image"))
			{
				m_view = "R_CC";
				//m_modality = "3D";
			}
			else if (nodeName == QString(attributeValue + ": R MLO"))
			{
				m_view = "R_MLO";
				//m_modality = "2D";
			}
			else if (nodeName == QString(attributeValue + ": R CC Breast Tomosynthesis Image"))
			{
				m_view = "R_MLO";
				//m_modality = "3D";
			}
			else
			{
				m_view = "R_MLO";
				//m_modality = "2D";
			}
			m_dicomInf["View"] = m_view;
			d->imageInfTableWidget->setItem(0, 0, new QTableWidgetItem(m_view));

			vtkImageData* image = inputVolumeNode->GetImageData();
			// get the dimensions of input volume
			int dimensions[3];
			QString imageSize;
			image->GetDimensions(dimensions);
			imageSize = QString::number(dimensions[0], 10);
			m_dicomInf["imageRow"] = imageSize;
			imageSize = QString::number(dimensions[1], 10);
			m_dicomInf["imageColumn"] = imageSize;
			imageSize = QString::number(dimensions[2], 10);
			m_dicomInf["imageSlice"] = imageSize;
			attributeValue = QString::number(dimensions[0], 10) +"-"+ QString::number(dimensions[1], 10) +"-"+ QString::number(dimensions[2], 10);
			d->imageInfTableWidget->setItem(2, 0, new QTableWidgetItem(attributeValue));

			//get the spaceing of input volume
			double spaceing[3];
			inputVolumeNode->GetSpacing(spaceing);
			QString imageSpaceingSize;
			imageSpaceingSize = QString::number(spaceing[0], 10, 4);
			m_dicomInf["imageRowSpaceing"] = imageSpaceingSize;
			imageSpaceingSize = QString::number(spaceing[1], 10, 4);
			m_dicomInf["imageColumnSpaceing"] = imageSpaceingSize;
			imageSpaceingSize = QString::number(spaceing[2], 10, 4);
			m_dicomInf["imageSliceSpaceing"] = imageSpaceingSize;
			attributeValue = QString::number(spaceing[0], 10, 4) + "-" + QString::number(spaceing[1], 10, 4) + "-" + QString::number(spaceing[2], 10, 4);
			d->imageInfTableWidget->setItem(3, 0, new QTableWidgetItem(attributeValue));
			// populate Scalar Types
			QComboBox *scalarType = new QComboBox();
			for (int i = VTK_VOID; i < VTK_OBJECT; ++i)
			{
				scalarType->addItem(vtkImageScalarTypeNameMacro(i), i);
			}
			int type=image->GetScalarType();
			scalarType->setCurrentIndex(type);
			m_dicomInf["scalarType"] = scalarType->currentText();
			d->imageInfTableWidget->setCellWidget(4, 0, scalarType);
			double origin[3];
			if (dimensions[2] > 1)
			{
				m_modality = "3D";
				origin[0] = origin[1] = 0;
				origin[2] = int(dimensions[2] / 2);
				origin[2] = -origin[2];
				inputVolumeNode->SetOrigin(origin);
			}
			else
			{
				m_modality = "2D";
				origin[0] = origin[1] = origin[2] = 0;
				inputVolumeNode->SetOrigin(origin);
			}
			m_dicomInf["Modality"] = m_modality;
			d->imageInfTableWidget->setItem(1, 0, new QTableWidgetItem(m_modality));
			this->updateVolume(inputVolumeNode);
		}
	}
}

void qSlicerbreastImageModuleWidget::updateVolume(vtkMRMLVolumeNode* inputVolumeNode)
{
	Q_D(qSlicerbreastImageModuleWidget);
	if (this->mrmlScene() && !this->mrmlScene()->IsClosing() && !this->mrmlScene()->IsBatchProcessing())
	{
		//set it to be active in the slice windows
		vtkSlicerApplicationLogic *appLogic = this->module()->appLogic();
		vtkMRMLSelectionNode *selectionNode = appLogic->GetSelectionNode();
		selectionNode->SetReferenceActiveVolumeID(inputVolumeNode->GetID());
		appLogic->PropagateVolumeSelection();
	}
}

//-----------------------------------------------------------------------------
void qSlicerbreastImageModuleWidget::onInputVolumeAdded(vtkMRMLNode *mrmlNode)
{
	Q_D(qSlicerbreastImageModuleWidget);
	if (!mrmlNode)
	{
		return;
	}
	if (d->inputEditVolumeNodeComboBox->currentNode() != NULL)
	{
		// there's already a selected node, don't reset it
		return;
	}
	d->inputEditVolumeNodeComboBox->setCurrentNode(mrmlNode);
}

void qSlicerbreastImageModuleWidget::onInputROIChanged()
{
	Q_D(const qSlicerbreastImageModuleWidget);
	vtkSmartPointer<vtkMRMLAnnotationROINode> inputAnnotationRoiNode = vtkMRMLAnnotationROINode::SafeDownCast(d->inputEditROINodeComboBox->currentNode());
	if (inputAnnotationRoiNode)
	{
		inputAnnotationRoiNode->SetDisplayVisibility(true);
	}
}

void qSlicerbreastImageModuleWidget::onInputRulerChanged()
{
	Q_D(const qSlicerbreastImageModuleWidget);
	vtkSmartPointer<vtkMRMLAnnotationRulerNode> inputAnnotationRulerNode = vtkMRMLAnnotationRulerNode::SafeDownCast(d->inputEditRulerNodeComboBox->currentNode());
	if (inputAnnotationRulerNode)
	{
		inputAnnotationRulerNode->SetDisplayVisibility(true);
	}
}

void qSlicerbreastImageModuleWidget::onInputROIAdded(vtkMRMLNode *mrmlNode)
{
	Q_D(qSlicerbreastImageModuleWidget);
	if (!mrmlNode)
	{
		return;
	}
	if (d->inputEditROINodeComboBox->currentNode() != NULL)
	{
		// there's already a selected node, don't reset it
		return;
	}
	d->inputEditROINodeComboBox->setCurrentNode(mrmlNode);
}

void qSlicerbreastImageModuleWidget::onInputRulerAdded(vtkMRMLNode *mrmlNode)
{
	Q_D(qSlicerbreastImageModuleWidget);
	if (!mrmlNode)
	{
		return;
	}
	if (d->inputEditRulerNodeComboBox->currentNode() != NULL)
	{
		// there's already a selected node, don't reset it
		return;
	}
	d->inputEditRulerNodeComboBox->setCurrentNode(mrmlNode);
}

void qSlicerbreastImageModuleWidget::on_transformButton_clicked()
{
	Q_D(const qSlicerbreastImageModuleWidget);
	//vtkSlicerbreastImageLogic *logic = d->logic();
	vtkSmartPointer<vtkMRMLVolumeNode> inputVolumeNode = vtkMRMLVolumeNode::SafeDownCast(d->inputEditVolumeNodeComboBox->currentNode());
	//logic->coordinatesTransform(inputVolumeNode);
	if (!inputVolumeNode)
	{
		return;
	}

	vtkSmartPointer<vtkImageData> imageData = inputVolumeNode->GetImageData();
	vtkNew<vtkImageData> imageDataWorkingCopy;
	imageDataWorkingCopy->DeepCopy(imageData);
	int* dims = imageData->GetDimensions();
	for (int k = 0; k < dims[2]; k++)
	{
		for (int j = 0; j < dims[1]; j++)
		{
			for (int i = 0; i < dims[0]; i++)
			{
				int a, b;
				a = -i + dims[0] - 1;
				b = -j + dims[1] - 1;
				unsigned short* p = (unsigned short*)(imageDataWorkingCopy->GetScalarPointer(a, b, k));
				unsigned short value = *p;
				unsigned short* q = (unsigned short*)(imageData->GetScalarPointer(i, j, k));
				*q = value;
				d->transformProgressBar->setValue(k * 100 / (dims[2]-1));
			}

		}

	}
	double spaceing[3];
	spaceing[0] = 0.07 * 3328 / 1996;
	spaceing[1] = 0.07 * 3328 / 1996;
	spaceing[2] = 1;
	QString imageSpaceingSize;
	imageSpaceingSize = QString::number(spaceing[0], 10, 4);
	m_dicomInf["imageRowSpaceing"] = imageSpaceingSize;
	imageSpaceingSize = QString::number(spaceing[1], 10, 4);
	m_dicomInf["imageColumnSpaceing"] = imageSpaceingSize;
	imageSpaceingSize = QString::number(spaceing[2], 10, 4);
	m_dicomInf["imageSliceSpaceing"] = imageSpaceingSize;
	inputVolumeNode->SetSpacing(spaceing);
	inputVolumeNode->Modified();
	this->updateVolume(inputVolumeNode);
}
void qSlicerbreastImageModuleWidget::init()
{
	Q_D(const qSlicerbreastImageModuleWidget);
	//init editInfWidget
	QSpinBox *number = new QSpinBox();
	d->editInfTableWidget->setCellWidget(0, 0, number);

	QDoubleSpinBox *size = new QDoubleSpinBox();
	size->setSuffix("mm");
	d->editInfTableWidget->setCellWidget(0, 1, size);

	QComboBox *shape = new QComboBox();
	shape->addItem("Amorphous");
	shape->addItem("Coarse");
	shape->addItem("Pleomorphic");
	shape->addItem("Fine_Linear_Branching");
	d->editInfTableWidget->setCellWidget(0, 2, shape);

	QComboBox *distribution = new QComboBox();
	distribution->addItem("Clustered");
	distribution->addItem("Linear");
	distribution->addItem("Regional");
	distribution->addItem("Segmental");
	distribution->addItem("Diffuse");
	d->editInfTableWidget->setCellWidget(0, 3, distribution);

	//init m_dicomInf
	m_dicomInf.insert("PatientID", "NA");
	m_dicomInf.insert("PatientBirthDate", "NA");
	m_dicomInf.insert("StudyID", "NA");
	m_dicomInf.insert("StudyDate", "NA");
	m_dicomInf.insert("SeriesNumber", "NA");
	m_dicomInf.insert("View", "NA");
	m_dicomInf.insert("Modality", "NA");
	m_dicomInf.insert("imageRow", "NA");
	m_dicomInf.insert("imageColumn", "NA");
	m_dicomInf.insert("imageSlice", "NA");
	m_dicomInf.insert("imageRowSpaceing", "NA");
	m_dicomInf.insert("imageColumnSpaceing", "NA");
	m_dicomInf.insert("imageSliceSpaceing", "NA");
	m_dicomInf.insert("scalarType","NA");

	//init m_pacasInf
	m_pacasInf.insert("subtlety","NA");
	m_pacasInf.insert("density", "NA");
	m_pacasInf.insert("assessment", "NA");
	m_pacasInf.insert("pathology", "NA");

	//init m_AnnotationInf
	m_AnnotationInf.insert("clusterNumber","NA");
	m_AnnotationInf.insert("1-number", "NA");
	m_AnnotationInf.insert("1-size", "NA");
	m_AnnotationInf.insert("1-shape", "NA");
	m_AnnotationInf.insert("1-distribution", "NA");
	m_AnnotationInf.insert("1-xCenterRas", "NA");
	m_AnnotationInf.insert("1-yCenterRas", "NA");
	m_AnnotationInf.insert("1-zCenterRas", "NA");
	m_AnnotationInf.insert("1-xRadiusRas", "NA");
	m_AnnotationInf.insert("1-yRadiusRas", "NA");
	m_AnnotationInf.insert("1-zRadiusRas", "NA");
	m_AnnotationInf.insert("1-xCenterIjk", "NA");
	m_AnnotationInf.insert("1-yCenterIjk", "NA");
	m_AnnotationInf.insert("1-zCenterIjk", "NA");
	m_AnnotationInf.insert("1-xRadiusIjk", "NA");
	m_AnnotationInf.insert("1-yRadiusIjk", "NA");
	m_AnnotationInf.insert("1-zRadiusIjk", "NA");
	m_number = 1;
	m_index = 1;
	m_readMode = false;
}

void qSlicerbreastImageModuleWidget::on_addButton_clicked()
{
	Q_D(const qSlicerbreastImageModuleWidget);
	int rowNumber = d->editInfTableWidget->rowCount();
	d->editInfTableWidget->setRowCount(rowNumber+1);
	QSpinBox *number = new QSpinBox();
	d->editInfTableWidget->setCellWidget(rowNumber, 0, number);

	QDoubleSpinBox *size = new QDoubleSpinBox();
	size->setSuffix("mm");
	d->editInfTableWidget->setCellWidget(rowNumber, 1, size);

	QComboBox *shape = new QComboBox();
	shape->addItem("Amorphous");
	shape->addItem("Coarse");
	shape->addItem("Pleomorphic");
	shape->addItem("Fine_Linear_Branching");
	d->editInfTableWidget->setCellWidget(rowNumber, 2, shape);

	QComboBox *distribution = new QComboBox();
	distribution->addItem("Clustered");
	distribution->addItem("Linear");
	distribution->addItem("Regional");
	distribution->addItem("Segmental");
	distribution->addItem("Diffuse");
	d->editInfTableWidget->setCellWidget(rowNumber, 3, distribution);
	m_number = m_number + 1;
	//locate the current row
	m_index = m_number;
	m_AnnotationInf.insert(QString("%1-number").arg(m_number), "NA");
	m_AnnotationInf.insert(QString("%1-size").arg(m_number), "NA");
	m_AnnotationInf.insert(QString("%1-shape").arg(m_number), "NA");
	m_AnnotationInf.insert(QString("%1-distribution").arg(m_number), "NA");
	m_AnnotationInf.insert(QString("%1-xCenterRas").arg(m_number), "NA");
	m_AnnotationInf.insert(QString("%1-yCenterRas").arg(m_number), "NA");
	m_AnnotationInf.insert(QString("%1-zCenterRas").arg(m_number), "NA");
	m_AnnotationInf.insert(QString("%1-xRadiusRas").arg(m_number), "NA");
	m_AnnotationInf.insert(QString("%1-yRadiusRas").arg(m_number), "NA");
	m_AnnotationInf.insert(QString("%1-zRadiusRas").arg(m_number), "NA");
	m_AnnotationInf.insert(QString("%1-xCenterIjk").arg(m_number), "NA");
	m_AnnotationInf.insert(QString("%1-yCenterIjk").arg(m_number), "NA");
	m_AnnotationInf.insert(QString("%1-zCenterIjk").arg(m_number), "NA");
	m_AnnotationInf.insert(QString("%1-xRadiusIjk").arg(m_number), "NA");
	m_AnnotationInf.insert(QString("%1-yRadiusIjk").arg(m_number), "NA");
	m_AnnotationInf.insert(QString("%1-zRadiusIjk").arg(m_number), "NA");
}

void qSlicerbreastImageModuleWidget::on_deleteButton_clicked()
{
	Q_D(const qSlicerbreastImageModuleWidget);
	this->onEditInfTableChanged();
	int rowIndex = d->editInfTableWidget->currentRow();
	if (rowIndex != -1)
	{
		m_index = rowIndex+1;
		if (m_index != m_number)
		{
			for (int i = 1; i <= m_number - m_index; i++)
			{
				m_AnnotationInf[QString("%1-number").arg(m_index)] = m_AnnotationInf[QString("%1-number").arg(m_index + 1)];
				m_AnnotationInf[QString("%1-size").arg(m_index)] = m_AnnotationInf[QString("%1-size").arg(m_index + 1)];
				m_AnnotationInf[QString("%1-shape").arg(m_index)] = m_AnnotationInf[QString("%1-shape").arg(m_index + 1)];
				m_AnnotationInf[QString("%1-distribution").arg(m_index)] = m_AnnotationInf[QString("%1-distribution").arg(m_index + 1)];
				m_AnnotationInf[QString("%1-xCenterRas").arg(m_index)] = m_AnnotationInf[QString("%1-xCenterRas").arg(m_index + 1)];
				m_AnnotationInf[QString("%1-yCenterRas").arg(m_index)] = m_AnnotationInf[QString("%1-yCenterRas").arg(m_index + 1)];
				m_AnnotationInf[QString("%1-zCenterRas").arg(m_index)] = m_AnnotationInf[QString("%1-zCenterRas").arg(m_index + 1)];
				m_AnnotationInf[QString("%1-xRadiusRas").arg(m_index)] = m_AnnotationInf[QString("%1-xRadiusRas").arg(m_index + 1)];
				m_AnnotationInf[QString("%1-yRadiusRas").arg(m_index)] = m_AnnotationInf[QString("%1-yRadiusRas").arg(m_index + 1)];
				m_AnnotationInf[QString("%1-zRadiusRas").arg(m_index)] = m_AnnotationInf[QString("%1-zRadiusRas").arg(m_index + 1)];
				m_AnnotationInf[QString("%1-xCenterIjk").arg(m_index)] = m_AnnotationInf[QString("%1-xCenterIjk").arg(m_index + 1)];
				m_AnnotationInf[QString("%1-yCenterIjk").arg(m_index)] = m_AnnotationInf[QString("%1-yCenterIjk").arg(m_index + 1)];
				m_AnnotationInf[QString("%1-zCenterIjk").arg(m_index)] = m_AnnotationInf[QString("%1-zCenterIjk").arg(m_index + 1)];
				m_AnnotationInf[QString("%1-xRadiusIjk").arg(m_index)] = m_AnnotationInf[QString("%1-xRadiusIjk").arg(m_index + 1)];
				m_AnnotationInf[QString("%1-yRadiusIjk").arg(m_index)] = m_AnnotationInf[QString("%1-yRadiusIjk").arg(m_index + 1)];
				m_AnnotationInf[QString("%1-zRadiusIjk").arg(m_index)] = m_AnnotationInf[QString("%1-zRadiusIjk").arg(m_index + 1)];
				m_index = m_index + 1;
			}
		}
		d->editInfTableWidget->removeRow(rowIndex);
		m_AnnotationInf.remove(QString("%1-number").arg(m_number));
		m_AnnotationInf.remove(QString("%1-size").arg(m_number));
		m_AnnotationInf.remove(QString("%1-shape").arg(m_number));
		m_AnnotationInf.remove(QString("%1-distribution").arg(m_number));
		m_AnnotationInf.remove(QString("%1-xCenterRas").arg(m_number));
		m_AnnotationInf.remove(QString("%1-yCenterRas").arg(m_number));
		m_AnnotationInf.remove(QString("%1-zCenterRas").arg(m_number));
		m_AnnotationInf.remove(QString("%1-xRadiusRas").arg(m_number));
		m_AnnotationInf.remove(QString("%1-yRadiusRas").arg(m_number));
		m_AnnotationInf.remove(QString("%1-zRadiusRas").arg(m_number));
		m_AnnotationInf.remove(QString("%1-xCenterIjk").arg(m_number));
		m_AnnotationInf.remove(QString("%1-yCenterIjk").arg(m_number));
		m_AnnotationInf.remove(QString("%1-zCenterIjk").arg(m_number));
		m_AnnotationInf.remove(QString("%1-xRadiusIjk").arg(m_number));
		m_AnnotationInf.remove(QString("%1-yRadiusIjk").arg(m_number));
		m_AnnotationInf.remove(QString("%1-zRadiusIjk").arg(m_number));
		m_number = m_number - 1;
		m_index = m_number;
	}
}

void qSlicerbreastImageModuleWidget::onPacasInfChanged()
{
	Q_D(const qSlicerbreastImageModuleWidget);
	m_pacasInf["subtlety"] = d->subtleyComboBox->currentText();
	m_pacasInf["density"] = d->densityComboBox->currentText();
	m_pacasInf["assessment"] = d->biradsComboBox->currentText();
	m_pacasInf["pathology"] = d->resultComboBox->currentText();
}

void qSlicerbreastImageModuleWidget::onEditInfTableChanged()
{
	Q_D(const qSlicerbreastImageModuleWidget);
	m_AnnotationInf["clusterNumber"] = QString::number(m_number, 10);
	for (int i = 1; i <= m_number; i++)
	{
		QWidget *widget = d->editInfTableWidget->cellWidget(i-1,0);
		QSpinBox *number = (QSpinBox*)widget;
		QString str = QString::number(number->value(), 10);
		m_AnnotationInf[QString("%1-number").arg(i)] = str;

		widget = d->editInfTableWidget->cellWidget(i-1,1);
		QDoubleSpinBox *size = (QDoubleSpinBox*)widget;
		str = QString::number(size->value(), 10, 4);;
		m_AnnotationInf[QString("%1-size").arg(i)] = str;

		widget = d->editInfTableWidget->cellWidget(i-1, 2);
		QComboBox *shape = (QComboBox*)widget;
		str = shape->currentText();
		m_AnnotationInf[QString("%1-shape").arg(i)] = str;

		widget = d->editInfTableWidget->cellWidget(i-1, 3);
		QComboBox *distribution = (QComboBox*)widget;
		str = distribution->currentText();
		m_AnnotationInf[QString("%1-distribution").arg(i)] = str;
	}
}

void qSlicerbreastImageModuleWidget::on_importXMLButton_clicked()
{
	Q_D(const qSlicerbreastImageModuleWidget);
	vtkSlicerbreastImageLogic *logic = d->logic();
	for (int j=1;j<=m_number;j++)
	{
		d->editInfTableWidget->removeRow(j-1);
	}
	m_number = 0;
	QString fileName = QFileDialog::getOpenFileName(this, tr("Open breastImage XML File"), "/home", tr("XML files(*.xml)"));
	logic->readAnnotationXML(fileName, t_dicomInf, t_pacasInf, t_annotationInf);
	int number = t_annotationInf.value("clusterNumber").toInt();
	for (int i = 1; i <= number; i++)
	{
		m_number = m_number + 1;
		d->editInfTableWidget->setRowCount(i);
		int t_number = t_annotationInf.value(QString("number-%1").arg(i)).toInt();
		QSpinBox *number = new QSpinBox();
		number->setValue(t_number);
		d->editInfTableWidget->setCellWidget(i-1, 0, number);

		double t_size = t_annotationInf.value(QString("size-%1").arg(i)).toDouble();
		QDoubleSpinBox *size = new QDoubleSpinBox();
		size->setValue(t_size);
		size->setSuffix("mm");
		d->editInfTableWidget->setCellWidget(i-1, 1, size);

		QString t_shape = t_annotationInf.value(QString("shape-%1").arg(i));
		QComboBox *shape = new QComboBox();
		shape->addItem(t_shape);
		d->editInfTableWidget->setCellWidget(i-1, 2, shape);

		QString t_distribution = t_annotationInf.value(QString("distribution-%1").arg(i));
		QComboBox *distribution = new QComboBox();
		distribution->addItem(t_distribution);
		d->editInfTableWidget->setCellWidget(i-1, 3, distribution);

		QString xCenterRas = t_annotationInf.value(QString("xCenterRas-%1").arg(i));
		d->editInfTableWidget->setItem(i - 1, 4, new QTableWidgetItem(xCenterRas));
		QString yCenterRas = t_annotationInf.value(QString("yCenterRas-%1").arg(i));
		d->editInfTableWidget->setItem(i - 1, 5, new QTableWidgetItem(yCenterRas));
		QString zCenterRas = t_annotationInf.value(QString("zCenterRas-%1").arg(i));
		d->editInfTableWidget->setItem(i - 1, 6, new QTableWidgetItem(zCenterRas));

		QString xRadiusRas = t_annotationInf.value(QString("xRadiusRas-%1").arg(i));
		d->editInfTableWidget->setItem(i - 1, 7, new QTableWidgetItem(xRadiusRas));
		QString yRadiusRas = t_annotationInf.value(QString("yRadiusRas-%1").arg(i));
		d->editInfTableWidget->setItem(i - 1, 8, new QTableWidgetItem(yRadiusRas));
		QString zRadiusRas = t_annotationInf.value(QString("zRadiusRas-%1").arg(i));
		d->editInfTableWidget->setItem(i - 1, 9, new QTableWidgetItem(zRadiusRas));
	}

	double xyz1 = t_annotationInf.value("xCenterRas-1").toDouble();
	double xyz2 = t_annotationInf.value("yCenterRas-1").toDouble();
	double xyz3 = t_annotationInf.value("zCenterRas-1").toDouble();
	double xyzRadius1 = t_annotationInf.value("xRadiusRas-1").toDouble();
	double xyzRadius2 = t_annotationInf.value("yRadiusRas-1").toDouble();
	double xyzRadius3 = t_annotationInf.value("zRadiusRas-1").toDouble();

	vtkMRMLAnnotationROINode *roiNode = vtkMRMLAnnotationROINode::New();
	vtkSmartPointer<vtkMRMLScene> scene = this->mrmlScene();
	scene->AddNode(roiNode);
	roiNode->SetXYZ(xyz1, xyz2, xyz3);
	roiNode->SetRadiusXYZ(xyzRadius1, xyzRadius2, xyzRadius3);
	d->inputEditROINodeComboBox->setCurrentNode(roiNode);
	roiNode->Delete();
	m_readMode = true;
}

void qSlicerbreastImageModuleWidget::on_outputXMLButton_clicked()
{
	Q_D(const qSlicerbreastImageModuleWidget);
	vtkSmartPointer<vtkMRMLVolumeNode> inputVolumeNode = vtkMRMLVolumeNode::SafeDownCast(d->inputEditVolumeNodeComboBox->currentNode());
	if (d->reportIdSpinBox->value() != 0)
	{
		this->onPacasInfChanged();
		this->onEditInfTableChanged();
		vtkSlicerbreastImageLogic *logic = d->logic();
		QString strInit = "";
		QString dirPath = QFileDialog::getExistingDirectory(this, QString::fromLocal8Bit("Select file path "),
			strInit, QFileDialog::ShowDirsOnly);
		QString str = QString::number(d->reportIdSpinBox->value(),10);
		QString fileName;
		QDateTime current_date_time = QDateTime::currentDateTime();
		QString current_date = current_date_time.toString("yyyyMMdd");
		dirPath = dirPath + QString("/case%1_%2_%3").arg(str).arg(m_dicomInf.value("PatientID")).arg(current_date);
		QDir dir(dirPath);
		if (!dir.exists())
			dir.mkdir(dirPath);
		if (m_pacasInf.value("pathology") == "Benign")
		{
			dirPath = dirPath + QString("/A_%1_%2_%3_annotation.xml").arg(str).arg(m_view).arg(m_modality);
			fileName = QString("A_%1_%2_%3_annotation").arg(str).arg(m_view).arg(m_modality);
			inputVolumeNode->SetName(QString("A_%1_%2_%3_image").arg(str).arg(m_view).arg(m_modality).toStdString().c_str());
			inputVolumeNode->Modified();
		}
		else if (m_pacasInf.value("pathology") == "Malignant")
		{
			dirPath = dirPath + QString("/B_%1_%2_%3_annotation.xml").arg(str).arg(m_view).arg(m_modality);
			fileName = QString("B_%1_%2_%3_annotation").arg(str).arg(m_view).arg(m_modality);
			inputVolumeNode->SetName(QString("B_%1_%2_%3_image").arg(str).arg(m_view).arg(m_modality).toStdString().c_str());
			inputVolumeNode->Modified();
		}
		logic->writeAnnotationXML(dirPath, fileName, m_dicomInf, m_pacasInf, m_AnnotationInf);
		vtkSlicerApplicationLogic *appLogic = this->module()->appLogic();
		vtkMRMLSelectionNode *selectionNode = appLogic->GetSelectionNode();
		selectionNode->SetReferenceActiveVolumeID(inputVolumeNode->GetID());
		appLogic->PropagateVolumeSelection();
	}
	else
	{
		QMessageBox msgBox;
		QString str = "Please input valid report id!";
		msgBox.setIcon(QMessageBox::Information);
		msgBox.setText(str);
		msgBox.setStandardButtons(QMessageBox::Ok);
		msgBox.setDefaultButton(QMessageBox::Ok);
		int ret = msgBox.exec();
	}
}

void qSlicerbreastImageModuleWidget::on_refreshRoiButton_clicked()
{
	Q_D(const qSlicerbreastImageModuleWidget);
	int rowIndex = d->editInfTableWidget->currentRow();
	if (rowIndex != -1)
	{
		vtkSmartPointer<vtkMRMLVolumeNode> inputVolumeNode = vtkMRMLVolumeNode::SafeDownCast(d->inputEditVolumeNodeComboBox->currentNode());
		vtkSmartPointer<vtkMRMLAnnotationROINode> inputAnnotationRoiNode = vtkMRMLAnnotationROINode::SafeDownCast(d->inputEditROINodeComboBox->currentNode());
		if (m_readMode == false)
		{
			vtkSlicerbreastImageLogic *logic = d->logic();
			if (inputAnnotationRoiNode)
			{
				//ras coordinates
				double roiXYZ[3];
				double roiRadius[3];
				inputAnnotationRoiNode->GetXYZ(roiXYZ);
				inputAnnotationRoiNode->GetRadiusXYZ(roiRadius);
				QString attributeValue;
				attributeValue = QString::number(roiXYZ[0], 10, 4);
				m_AnnotationInf[QString("%1-xCenterRas").arg(rowIndex + 1)] = attributeValue;
				d->editInfTableWidget->setItem(rowIndex, 4, new QTableWidgetItem(attributeValue));
				attributeValue = QString::number(roiXYZ[1], 10, 4);
				m_AnnotationInf[QString("%1-yCenterRas").arg(rowIndex + 1)] = attributeValue;
				d->editInfTableWidget->setItem(rowIndex, 5, new QTableWidgetItem(attributeValue));
				attributeValue = QString::number(roiXYZ[2], 10, 4);
				m_AnnotationInf[QString("%1-zCenterRas").arg(rowIndex + 1)] = attributeValue;
				d->editInfTableWidget->setItem(rowIndex, 6, new QTableWidgetItem(attributeValue));
				attributeValue = QString::number(roiRadius[0], 10, 4);
				m_AnnotationInf[QString("%1-xRadiusRas").arg(rowIndex + 1)] = attributeValue;
				d->editInfTableWidget->setItem(rowIndex, 7, new QTableWidgetItem(attributeValue));
				attributeValue = QString::number(roiRadius[1], 10, 4);
				m_AnnotationInf[QString("%1-yRadiusRas").arg(rowIndex + 1)] = attributeValue;
				d->editInfTableWidget->setItem(rowIndex, 8, new QTableWidgetItem(attributeValue));
				attributeValue = QString::number(roiRadius[2], 10, 4);
				m_AnnotationInf[QString("%1-zRadiusRas").arg(rowIndex + 1)] = attributeValue;
				d->editInfTableWidget->setItem(rowIndex, 9, new QTableWidgetItem(attributeValue));;
			}
			if (inputVolumeNode && inputAnnotationRoiNode)
			{
				logic->acquireRoiLocation(inputVolumeNode, inputAnnotationRoiNode);
				// ijk coordinates
				m_AnnotationInf[QString("%1-xCenterIjk").arg(rowIndex + 1)] = QString::number(logic->roiXYZIJK[0], 10);
				m_AnnotationInf[QString("%1-yCenterIjk").arg(rowIndex + 1)] = QString::number(logic->roiXYZIJK[1], 10);
				m_AnnotationInf[QString("%1-zCenterIjk").arg(rowIndex + 1)] = QString::number(logic->roiXYZIJK[2], 10);
				m_AnnotationInf[QString("%1-xRadiusIjk").arg(rowIndex + 1)] = QString::number(logic->roiRadiusIJK[0], 10);
				m_AnnotationInf[QString("%1-yRadiusIjk").arg(rowIndex + 1)] = QString::number(logic->roiRadiusIJK[1], 10);
				m_AnnotationInf[QString("%1-zRadiusIjk").arg(rowIndex + 1)] = QString::number(logic->roiRadiusIJK[2], 10);
			}
		}
		else
		{
			if (inputAnnotationRoiNode)
			{
				double xyz1 = t_annotationInf.value(QString("xCenterRas-%1").arg(rowIndex + 1)).toDouble();
				double xyz2 = t_annotationInf.value(QString("yCenterRas-%1").arg(rowIndex + 1)).toDouble();
				double xyz3 = t_annotationInf.value(QString("zCenterRas-%1").arg(rowIndex + 1)).toDouble();
				double xyzRadius1 = t_annotationInf.value(QString("xRadiusRas-%1").arg(rowIndex + 1)).toDouble();
				double xyzRadius2 = t_annotationInf.value(QString("yRadiusRas-%1").arg(rowIndex + 1)).toDouble();
				double xyzRadius3 = t_annotationInf.value(QString("zRadiusRas-%1").arg(rowIndex + 1)).toDouble();
				inputAnnotationRoiNode->SetXYZ(xyz1, xyz2, xyz3);
				inputAnnotationRoiNode->SetRadiusXYZ(xyzRadius1, xyzRadius2, xyzRadius3);
			}
		}
	}
}

void qSlicerbreastImageModuleWidget::on_refreshRulerButton_clicked()
{
	Q_D(const qSlicerbreastImageModuleWidget);
	vtkSmartPointer<vtkMRMLAnnotationRulerNode> inputAnnotationRulerNode = vtkMRMLAnnotationRulerNode::SafeDownCast(d->inputEditRulerNodeComboBox->currentNode());
	int rowIndex = d->editInfTableWidget->currentRow();
	if (rowIndex != -1)
	{
		if (inputAnnotationRulerNode)
		{
			double length = inputAnnotationRulerNode->GetDistanceMeasurement();
			QDoubleSpinBox *size = new QDoubleSpinBox();
			size->setValue(length);
			size->setSuffix("mm");
			d->editInfTableWidget->setCellWidget(rowIndex, 1, size);
		}
	}
}