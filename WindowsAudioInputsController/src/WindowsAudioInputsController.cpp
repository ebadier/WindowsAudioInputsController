/******************************************************************************************************************************************************
* MIT License																																		  *
*																																					  *
* Copyright (c) 2024																																  *
* Emmanuel Badier <emmanuel.badier@gmail.com>																										  *
* 																																					  *
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"),  *
* to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,  *
* and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:		  *
* 																																					  *
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.					  *
* 																																					  *
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, *
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 																							  *
* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, 		  *
* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.							  *
******************************************************************************************************************************************************/

#include <mmdeviceapi.h>
#include <audioclient.h>
#include <atlbase.h>
#include <functiondiscoverykeys_devpkey.h>
#include <iostream>

#include "WindowsAudioInputsController.h"

// Hardcoded values
const GUID LISTEN_SETTING_GUID = { 0x24DBB0FC, 0x9311, 0x4B3D, { 0x9C, 0xF0, 0x18, 0xFF, 0x15, 0x56, 0x39, 0xD4 } };
const int CHECKBOX_PID = 1;
const int LISTENING_DEVICE_PID = 0;

///////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////// HELPERS /////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////
void ToWString(const char* pInStr, std::wstring& pOutStr)
{
	int len = MultiByteToWideChar(CP_UTF8, 0, pInStr, -1, NULL, 0);
	if (len > 0)
	{
		pOutStr.resize(len - 1); // resize without the null terminator
		MultiByteToWideChar(CP_UTF8, 0, pInStr, -1, &pOutStr[0], len);
	}
	else
	{
		// Clear output if conversion fails
		pOutStr.clear();
	}
}

void GetDeviceID(IMMDevice* pDevice, std::wstring& pDeviceID)
{
	LPWSTR pwszID = NULL;
	HRESULT hr = pDevice->GetId(&pwszID);
	if (SUCCEEDED(hr))
	{
		pDeviceID = pwszID;
		CoTaskMemFree(pwszID);
	}
}

IMMDevice* GetDeviceByName(IMMDeviceEnumerator* pEnumerator, const std::wstring& pDeviceName)
{
	IMMDeviceCollection* pDeviceCollection = NULL;
	IMMDevice* pDevice = NULL;
	IPropertyStore* pProps = NULL;
	HRESULT hr = S_OK;

	// Obtenir tous les périphériques de capture audio (microphones)
	hr = pEnumerator->EnumAudioEndpoints(eCapture, DEVICE_STATE_ACTIVE, &pDeviceCollection);
	if (FAILED(hr)) return NULL;

	UINT deviceCount;
	pDeviceCollection->GetCount(&deviceCount);

	for (UINT i = 0; i < deviceCount; i++)
	{
		hr = pDeviceCollection->Item(i, &pDevice);
		if (FAILED(hr)) continue;

		// Récupérer le Property Store pour lire les propriétés du périphérique
		hr = pDevice->OpenPropertyStore(STGM_READ, &pProps);
		if (FAILED(hr)) continue;

		// Lire la propriété PKEY_Device_FriendlyName
		PROPVARIANT varName;
		PropVariantInit(&varName);
		hr = pProps->GetValue(PKEY_Device_FriendlyName, &varName);
		if (SUCCEEDED(hr))
		{
			// Comparer le nom du périphérique avec le nom cible
			if (pDeviceName == varName.pwszVal)
			{
				// Si les noms correspondent, nous avons trouvé notre périphérique
				pProps->Release();
				PropVariantClear(&varName);
				pDeviceCollection->Release();
				return pDevice;
			}
			PropVariantClear(&varName);
		}
		pProps->Release();
		pDevice->Release();
	}
	pDeviceCollection->Release();
	return NULL;
}

bool SetCheckboxListenToDeviceProperty(IPropertyStore* pPropertyStore, bool pEnableListen)
{
	HRESULT hr = E_FAIL;

	PROPERTYKEY checkboxPK;
	checkboxPK.fmtid = LISTEN_SETTING_GUID;
	checkboxPK.pid = CHECKBOX_PID;

	PROPVARIANT checkboxValue;
	PropVariantInit(&checkboxValue);
	checkboxValue.vt = VT_BOOL;
	checkboxValue.boolVal = pEnableListen ? VARIANT_TRUE : VARIANT_FALSE;

	hr = pPropertyStore->SetValue(checkboxPK, checkboxValue);
	hr &= PropVariantClear(&checkboxValue);

	return SUCCEEDED(hr);
}

bool GetCheckboxListenToDeviceProperty(IPropertyStore* pPropertyStore, bool& pIsListening)
{
	HRESULT hr = E_FAIL;
	pIsListening = false;

	PROPERTYKEY checkboxPK;
	checkboxPK.fmtid = LISTEN_SETTING_GUID;
	checkboxPK.pid = CHECKBOX_PID;

	PROPVARIANT checkboxValue;
	PropVariantInit(&checkboxValue);

	hr = pPropertyStore->GetValue(checkboxPK, &checkboxValue);
	if (SUCCEEDED(hr))
	{
		pIsListening = (checkboxValue.vt == VT_BOOL && checkboxValue.boolVal == VARIANT_TRUE);
		hr = PropVariantClear(&checkboxValue);
		return (SUCCEEDED(hr));
	}

	return false;
}

bool SetOutputDeviceListenToDeviceProperty(IPropertyStore* pPropertyStore, const std::wstring& pOutputDeviceID)
{
	HRESULT hr = E_FAIL;

	PROPERTYKEY devicePK;
	devicePK.fmtid = LISTEN_SETTING_GUID;
	devicePK.pid = LISTENING_DEVICE_PID;

	PROPVARIANT deviceValue;
	PropVariantInit(&deviceValue);

	if (!pOutputDeviceID.empty())
	{
		deviceValue.vt = VT_LPWSTR;
		deviceValue.pwszVal = const_cast<LPWSTR>(pOutputDeviceID.c_str());
	}
	else
	{
		deviceValue.vt = VT_EMPTY;
	}

	hr = pPropertyStore->SetValue(devicePK, deviceValue);
	hr &= PropVariantClear(&deviceValue);

	return SUCCEEDED(hr);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////// WindowsAudioInput ///////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////
WindowsAudioInput::WindowsAudioInput(IMMDevice* pAudioDevice): _audioDevice(pAudioDevice)
{

}

WindowsAudioInput::~WindowsAudioInput()
{
	if (_audioDevice != NULL)
	{
		_audioDevice->Release();
	}
}

WindowsAudioInput* WindowsAudioInput::Create(IMMDeviceEnumerator* pDeviceEnumerator, const char* pDeviceName)
{
	WindowsAudioInput* wai = NULL;
	std::wstring targetDeviceName;
	ToWString(pDeviceName, targetDeviceName);
	IMMDevice* audioDevice = GetDeviceByName(pDeviceEnumerator, targetDeviceName);
	if (audioDevice != NULL)
	{
		wai = new WindowsAudioInput(audioDevice);
	}
	return wai;
}

bool WindowsAudioInput::IsListening(bool& pIsListening) const
{
	pIsListening = false;
	CComPtr<IPropertyStore> propertyStore;
	HRESULT hr = _audioDevice->OpenPropertyStore(STGM_READ, &propertyStore);
	if (SUCCEEDED(hr)) 
	{
		return GetCheckboxListenToDeviceProperty(propertyStore, pIsListening);
	}
	return false;
}

bool WindowsAudioInput::SetListen(bool pListen)
{
	CComPtr<IPropertyStore> propertyStore;
	HRESULT hr = _audioDevice->OpenPropertyStore(STGM_READWRITE, &propertyStore);
	if (SUCCEEDED(hr))
	{
		// Set the "Listen to Device" checkbox
		bool sucess = SetCheckboxListenToDeviceProperty(propertyStore, pListen);
		sucess &= SetOutputDeviceListenToDeviceProperty(propertyStore, std::wstring()); // Set to default output device
		return sucess;
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////// WindowsAudioInputsController ////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////
WindowsAudioInputsController::WindowsAudioInputsController(): _hasError(false), _errorsLog(), _deviceEnumerator(NULL), _audioInputs()
{
	HRESULT hr = CoInitialize(NULL);
	if (SUCCEEDED(hr))
	{
		hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (void**)&_deviceEnumerator);
		if (SUCCEEDED(hr))
		{
			return;
		}
	}
	// FAIL
	_hasError = true;
	_errorsLog.append("[WindowsAudioInputsController] Initialization failed !\n");
}

WindowsAudioInputsController::~WindowsAudioInputsController()
{
	for (auto& it : _audioInputs)
	{
		delete it.second;
		it.second = NULL;
	}
	_audioInputs.clear();

	if (_deviceEnumerator != NULL)
	{
		_deviceEnumerator->Release();
	}

	CoUninitialize();
}

bool WindowsAudioInputsController::IsListening(const char* pDeviceName)
{
	bool isListening = false;
	WindowsAudioInput* audioInput = _GetOrCreate(pDeviceName);
	if (audioInput != NULL)
	{
		bool success = audioInput->IsListening(isListening);
		if (!success)
		{
			_hasError = true;
			_errorsLog.append("[WindowsAudioInputsController] IsListening(").append(pDeviceName).append(") failed !\n");
		}
	}
	return isListening;
}

bool WindowsAudioInputsController::SetListenToAudioInputDevice(const char* pDeviceName, bool pListen)
{
	WindowsAudioInput* audioInput = _GetOrCreate(pDeviceName);
	if (audioInput != NULL)
	{
		bool success = audioInput->SetListen(pListen);
		if (!success)
		{
			_hasError = true;
			_errorsLog.append("[WindowsAudioInputsController] SetListenToAudioInputDevice(")
				.append(pDeviceName).append(", ").append(pListen ? "true" : "false").append(") failed !\n");
		}
		return success;
	}
	return false;
}

WindowsAudioInput* WindowsAudioInputsController::_GetOrCreate(const char* pDeviceName)
{
	WindowsAudioInput* audioInput = NULL;
	if (_deviceEnumerator != NULL)
	{
		// Device already exists ?
		auto it = _audioInputs.find(pDeviceName);
		if (it != _audioInputs.end())
		{
			audioInput = it->second;
		}
		else
		{
			// Try to create it, if not existing
			audioInput = WindowsAudioInput::Create(_deviceEnumerator, pDeviceName);
			if (audioInput != NULL)
			{
				_audioInputs[pDeviceName] = audioInput;
			}
			else
			{
				_hasError = true;
				_errorsLog.append("[WindowsAudioInputsController] Audio device ").append(pDeviceName).append(" not found !\n");
			}
		}
	}
	else
	{
		_hasError = true;
		_errorsLog.append("[WindowsAudioInputsController] IMMDeviceEnumerator is null !\n");
	}
	return audioInput;
}
