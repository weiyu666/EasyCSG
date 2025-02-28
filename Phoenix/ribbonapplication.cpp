#include "ribbonapplication.hpp"

UnionValue::UnionValue(INT intValue)
{
	this->dataType = UnionValue::DATATYPE::TINT;
	this->intValue = intValue;
}

UnionValue::UnionValue(std::basic_string<TCHAR> stringValue)
{
	this->dataType = UnionValue::DATATYPE::TSTRING;
	new (&this->stringValue) std::basic_string<TCHAR>(stringValue);
}

UnionValue::UnionValue(_IN_(UnionValue & in))
{
	switch(in.dataType)
	{
		case UnionValue::DATATYPE::TINT:
			{
				this->dataType = UnionValue::DATATYPE::TINT;
				this->intValue = in.intValue;
			}
			break;

		case UnionValue::DATATYPE::TSTRING:
			{
				this->dataType = UnionValue::DATATYPE::TSTRING;
				new (&this->stringValue) std::basic_string<TCHAR>(in.stringValue);
			}
			break;
	}
}

UnionValue::~UnionValue()
{
	switch(this->dataType)
	{
		case UnionValue::DATATYPE::TSTRING:
			{
				this->stringValue.~basic_string();
			}
			break;
	}
}

UnionValue::operator INT()
{
	if(this->dataType == UnionValue::DATATYPE::TINT)
	{
		return this->intValue;
	}
	else
	{
		return 0;
	}
}

UnionValue::operator std::basic_string<TCHAR>()
{
	if(this->dataType == UnionValue::DATATYPE::TSTRING)
	{
		return this->stringValue;
	}
	else
	{
		return std::basic_string<TCHAR>();
	}
}

RibbonApplication::RibbonApplication(HWND hWnd):
cRef(1),
hWnd(hWnd)
{
	this->controlsEnabled.fill(FALSE); //0. Project, 1. Object, 2. CSG nodes

	std::vector<UnionValue> tmp;
	this->mutableControls.insert({_T("objectName"), tmp});
	this->mutableControls.insert({_T("sceneModeType"), tmp});
	this->mutableControls.insert({_T("transformationType"), tmp});
	this->mutableControls.insert({_T("transformationAxis"), tmp});
	this->mutableControls.insert({_T("lightValues"), tmp});
	this->mutableControls.insert({_T("materialValues"), tmp});
	this->mutableControls.insert({_T("lightPosition"), tmp});
	this->mutableControls.insert({_T("csgTreeNames"), tmp});
	this->mutableControls.insert({_T("csgTreeNodes"), tmp});
}

RibbonApplication::~RibbonApplication()
{
}

STDMETHODIMP_(ULONG) RibbonApplication::Release()
{
	LONG cRef = InterlockedDecrement(&this->cRef);

	if(cRef == 0)
	{
		delete this;
	}

	return cRef;
}

STDMETHODIMP RibbonApplication::QueryInterface(_IN_(IID & iid), _OUT_(VOID** ppv))
{
	HRESULT hResult = RibbonFunctions::QueryInterface(iid, ppv, *this);

	if(SUCCEEDED(hResult))
	{
		this->AddRef();
	}

	return hResult;
}

STDMETHODIMP RibbonApplication::OnViewChanged(UINT32 nViewID, UI_VIEWTYPE typeID, IUnknown* pView, UI_VIEWVERB verb, INT32 uReasonCode)
{
	UNREFERENCED_PARAMETER(nViewID);
	UNREFERENCED_PARAMETER(pView);
	UNREFERENCED_PARAMETER(uReasonCode);

	HRESULT hResult = E_NOTIMPL;

	if(UI_VIEWTYPE_RIBBON == typeID)
	{
		switch (verb)
		{
			case UI_VIEWVERB_SIZE:
				{
					hResult = SendMessage(this->hWnd, WM_SIZE, 0, 0);
				}
				break;
		}
	}

	return hResult;
}

//for each control create its own handler
STDMETHODIMP RibbonApplication::OnCreateUICommand(UINT32 nCmdID, UI_COMMANDTYPE typeID, IUICommandHandler** ppCommandHandler)
{ 
	UNREFERENCED_PARAMETER(typeID);

	HRESULT hResult = E_NOTIMPL;

	switch(nCmdID)
	{
		case cmdMenuNewProject:
		case cmdMenuOpenProject:
		case cmdMenuExit:
		case cmdNewProject:
		case cmdOpenProject:
		case cmdShowShortcuts:
		case cmdShowLogFile:
		case cmdDatabaseConnect:
		case cmdHelp:
			{
				hResult = RibbonApplication::AddButton(ppCommandHandler, this->hWnd, TRUE);
			}
			break;

		case cmdSubtraction:
		case cmdIntersection:
		case cmdUnion:
		case cmdDatabaseDisconnect:
		case cmdDatabaseUpload:
		case cmdDatabaseDownload:
			{
				hResult = RibbonApplication::AddButton(ppCommandHandler, this->hWnd, FALSE);
			}
			break;

		case cmdMenuSaveAsProject:
		case cmdMenuSaveProject:
		case cmdSaveAsProject:
		case cmdSaveProject:
		case cmdObjectAdd:
		case cmdTreeDelete:
		case cmdCsgCalculate:
			{
				hResult = RibbonApplication::AddButton(ppCommandHandler, this->hWnd, this->controlsEnabled[0]);
			}
			break;

		case cmdObjectClone:
		case cmdObjectDelete:
		case cmdTextureAdd:
		case cmdNodeAdd:
			{
				hResult = RibbonApplication::AddButton(ppCommandHandler, this->hWnd, this->controlsEnabled[1]);
			}
			break;

		case cmdNodeRemove:
			{
				hResult = RibbonApplication::AddButton(ppCommandHandler, this->hWnd, this->controlsEnabled[2]);
			}
			break;
	//-----
		case cmdShowAxis:
		case cmdShowGrid:
			{
				hResult = RibbonApplication::AddCheckBox(ppCommandHandler, this->hWnd, this->controlsEnabled[0], TRUE);
			}
			break;
	//-----
		case cmdObjectName:
			{
				std::basic_string<TCHAR> objectName{_T("-")};
				this->GetValue<std::basic_string<TCHAR>>(_T("objectName"), objectName);

				std::vector<Property> propertySet;
				ExceptionHandler::push_back<Property>(propertySet, Property(objectName, UI_COLLECTION_INVALIDINDEX, 0, UI_COMMANDTYPE_UNKNOWN, Property::CONTENTTYPE::ITEM));

				hResult = RibbonApplication::AddComboBox(ppCommandHandler, this->hWnd, this->controlsEnabled[0], propertySet);
			}
			break;

		case cmdObjectValues:
			{
				std::vector<Property> propertySet;
				ExceptionHandler::push_back<Property>(propertySet, Property(std::basic_string<TCHAR>{_T("-")}, UI_COLLECTION_INVALIDINDEX, 0, UI_COMMANDTYPE_UNKNOWN, Property::CONTENTTYPE::ITEM));

				hResult = RibbonApplication::AddComboBox(ppCommandHandler, this->hWnd, this->controlsEnabled[0], propertySet);
			}
			break;

		case cmdSceneModeType:
			{
				INT selected = 1;
				this->GetValue<INT>(_T("sceneModeType"), selected);

				std::vector<Property> propertySet;
				ExceptionHandler::push_back<Property>(propertySet, Property(std::basic_string<TCHAR>{_T("Wire mode")}, 0, 0, UI_COMMANDTYPE_UNKNOWN, Property::CONTENTTYPE::CATEGORY));
				ExceptionHandler::push_back<Property>(propertySet, Property(std::basic_string<TCHAR>{_T("[1] wire")}, 0, 0, UI_COMMANDTYPE_UNKNOWN, Property::CONTENTTYPE::ITEM));
				ExceptionHandler::push_back<Property>(propertySet, Property(std::basic_string<TCHAR>{_T("Color mode")}, 1, 0, UI_COMMANDTYPE_UNKNOWN, Property::CONTENTTYPE::CATEGORY));
				ExceptionHandler::push_back<Property>(propertySet, Property(std::basic_string<TCHAR>{_T("[2] color")}, 1, 0, UI_COMMANDTYPE_UNKNOWN, Property::CONTENTTYPE::ITEM));
				ExceptionHandler::push_back<Property>(propertySet, Property(std::basic_string<TCHAR>{_T("Light mode")}, 2, 0, UI_COMMANDTYPE_UNKNOWN, Property::CONTENTTYPE::CATEGORY));
				ExceptionHandler::push_back<Property>(propertySet, Property(std::basic_string<TCHAR>{_T("[3] lighting")}, 2, 0, UI_COMMANDTYPE_UNKNOWN, Property::CONTENTTYPE::ITEM));
				ExceptionHandler::push_back<Property>(propertySet, Property(std::basic_string<TCHAR>{_T("Texture mode")}, 3, 0, UI_COMMANDTYPE_UNKNOWN, Property::CONTENTTYPE::CATEGORY));
				ExceptionHandler::push_back<Property>(propertySet, Property(std::basic_string<TCHAR>{_T("[4] texture")}, 3, 0, UI_COMMANDTYPE_UNKNOWN, Property::CONTENTTYPE::ITEM));
				ExceptionHandler::push_back<Property>(propertySet, Property(std::basic_string<TCHAR>{_T("Csg mode")}, 4, 0, UI_COMMANDTYPE_UNKNOWN, Property::CONTENTTYPE::CATEGORY));
				ExceptionHandler::push_back<Property>(propertySet, Property(std::basic_string<TCHAR>{_T("[5] boolean operations")}, 4, 0, UI_COMMANDTYPE_UNKNOWN, Property::CONTENTTYPE::ITEM));

				hResult = RibbonApplication::AddComboBox(ppCommandHandler, this->hWnd, this->controlsEnabled[0], propertySet, selected);
			}
			break;

		case cmdTransformationType:
			{
				INT selected = 0;
				this->GetValue<INT>(_T("transformationType"), selected);

				std::vector<Property> propertySet;
				ExceptionHandler::push_back<Property>(propertySet, Property(std::basic_string<TCHAR>{_T("translate")}, UI_COLLECTION_INVALIDINDEX, 0, UI_COMMANDTYPE_UNKNOWN, Property::CONTENTTYPE::ITEM));
				ExceptionHandler::push_back<Property>(propertySet, Property(std::basic_string<TCHAR>{_T("scale")}, UI_COLLECTION_INVALIDINDEX, 0, UI_COMMANDTYPE_UNKNOWN, Property::CONTENTTYPE::ITEM));
				ExceptionHandler::push_back<Property>(propertySet, Property(std::basic_string<TCHAR>{_T("rotate")}, UI_COLLECTION_INVALIDINDEX, 0, UI_COMMANDTYPE_UNKNOWN, Property::CONTENTTYPE::ITEM));

				hResult = RibbonApplication::AddComboBox(ppCommandHandler, this->hWnd, this->controlsEnabled[0], propertySet, selected);
			}
			break;

		case cmdLightMaterialValues:
			{
				std::vector<std::basic_string<TCHAR>> lightValues;
				ExceptionHandler::push_back<std::basic_string<TCHAR>>(lightValues, std::basic_string<TCHAR>{_T("r=1.00 g=1.00 b=1.00 a=1.00 (ambient)")});
				ExceptionHandler::push_back<std::basic_string<TCHAR>>(lightValues, std::basic_string<TCHAR>{_T("r=1.00 g=1.00 b=1.00 a=1.00 (diffuse)")});
				ExceptionHandler::push_back<std::basic_string<TCHAR>>(lightValues, std::basic_string<TCHAR>{_T("r=1.00 g=1.00 b=1.00 a=1.00 (specular)")});
				this->GetValues<std::basic_string<TCHAR>>(_T("lightValues"), lightValues);

				std::vector<std::basic_string<TCHAR>> materialValues;
				this->GetValues<std::basic_string<TCHAR>>(_T("materialValues"), materialValues);

				std::vector<Property> propertySet;
				ExceptionHandler::push_back<Property>(propertySet, Property(std::basic_string<TCHAR>{_T("Light and material")}, 0, 0, UI_COMMANDTYPE_UNKNOWN, Property::CONTENTTYPE::CATEGORY));
				ExceptionHandler::push_back<Property>(propertySet, Property(std::basic_string<TCHAR>{_T("")}, 0, 0, UI_COMMANDTYPE_UNKNOWN, Property::CONTENTTYPE::ITEM));
				ExceptionHandler::push_back<Property>(propertySet, Property(std::basic_string<TCHAR>{_T("Light")}, 1, 0, UI_COMMANDTYPE_UNKNOWN, Property::CONTENTTYPE::CATEGORY));

				for(std::vector<std::basic_string<TCHAR>>::const_iterator i = lightValues.begin(), j = lightValues.end(); i != j; i++)
				{
					ExceptionHandler::push_back<Property>(propertySet, Property(*i, 1, 0, UI_COMMANDTYPE_UNKNOWN, Property::CONTENTTYPE::ITEM));
				}

				ExceptionHandler::push_back<Property>(propertySet, Property(std::basic_string<TCHAR>{_T("Material")}, 2, 0, UI_COMMANDTYPE_UNKNOWN, Property::CONTENTTYPE::CATEGORY));

				for(std::vector<std::basic_string<TCHAR>>::const_iterator i = materialValues.begin(), j = materialValues.end(); i != j; i++)
				{
					ExceptionHandler::push_back<Property>(propertySet, Property(*i, 2, 0, UI_COMMANDTYPE_UNKNOWN, Property::CONTENTTYPE::ITEM));
				}

				hResult = RibbonApplication::AddComboBox(ppCommandHandler, this->hWnd, this->controlsEnabled[0], propertySet);
			}
			break;

		case cmdLightPosition:
			{
				std::basic_string<TCHAR> lightPosition{_T("x=0.0000 y=0.0000 z=0.0000")};
				this->GetValue<std::basic_string<TCHAR>>(_T("lightPosition"), lightPosition);

				std::vector<Property> propertySet;
				ExceptionHandler::push_back<Property>(propertySet, Property(std::basic_string<TCHAR>{_T("")}, 0, 0, UI_COMMANDTYPE_UNKNOWN, Property::CONTENTTYPE::ITEM));
				ExceptionHandler::push_back<Property>(propertySet, Property(std::basic_string<TCHAR>{_T("Light")}, 1, 0, UI_COMMANDTYPE_UNKNOWN, Property::CONTENTTYPE::CATEGORY));
				ExceptionHandler::push_back<Property>(propertySet, Property(lightPosition, 1, 0, UI_COMMANDTYPE_UNKNOWN, Property::CONTENTTYPE::ITEM));

				hResult = RibbonApplication::AddComboBox(ppCommandHandler, this->hWnd, this->controlsEnabled[0], propertySet);
			}
			break;

		case cmdCsgTreeNames:
			{
				std::vector<std::basic_string<TCHAR>> csgTreeNames;
				ExceptionHandler::push_back<std::basic_string<TCHAR>>(csgTreeNames, std::basic_string<TCHAR>{_T("default")});
				this->GetValues<std::basic_string<TCHAR>>(_T("csgTreeNames"), csgTreeNames);

				std::vector<Property> propertySet;
				for(std::vector<std::basic_string<TCHAR>>::const_iterator i = csgTreeNames.begin(), j = csgTreeNames.end(); i != j; i++)
				{
					ExceptionHandler::push_back<Property>(propertySet, Property(*i, UI_COLLECTION_INVALIDINDEX, 0, UI_COMMANDTYPE_UNKNOWN, Property::CONTENTTYPE::ITEM));
				}

				hResult = RibbonApplication::AddComboBox(ppCommandHandler, this->hWnd, this->controlsEnabled[0], propertySet);
			}
			break;

		case cmdCsgTreeNodes:
			{
				std::vector<std::basic_string<TCHAR>> csgTreeNodes;
				this->GetValues<std::basic_string<TCHAR>>(_T("csgTreeNodes"), csgTreeNodes);

				std::vector<Property> propertySet;
				for(std::vector<std::basic_string<TCHAR>>::const_iterator i = csgTreeNodes.begin(), j = csgTreeNodes.end(); i != j; i++)
				{
					ExceptionHandler::push_back<Property>(propertySet, Property(*i, UI_COLLECTION_INVALIDINDEX, 0, UI_COMMANDTYPE_UNKNOWN, Property::CONTENTTYPE::ITEM));
				}

				hResult = RibbonApplication::AddComboBox(ppCommandHandler, this->hWnd, this->controlsEnabled[2], propertySet);
			}
			break;
	//-----
		case cmdCsgAlgorithm:
		case cmdDepthAlgorithm:
		case cmdOffscreenType:
		case cmdOptimization:
			{
				hResult = RibbonApplication::AddDropDownButton(ppCommandHandler, this->controlsEnabled[0]);
			}
			break;

		case cmdObjectType:
			{
				std::vector<Property> propertySet;
				ExceptionHandler::push_back<Property>(propertySet, Property(std::basic_string<TCHAR>{_T("3D Objects")}, 1, 0, UI_COMMANDTYPE_UNKNOWN, Property::CONTENTTYPE::CATEGORY));
				ExceptionHandler::push_back<Property>(propertySet, Property(std::basic_string<TCHAR>{_T("")}, 1, cmd3Dcube, UI_COMMANDTYPE_BOOLEAN, Property::CONTENTTYPE::COMMAND));
				ExceptionHandler::push_back<Property>(propertySet, Property(std::basic_string<TCHAR>{_T("")}, 1, cmd3Dsphere, UI_COMMANDTYPE_BOOLEAN, Property::CONTENTTYPE::COMMAND));
				ExceptionHandler::push_back<Property>(propertySet, Property(std::basic_string<TCHAR>{_T("")}, 1, cmd3Dcylinder, UI_COMMANDTYPE_BOOLEAN, Property::CONTENTTYPE::COMMAND));
				ExceptionHandler::push_back<Property>(propertySet, Property(std::basic_string<TCHAR>{_T("")}, 1, cmd3Dcone, UI_COMMANDTYPE_BOOLEAN, Property::CONTENTTYPE::COMMAND));
				ExceptionHandler::push_back<Property>(propertySet, Property(std::basic_string<TCHAR>{_T("")}, 1, cmd3Dpyramid, UI_COMMANDTYPE_BOOLEAN, Property::CONTENTTYPE::COMMAND));
				ExceptionHandler::push_back<Property>(propertySet, Property(std::basic_string<TCHAR>{_T("")}, 1, cmd3Dtorus, UI_COMMANDTYPE_BOOLEAN, Property::CONTENTTYPE::COMMAND));

				hResult = RibbonApplication::AddDropDownGallery(ppCommandHandler, this->controlsEnabled[0], propertySet);
			}
			break;

		case cmdLightMaterial:
			{
				std::vector<Property> propertySet;
				ExceptionHandler::push_back<Property>(propertySet, Property(std::basic_string<TCHAR>{_T("Light")}, 0, 0, UI_COMMANDTYPE_UNKNOWN, Property::CONTENTTYPE::CATEGORY));
				ExceptionHandler::push_back<Property>(propertySet, Property(std::basic_string<TCHAR>{_T("")}, 0, cmdAmbientLight, UI_COMMANDTYPE_BOOLEAN, Property::CONTENTTYPE::COMMAND));
				ExceptionHandler::push_back<Property>(propertySet, Property(std::basic_string<TCHAR>{_T("")}, 0, cmdDiffuseLight, UI_COMMANDTYPE_BOOLEAN, Property::CONTENTTYPE::COMMAND));
				ExceptionHandler::push_back<Property>(propertySet, Property(std::basic_string<TCHAR>{_T("")}, 0, cmdSpecularLight, UI_COMMANDTYPE_BOOLEAN, Property::CONTENTTYPE::COMMAND));
				ExceptionHandler::push_back<Property>(propertySet, Property(std::basic_string<TCHAR>{_T("Material")}, 1, 0, UI_COMMANDTYPE_UNKNOWN, Property::CONTENTTYPE::CATEGORY));
				ExceptionHandler::push_back<Property>(propertySet, Property(std::basic_string<TCHAR>{_T("")}, 1, cmdAmbientMaterial, UI_COMMANDTYPE_BOOLEAN, Property::CONTENTTYPE::COMMAND));
				ExceptionHandler::push_back<Property>(propertySet, Property(std::basic_string<TCHAR>{_T("")}, 1, cmdDiffuseMaterial, UI_COMMANDTYPE_BOOLEAN, Property::CONTENTTYPE::COMMAND));
				ExceptionHandler::push_back<Property>(propertySet, Property(std::basic_string<TCHAR>{_T("")}, 1, cmdSpecularMaterial, UI_COMMANDTYPE_BOOLEAN, Property::CONTENTTYPE::COMMAND));
				ExceptionHandler::push_back<Property>(propertySet, Property(std::basic_string<TCHAR>{_T("")}, 1, cmdEmissiveMaterial, UI_COMMANDTYPE_BOOLEAN, Property::CONTENTTYPE::COMMAND));

				hResult = RibbonApplication::AddDropDownGallery(ppCommandHandler, this->controlsEnabled[0], propertySet);
			}
			break;
	//-----
		case cmdTransformationStep:
			{
				hResult = RibbonApplication::AddSpinner(ppCommandHandler, this->hWnd, this->controlsEnabled[0], 1, 100, 1);
			}
			break;

		case cmdLightSource:
			{
				hResult = RibbonApplication::AddSpinner(ppCommandHandler, this->hWnd, FALSE, 1, 1, 1);
			}
			break;
	//-----
		case cmd3Dcube:
		case cmd3Dsphere:
		case cmd3Dcylinder:
		case cmdAutomatic:
		case cmdGoldfeather:
		case cmdOcclusionQuery:
		case cmdDepthComplexitySampling:
		case cmdFrameBufferObject:
		case cmdPBuffer:
		case cmdFrameBufferObjectARB:
		case cmdFrameBufferObjectEXT:
		case cmdOptimizationDefault:
		case cmdOptimizationOn:
			{
				hResult = RibbonApplication::AddToggleButton(ppCommandHandler, this->hWnd, TRUE, FALSE);
			}
			break;

		case cmd3Dcone:
		case cmd3Dpyramid:
		case cmd3Dtorus:
			{
				hResult = RibbonApplication::AddToggleButton(ppCommandHandler, this->hWnd, FALSE, FALSE);
			}
			break;

		case cmdTransformationXAxis:
		case cmdTransformationYAxis:
		case cmdTransformationZAxis:
			{
				INT axis = cmdTransformationXAxis;
				this->GetValue<INT>(_T("transformationAxis"), axis, cmdTransformationZAxis == nCmdID);

				hResult = RibbonApplication::AddToggleButton(ppCommandHandler, this->hWnd, this->controlsEnabled[0], axis == nCmdID);
			}
			break;

		case cmdAmbientLight:
		case cmdDiffuseLight:
		case cmdSpecularLight:
			{
				hResult = RibbonApplication::AddToggleButton(ppCommandHandler, this->hWnd, this->controlsEnabled[0], FALSE);
			}
			break;

		case cmdAmbientMaterial:
		case cmdDiffuseMaterial:
		case cmdSpecularMaterial:
		case cmdEmissiveMaterial:
			{
				hResult = RibbonApplication::AddToggleButton(ppCommandHandler, this->hWnd, this->controlsEnabled[1], FALSE);
			}
			break;

		case cmdSCS:
		case cmdNoDepthComplexitySampling:
		case cmdAutomaticOffscreenType:
		case cmdOptimizationOff:
			{
				hResult = RibbonApplication::AddToggleButton(ppCommandHandler, this->hWnd, TRUE, TRUE);
			}
			break;
	}

	return hResult;
}

STDMETHODIMP RibbonApplication::OnDestroyUICommand(UINT32 commandId, UI_COMMANDTYPE typeID, IUICommandHandler* pCommandHandler)
{
	UNREFERENCED_PARAMETER(commandId);
	UNREFERENCED_PARAMETER(typeID);
	UNREFERENCED_PARAMETER(pCommandHandler);

	return E_NOTIMPL; 
}

HRESULT RibbonApplication::AddButton(_OUT_(IUICommandHandler** ppCommandHandler), HWND hWnd, BOOL enabled)
{
	IUICommandHandler* handler = NULL;

	return RibbonApplication::AddControl(ppCommandHandler, RButtonHandler::CreateInstance(&handler, hWnd, enabled), &handler);
}

HRESULT RibbonApplication::AddCheckBox(_OUT_(IUICommandHandler** ppCommandHandler), HWND hWnd, BOOL enabled, BOOL defValue)
{
	IUICommandHandler* handler = NULL;

	return RibbonApplication::AddControl(ppCommandHandler, RCheckBoxHandler::CreateInstance(&handler, hWnd, enabled, defValue), &handler);
}

HRESULT RibbonApplication::AddComboBox(_OUT_(IUICommandHandler** ppCommandHandler), HWND hWnd, BOOL enabled, _IN_(std::vector<Property> & propertySet), USHORT selected)
{
	IUICommandHandler* handler = NULL;

	return RibbonApplication::AddControl(ppCommandHandler, RComboBoxHandler::CreateInstance(&handler, hWnd, enabled, propertySet, selected), &handler);
}

HRESULT RibbonApplication::AddDropDownButton(_OUT_(IUICommandHandler** ppCommandHandler), BOOL enabled)
{
	IUICommandHandler* handler = NULL;

	return RibbonApplication::AddControl(ppCommandHandler, RDropDownButtonHandler::CreateInstance(&handler, enabled), &handler);
}

HRESULT RibbonApplication::AddDropDownGallery(_OUT_(IUICommandHandler** ppCommandHandler), BOOL enabled, _IN_(std::vector<Property> & propertySet))
{
	IUICommandHandler* handler = NULL;

	return RibbonApplication::AddControl(ppCommandHandler, RDropDownGalleryHandler::CreateInstance(&handler, enabled, propertySet), &handler);
}

HRESULT RibbonApplication::AddSpinner(_OUT_(IUICommandHandler** ppCommandHandler), HWND hWnd, BOOL enabled, FLOAT minValue, FLOAT maxValue, FLOAT increment)
{
	IUICommandHandler* handler = NULL;

	return RibbonApplication::AddControl(ppCommandHandler, RSpinnerHandler::CreateInstance(&handler, hWnd, enabled, minValue, maxValue, increment), &handler);
}

HRESULT RibbonApplication::AddToggleButton(_OUT_(IUICommandHandler** ppCommandHandler), HWND hWnd, BOOL enabled, BOOL defValue)
{
	IUICommandHandler* handler = NULL;

	return RibbonApplication::AddControl(ppCommandHandler, RToggleButtonHandler::CreateInstance(&handler, hWnd, enabled, defValue), &handler);
}

HRESULT RibbonApplication::AddControl(_OUT_(IUICommandHandler** ppCommandHandler), HRESULT createResult, _OUT_(IUICommandHandler** control))
{
	HRESULT hResult = createResult;

	if(SUCCEEDED(hResult))
	{
		if(*control == NULL)
		{
			return E_POINTER;
		}

		hResult = (*control)->QueryInterface(IID_PPV_ARGS(ppCommandHandler));

		(*control)->Release();
	}

	return hResult;
}