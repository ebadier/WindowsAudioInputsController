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

#include "framework.h"
#include "WindowsAudioInputsControllerC.h"

#include <iostream>

#define MICROPHONE_NAME "Microphone (HD Webcam C525)"
#define SPACE_KEYCODE 32
#define ESCAPE_KEYCODE 27

int main()
{
    Init();

    std::cout << "Press Space to start/stop listening to microphone: " << MICROPHONE_NAME << std::endl;
    std::cout << "Press Escape to exit." << std::endl;
    bool quit = false;
    while (!quit)
    {
        if (GetAsyncKeyState(SPACE_KEYCODE) & 0x8000)
        {
            bool isListening = IsListening(MICROPHONE_NAME);
            if (SetListenToAudioInputDevice(MICROPHONE_NAME, !isListening))
            {
                std::cout << "Listening to device " << MICROPHONE_NAME << (isListening ? " disabled" : " enabled") << ".\n";
            }
        }
        else if (GetAsyncKeyState(ESCAPE_KEYCODE) & 0x8000)
        {
            quit = true;
        }

        if (HasError())
        {
            std::cout << GetErrors() << std::endl;
        }

        Sleep(100);
    }

    Terminate();

    return EXIT_SUCCESS;
}