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

#ifdef WAIC_EXPORTS
#define WAIC_API __declspec(dllexport)
#else
#define WAIC_API __declspec(dllimport)
#endif

extern "C"
{
	// Must be called first, before any other call.
	WAIC_API void Init();

	WAIC_API bool IsListening(const char* pDeviceName);

	/// <summary>
	/// Enable/Disable to listen to the given audio input using default audio output (eg. listen to a microphone using default speakers).
	/// <returns>True if operation succeeded, False if pDeviceName not found</returns>
	WAIC_API bool SetListenToAudioInputDevice(const char* pDeviceName, bool pListen);

	WAIC_API bool HasError();

	WAIC_API const char* GetErrors();

	// Must be called at the end of the program.
	WAIC_API void Terminate();
}