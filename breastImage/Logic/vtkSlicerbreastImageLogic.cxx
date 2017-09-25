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

// breastImage Logic includes
#include "vtkSlicerbreastImageLogic.h"

// MRML includes
#include <vtkMRMLScene.h>

// VTK includes
#include <vtkIntArray.h>
#include <vtkNew.h>
#include <vtkImageData.h>
#include <vtkMatrix4x4.h>
#include <vtkObjectFactory.h>

// MRML includes
#include <vtkMRMLVolumeNode.h>
#include <vtkMRMLAnnotationROINode.h>
#include <vtkMRMLTransformNode.h>
#include <vtkMRMLLinearTransformNode.h>

// STD includes
#include <cassert>

//QT includes
#include <QDebug>
#include <QMessageBox>
#include <QDomDocument>
#include <QFile>
#include <QTextStream>
#include <QDateTime>

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkSlicerbreastImageLogic);

//----------------------------------------------------------------------------
vtkSlicerbreastImageLogic::vtkSlicerbreastImageLogic()
{
}

//----------------------------------------------------------------------------
vtkSlicerbreastImageLogic::~vtkSlicerbreastImageLogic()
{
}

//----------------------------------------------------------------------------
void vtkSlicerbreastImageLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//---------------------------------------------------------------------------
void vtkSlicerbreastImageLogic::SetMRMLSceneInternal(vtkMRMLScene * newScene)
{
  vtkNew<vtkIntArray> events;
  events->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
  events->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);
  events->InsertNextValue(vtkMRMLScene::EndBatchProcessEvent);
  this->SetAndObserveMRMLSceneEventsInternal(newScene, events.GetPointer());
}

//-----------------------------------------------------------------------------
void vtkSlicerbreastImageLogic::RegisterNodes()
{
  assert(this->GetMRMLScene() != 0);
}

//---------------------------------------------------------------------------
void vtkSlicerbreastImageLogic::UpdateFromMRMLScene()
{
  assert(this->GetMRMLScene() != 0);
}

//---------------------------------------------------------------------------
void vtkSlicerbreastImageLogic
::OnMRMLSceneNodeAdded(vtkMRMLNode* vtkNotUsed(node))
{
}

//---------------------------------------------------------------------------
void vtkSlicerbreastImageLogic
::OnMRMLSceneNodeRemoved(vtkMRMLNode* vtkNotUsed(node))
{
}

//---------------------------------------------------------------------------
QMap< QString, QString> vtkSlicerbreastImageLogic
::GetNodeAttribute(vtkMRMLNode *node)
{
	std::vector< std::string > attributeNames = node->GetAttributeNames();
	if (attributeNames.size() == 0)
	{
		qDebug() << "this node do not have attributes";
		QMessageBox::information(NULL, "Node Information", "this node do not have attributes!");
	}
	else
	{
		int row = 0;
		QMap< QString, QString> map;
		for (std::vector< std::string >::iterator iter = attributeNames.begin();
			iter != attributeNames.end(); ++iter, ++row)
		{
			map.insert(QString(iter->c_str()), QString(node->GetAttribute(iter->c_str())));
		}
		return map;
	}
}

//---------------------------------------------------------------------------
void vtkSlicerbreastImageLogic::acquireRoiLocation(vtkMRMLVolumeNode* inputVolume, vtkMRMLAnnotationROINode* inputROI)
{
	// make sure inputs are initialized
	if (!inputVolume || !inputROI)
	{
		return;
	}

	vtkNew<vtkImageData> imageDataWorkingCopy;
	imageDataWorkingCopy->DeepCopy(inputVolume->GetImageData());

	vtkNew<vtkMatrix4x4> inputRASToIJK;
	inputVolume->GetRASToIJKMatrix(inputRASToIJK.GetPointer());
	vtkNew<vtkMatrix4x4> inputIJKToRAS;
	inputVolume->GetIJKToRASMatrix(inputIJKToRAS.GetPointer());

	//ras
	double roiXYZ[3];
	double roiRadius[3];
	inputROI->GetXYZ(roiXYZ);
	inputROI->GetRadiusXYZ(roiRadius);

	double minXYZRAS[] = { roiXYZ[0] - roiRadius[0], roiXYZ[1] - roiRadius[1], roiXYZ[2] - roiRadius[2], 1. };
	double maxXYZRAS[] = { roiXYZ[0] + roiRadius[0], roiXYZ[1] + roiRadius[1], roiXYZ[2] + roiRadius[2], 1. };

	vtkMRMLTransformNode* roiTransform = inputROI->GetParentTransformNode();
	// account for the ROI parent transform, if present
	if (roiTransform && roiTransform->IsTransformToWorldLinear())
	{
		vtkNew<vtkMatrix4x4> roiTransformMatrix;
		roiTransform->GetMatrixTransformToWorld(roiTransformMatrix.GetPointer());
		if (roiTransformMatrix.GetPointer())
		{
			// multiply ROI's min and max corners with parent's transform to world to get real RAS position
			roiTransformMatrix->MultiplyPoint(minXYZRAS, minXYZRAS);
			roiTransformMatrix->MultiplyPoint(maxXYZRAS, maxXYZRAS);
		}
	}

	//transform to ijk
	double minXYZIJK[4], maxXYZIJK[4];
	inputRASToIJK->MultiplyPoint(minXYZRAS, minXYZIJK);
	inputRASToIJK->MultiplyPoint(maxXYZRAS, maxXYZIJK);

	double minX = std::min(minXYZIJK[0], maxXYZIJK[0]);
	double maxX = std::max(minXYZIJK[0], maxXYZIJK[0]); 
	double minY = std::min(minXYZIJK[1], maxXYZIJK[1]);
	double maxY = std::max(minXYZIJK[1], maxXYZIJK[1]);
	double minZ = std::min(minXYZIJK[2], maxXYZIJK[2]);
	double maxZ = std::max(minXYZIJK[2], maxXYZIJK[2]);

	int originalImageExtents[6];
	imageDataWorkingCopy->GetExtent(originalImageExtents);

	minX = std::max(minX, 0.);
	maxX = std::min(maxX, static_cast<double>(originalImageExtents[1]));
	minY = std::max(minY, 0.);
	maxY = std::min(maxY, static_cast<double>(originalImageExtents[3]));
	minZ = std::max(minZ, 0.);
	maxZ = std::min(maxZ, static_cast<double>(originalImageExtents[5]));

	int outputWholeExtent[6] = {
		static_cast<int>(minX),
		static_cast<int>(maxX),
		static_cast<int>(minY),
		static_cast<int>(maxY),
		static_cast<int>(minZ),
		static_cast<int>(maxZ) };

	//ijk
	roiRadiusIJK[0] = ((outputWholeExtent[1] - outputWholeExtent[0]) / 2);
	roiRadiusIJK[1] = ((outputWholeExtent[3] - outputWholeExtent[2]) / 2);
	roiRadiusIJK[2] = ((outputWholeExtent[5] - outputWholeExtent[4]) / 2);
	roiXYZIJK[0] = outputWholeExtent[0] + roiRadiusIJK[0];
	roiXYZIJK[1] = outputWholeExtent[2] + roiRadiusIJK[1];
	roiXYZIJK[2] = outputWholeExtent[4] + roiRadiusIJK[2];
}

void vtkSlicerbreastImageLogic::writeAnnotationXML(QString dir, QString fileName, QMap<QString, QString> m_dicomInf, QMap<QString, QString> m_pacasInf, QMap<QString, QString> m_annotationInf)
{
	//file dir
	QFile file(dir);
	if (!file.open(QFile::WriteOnly | QFile::Truncate))
		return;
	QDomDocument doc;
	QDomProcessingInstruction instruction;
	instruction = doc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"UTF-8\"");
	doc.appendChild(instruction);

	QDateTime current_date_time = QDateTime::currentDateTime();
	QString current_date = current_date_time.toString("yyyyMMdd");

	//root node  
	QDomElement rootNode = doc.createElement("BreastImageReport");
	rootNode.setAttribute("reportName", fileName);
	rootNode.setAttribute("reportDate", current_date);
	rootNode.setAttribute("version", "1.0");
	doc.appendChild(rootNode);

	//dicom information node
	QDomElement dicomNode = doc.createElement("DicomInformation");
	QDomElement dicomInf;
	QDomText dicomText;
	//patient information 
	dicomInf = doc.createElement("PatientID");
	dicomText = doc.createTextNode(m_dicomInf.value("PatientID"));
	dicomInf.appendChild(dicomText);
	dicomNode.appendChild(dicomInf);
	dicomInf = doc.createElement("PatientBirthDate");
	dicomText = doc.createTextNode(m_dicomInf.value("PatientBirthDate"));
	dicomInf.appendChild(dicomText);
	dicomNode.appendChild(dicomInf);
	//study information
	dicomInf = doc.createElement("StudyID");
	dicomText = doc.createTextNode(m_dicomInf.value("StudyID"));
	dicomInf.appendChild(dicomText);
	dicomNode.appendChild(dicomInf);
	dicomInf = doc.createElement("StudyDate");
	dicomText = doc.createTextNode(m_dicomInf.value("StudyDate"));
	dicomInf.appendChild(dicomText);
	dicomNode.appendChild(dicomInf);
	//series information
	dicomInf = doc.createElement("SeriesNumber");
	dicomText = doc.createTextNode(m_dicomInf.value("SeriesNumber"));
	dicomInf.appendChild(dicomText);
	dicomNode.appendChild(dicomInf);
	dicomInf = doc.createElement("View");
	dicomText = doc.createTextNode(m_dicomInf.value("View"));
	dicomInf.appendChild(dicomText);
	dicomNode.appendChild(dicomInf);
	dicomInf = doc.createElement("Modality");
	dicomText = doc.createTextNode(m_dicomInf.value("Modality"));
	dicomInf.appendChild(dicomText);
	dicomNode.appendChild(dicomInf);
	//image information
	dicomInf = doc.createElement("ImageRow");
	dicomText = doc.createTextNode(m_dicomInf.value("imageRow"));
	dicomInf.appendChild(dicomText);
	dicomNode.appendChild(dicomInf);
	dicomInf = doc.createElement("ImageColumn");
	dicomText = doc.createTextNode(m_dicomInf.value("imageColumn"));
	dicomInf.appendChild(dicomText);
	dicomNode.appendChild(dicomInf);
	dicomInf = doc.createElement("ImageSlice");
	dicomText = doc.createTextNode(m_dicomInf.value("imageSlice"));
	dicomInf.appendChild(dicomText);
	dicomNode.appendChild(dicomInf);
	dicomInf = doc.createElement("ImageRowSpaceing");
	dicomText = doc.createTextNode(m_dicomInf.value("imageRowSpaceing"));
	dicomInf.appendChild(dicomText);
	dicomNode.appendChild(dicomInf);
	dicomInf = doc.createElement("ImageColumnSpaceing");
	dicomText = doc.createTextNode(m_dicomInf.value("imageColumnSpaceing"));
	dicomInf.appendChild(dicomText);
	dicomNode.appendChild(dicomInf);
	dicomInf = doc.createElement("ImageSliceSpaceing");
	dicomText = doc.createTextNode(m_dicomInf.value("imageSliceSpaceing"));
	dicomInf.appendChild(dicomText);
	dicomNode.appendChild(dicomInf);
	dicomInf = doc.createElement("PixelBits");
	dicomText = doc.createTextNode("16");
	dicomInf.appendChild(dicomText);
	dicomNode.appendChild(dicomInf);
	dicomInf = doc.createElement("PixelSize");
	dicomText = doc.createTextNode("70");
	dicomInf.appendChild(dicomText);
	dicomNode.appendChild(dicomInf);
	rootNode.appendChild(dicomNode);

	//pacas information node
	QDomElement pacasNode = doc.createElement("PacasInformation");
	QDomElement pacasInf;
	QDomText pacasText;
	pacasInf = doc.createElement("Subtlety");
	pacasText = doc.createTextNode(m_pacasInf.value("subtlety"));
	pacasInf.appendChild(pacasText);
	pacasNode.appendChild(pacasInf);
	pacasInf = doc.createElement("Density");
	pacasText = doc.createTextNode(m_pacasInf.value("density"));
	pacasInf.appendChild(pacasText);
	pacasNode.appendChild(pacasInf);
	pacasInf = doc.createElement("Assessment");
	pacasText = doc.createTextNode(m_pacasInf.value("assessment"));
	pacasInf.appendChild(pacasText);
	pacasNode.appendChild(pacasInf);
	pacasInf = doc.createElement("Pathology");
	pacasText = doc.createTextNode(m_pacasInf.value("pathology"));
	pacasInf.appendChild(pacasText);
	pacasNode.appendChild(pacasInf);
	rootNode.appendChild(pacasNode);

	//annotation information node
	QDomElement annotationNode = doc.createElement("AnnotationInformation");
	QDomElement annotationInf;
	QDomText annotationText;
	int number = m_annotationInf.value("clusterNumber").toInt();
	annotationNode.setAttribute("clusterNumber", number);
	for (int i = 1; i <= number; i++)
	{
		QDomElement calcificationNode = doc.createElement(QString("cluster-%1").arg(i));
		annotationInf = doc.createElement(QString("number-%1").arg(i));
		annotationText = doc.createTextNode(m_annotationInf.value(QString("%1-number").arg(i)));
		annotationInf.appendChild(annotationText);
		calcificationNode.appendChild(annotationInf);
		annotationInf = doc.createElement(QString("size-%1").arg(i));
		annotationText = doc.createTextNode(m_annotationInf.value(QString("%1-size").arg(i)));
		annotationInf.appendChild(annotationText);
		calcificationNode.appendChild(annotationInf);
		annotationInf = doc.createElement(QString("shape-%1").arg(i));
		annotationText = doc.createTextNode(m_annotationInf.value(QString("%1-shape").arg(i)));
		annotationInf.appendChild(annotationText);
		calcificationNode.appendChild(annotationInf);
		annotationInf = doc.createElement(QString("distribution-%1").arg(i));
		annotationText = doc.createTextNode(m_annotationInf.value(QString("%1-distribution").arg(i)));
		annotationInf.appendChild(annotationText);
		calcificationNode.appendChild(annotationInf);
		annotationInf = doc.createElement(QString("xCenterRas-%1").arg(i));
		annotationText = doc.createTextNode(m_annotationInf.value(QString("%1-xCenterRas").arg(i)));
		annotationInf.appendChild(annotationText);
		calcificationNode.appendChild(annotationInf);
		annotationInf = doc.createElement(QString("yCenterRas-%1").arg(i));
		annotationText = doc.createTextNode(m_annotationInf.value(QString("%1-yCenterRas").arg(i)));
		annotationInf.appendChild(annotationText);
		calcificationNode.appendChild(annotationInf);
		annotationInf = doc.createElement(QString("zCenterRas-%1").arg(i));
		annotationText = doc.createTextNode(m_annotationInf.value(QString("%1-zCenterRas").arg(i)));
		annotationInf.appendChild(annotationText);
		calcificationNode.appendChild(annotationInf);
		annotationInf = doc.createElement(QString("xRadiusRas-%1").arg(i));
		annotationText = doc.createTextNode(m_annotationInf.value(QString("%1-xRadiusRas").arg(i)));
		annotationInf.appendChild(annotationText);
		calcificationNode.appendChild(annotationInf);
		annotationInf = doc.createElement(QString("yRadiusRas-%1").arg(i));
		annotationText = doc.createTextNode(m_annotationInf.value(QString("%1-yRadiusRas").arg(i)));
		annotationInf.appendChild(annotationText);
		calcificationNode.appendChild(annotationInf);
		annotationInf = doc.createElement(QString("zRadiusRas-%1").arg(i));
		annotationText = doc.createTextNode(m_annotationInf.value(QString("%1-zRadiusRas").arg(i)));
		annotationInf.appendChild(annotationText);
		calcificationNode.appendChild(annotationInf);
		annotationInf = doc.createElement(QString("xCenterIjk-%1").arg(i));
		annotationText = doc.createTextNode(m_annotationInf.value(QString("%1-xCenterIjk").arg(i)));
		annotationInf.appendChild(annotationText);
		calcificationNode.appendChild(annotationInf);
		annotationInf = doc.createElement(QString("yCenterIjk-%1").arg(i));
		annotationText = doc.createTextNode(m_annotationInf.value(QString("%1-yCenterIjk").arg(i)));
		annotationInf.appendChild(annotationText);
		calcificationNode.appendChild(annotationInf);
		annotationInf = doc.createElement(QString("zCenteIjk-%1").arg(i));
		annotationText = doc.createTextNode(m_annotationInf.value(QString("%1-zCenterIjk").arg(i)));
		annotationInf.appendChild(annotationText);
		calcificationNode.appendChild(annotationInf);
		annotationInf = doc.createElement(QString("xRadiusIjk-%1").arg(i));
		annotationText = doc.createTextNode(m_annotationInf.value(QString("%1-xRadiusIjk").arg(i)));
		annotationInf.appendChild(annotationText);
		calcificationNode.appendChild(annotationInf);
		annotationInf = doc.createElement(QString("yRadiusIjk-%1").arg(i));
		annotationText = doc.createTextNode(m_annotationInf.value(QString("%1-yRadiusIjk").arg(i)));
		annotationInf.appendChild(annotationText);
		calcificationNode.appendChild(annotationInf);
		annotationInf = doc.createElement(QString("zRadiusIjk-%1").arg(i));
		annotationText = doc.createTextNode(m_annotationInf.value(QString("%1-zRadiusIjk").arg(i)));
		annotationInf.appendChild(annotationText);
		calcificationNode.appendChild(annotationInf);
		annotationNode.appendChild(calcificationNode);
	}
	if (number > 0)
	{
		rootNode.appendChild(annotationNode);
	}

	//output file  
	QTextStream out_stream(&file);
	doc.save(out_stream, 4);
	file.close();
}

void vtkSlicerbreastImageLogic::readAnnotationXML(QString fileName, QMap<QString, QString> &m_dicomInf, QMap<QString, QString> &m_pacasInf, QMap<QString, QString> &m_annotationInf)
{
	QFile file(fileName);
	if (!file.open(QFile::ReadOnly))
		return;

	QDomDocument doc;
	if (!doc.setContent(&file))
	{
		file.close();
		return;
	}
	file.close();

	// root node
	QDomElement root = doc.documentElement(); 
	qDebug() << root.nodeName();
	qDebug() << root.attributeNode("reportName").value();

	QDomNode node = root.firstChild(); 
	while (!node.isNull())
	{
		QDomElement element = node.toElement();
		// dicom information node
		if (element.tagName() == "DicomInformation")
		{
			QDomNode dicomNode = element.firstChild();
			while (!dicomNode.isNull())
			{
				m_dicomInf.insert(dicomNode.nodeName(), dicomNode.toElement().text());
				dicomNode = dicomNode.nextSibling();
			}
		}
		// pacas information node
		if (element.tagName() == "PacasInformation")
		{
			QDomNode pacasNode = element.firstChild();
			while (!pacasNode.isNull())
			{
				m_pacasInf.insert(pacasNode.nodeName(), pacasNode.toElement().text());
				pacasNode = pacasNode.nextSibling();
			}
		}
		// annotation information node
		if (element.tagName() == "AnnotationInformation")
		{
			QDomNode annotationNode = element.firstChild();
			m_annotationInf.insert("clusterNumber", element.attribute("clusterNumber"));
			for (int i = 1; i <= element.attribute("clusterNumber").toInt(); i++)
			{
				if (!annotationNode.isNull())
				{
					QDomNode annotationLeafNode = annotationNode.firstChild();
					while (!annotationLeafNode.isNull())
					{
						m_annotationInf.insert(annotationLeafNode.nodeName(), annotationLeafNode.toElement().text());
						annotationLeafNode = annotationLeafNode.nextSibling();
					}
					annotationNode = annotationNode.nextSibling();
				}
			}
		}
		node = node.nextSibling();
	}
}

void vtkSlicerbreastImageLogic::coordinatesTransform(vtkMRMLVolumeNode* inputVolume)
{
	if (!inputVolume)
	{
		return;
	}

	vtkSmartPointer<vtkImageData> imageData = inputVolume->GetImageData();
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
				a = -i + dims[0]-1;
				b = -j + dims[1]-1;
				unsigned short* p = (unsigned short*)(imageDataWorkingCopy->GetScalarPointer(a, b, k));
				unsigned short value = *p;
				unsigned short* q = (unsigned short*)(imageData->GetScalarPointer(i, j, k));
				*q = value;
			}

		}

	}

}
