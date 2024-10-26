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

#pragma once

#include <string>
#include <map>

struct IMMDevice;
struct IMMDeviceEnumerator;

class WindowsAudioInput
{
private:
	WindowsAudioInput(IMMDevice* pAudioDevice);

public:
	~WindowsAudioInput();
	static WindowsAudioInput* Create(IMMDeviceEnumerator* pDeviceEnumerator, const char* pDeviceName);

	bool IsListening(bool& pIsListening)const;
	bool SetListen(bool pListen);

private:
	IMMDevice* _audioDevice;
};

class WindowsAudioInputsController
{
public:
	WindowsAudioInputsController();
	~WindowsAudioInputsController();

	inline bool HasError()const { return _hasError; }
	inline const char* GetErrors()const { return _errorsLog.c_str(); }
	
	bool IsListening(const char* pDeviceName);
	bool SetListenToAudioInputDevice(const char* pDeviceName, bool pListen);

private:
	WindowsAudioInput* _GetOrCreate(const char* pDeviceName);

private:
	bool _hasError;
	std::string _errorsLog;
	IMMDeviceEnumerator* _deviceEnumerator;
	std::map<std::string, WindowsAudioInput*> _audioInputs;
};