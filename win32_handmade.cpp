
#include <windows.h>
#include <stdint.h>
#include <xinput.h>
#include <DSound.h>
#include <math.h>
#include <time.h>

#pragma comment(lib, "XInput.lib") // Library containing necessary 360
#pragma comment(lib, "Dsound.lib") // Library containing necessary 360

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;


typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;



struct win32_offscreen_buffer{
     BITMAPINFO Info;
     void *Memory;
     int width;
     int height;
     int pitch;
     int bytesPerPixel;
};
static bool running;
static win32_offscreen_buffer backbuffer;
static LPDIRECTSOUNDBUFFER secondaryBuffer;

/*
//define xinput getstate
#define X_INPUT_GET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_STATE *pState)
typedef X_INPUT_GET_STATE(x_input_get_state);
X_INPUT_GET_STATE(XInputGetStateStub)
{
    return ERROR_DEVICE_NOT_CONNECTED;
}
static x_input_get_state *XInputGetState_ = XInputGetStateStub;

//define xinput set state
#define X_INPUT_SET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_VIBRATION *pVibration)
typedef X_INPUT_SET_STATE( x_input_set_state);
X_INPUT_SET_STATE(XInputSetStateStub)
{
    return ERROR_DEVICE_NOT_CONNECTED;
}
static x_input_set_state *XInputSetState_ = XInputSetStateStub;

#define XInputGetState XInputGetState_
#define XInputSetState XInputSetState_
*/

/*
static void LoadXInput() {
    HMODULE XInputLibrary = LoadLibrary("xinput1_3.dll");
    if(XInputLibrary)
    {
        XInputGetState = (x_input_get_state *)GetProcAddress(XInputLibrary, "XInputGetState");
        XInputSetState = (x_input_set_state *)GetProcAddress(XInputLibrary, "XInputSetState");
    }
}
*/
static void initSound(HWND windowHandle, int32 samplesPerSecond, int32 bufferSize){
    //load the Library
    HMODULE DSoundLibrary = LoadLibraryA("dsound.dll");
    if(DSoundLibrary)
    {
        LPDIRECTSOUND directSound;
        if(DirectSoundCreate(0, &directSound, 0) == DS_OK)
        {
               WAVEFORMATEX waveFormat = {};
               waveFormat.wFormatTag = WAVE_FORMAT_PCM;
               waveFormat.nChannels = 2;
               waveFormat.nSamplesPerSec = samplesPerSecond;
               waveFormat.wBitsPerSample = 16;
               waveFormat.nBlockAlign = (waveFormat.nChannels * waveFormat.wBitsPerSample) / 8 ;
               waveFormat.nAvgBytesPerSec = waveFormat.nSamplesPerSec * waveFormat.nBlockAlign;
               waveFormat.cbSize = 0;
               //HRESULT SetCooperativeLevel( HWND hwnd, DWORD dwLevel );
               if(directSound->SetCooperativeLevel(windowHandle, DSSCL_PRIORITY) == DS_OK)
               {
                    DSBUFFERDESC bufferDescription = {};
                    bufferDescription.dwSize = sizeof(bufferDescription);
                    bufferDescription.dwFlags = DSBCAPS_PRIMARYBUFFER;
                    LPDIRECTSOUNDBUFFER primaryBuffer;
                    if(SUCCEEDED(directSound->CreateSoundBuffer(&bufferDescription,&primaryBuffer,0)))
                    {

                         if(SUCCEEDED(primaryBuffer->SetFormat(&waveFormat)))
                         {
                         OutputDebugStringA("primary buffer format was set. \n");
                         }
                         else
                         {

                         }

                    }
                    else
                    {

                    }


               }
               else
               {

               }

               DSBUFFERDESC bufferDescription = {};
               bufferDescription.dwSize = sizeof(bufferDescription);
               bufferDescription.dwFlags = 0;
               bufferDescription.dwBufferBytes = bufferSize;
               bufferDescription.lpwfxFormat = &waveFormat;
               if(SUCCEEDED(directSound->CreateSoundBuffer(&bufferDescription,&secondaryBuffer,0)))
               {
                    OutputDebugStringA("secondary buffer was set. \n");
               }
        }
        else
        {

        }
    }
}
struct windows_dimension{
    int width;
    int height;
};
static windows_dimension getWindowDimension(HWND Window){
    windows_dimension result;
    RECT clientRect;
    GetClientRect(Window, &clientRect); // get the siz of the new window
    result.width = clientRect.right - clientRect.left;
    result.height = clientRect.bottom - clientRect.top;
    return result;
}

static void RenderWeirdGradient(win32_offscreen_buffer *buffer, int xOffset, int yOffset) {
    uint8 *row = (uint8 *)buffer->Memory; // starting address of the bitmap where we start to draw;
    //loop through each column
	for(int y = 0; y < buffer->height; y++)
    {
        uint32 *pixel = (uint32 *)row; // each pixel in the row;
        for(int x = 0; x < buffer->width; x++){
            uint8 blue = (x + xOffset);
            uint8 green = (y + yOffset);
            *pixel++ = (green << 8 | blue);
        }
        row += buffer->pitch;
    }
}

static void ResizeDIBSection(win32_offscreen_buffer *buffer, int width, int height){
    if(buffer->Memory){
        VirtualFree(buffer->Memory, 0, MEM_RELEASE);
    }
    //bitmap width and height equals to the windows width and height
    buffer->width = width;
    buffer->height = height;
    buffer->bytesPerPixel = 4;
    //create new bitmap with new size
    buffer->Info.bmiHeader.biSize = sizeof(buffer->Info.bmiHeader);
    buffer->Info.bmiHeader.biWidth = buffer->width;
    buffer->Info.bmiHeader.biHeight = -buffer->height;
    buffer->Info.bmiHeader.biPlanes = 1;
    buffer->Info.bmiHeader.biBitCount = 32;
    buffer->Info.bmiHeader.biCompression = BI_RGB;

    //allocate memory
    int bitmapMemorySize = (buffer->width * buffer->height) * buffer->bytesPerPixel;
    buffer->Memory = VirtualAlloc(0, bitmapMemorySize, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
    // the size of one pixel rows
    buffer->pitch = buffer->width * buffer->bytesPerPixel;
}

static void UpdateWindow(win32_offscreen_buffer *buffer,
                        HDC DeviceContext, int windowWidth, int windowHeight,
                        int X, int Y, int Width, int Height)
{
    //to do correct ratio
     StretchDIBits(DeviceContext,
                    /*X, Y, Width, Height,
                    X, Y, Width, Height,
                    */
                    0, 0, windowWidth, windowHeight,
                    0, 0, buffer->width, buffer->height,
                    buffer->Memory,
                    &buffer->Info,
                    DIB_RGB_COLORS,SRCCOPY);

}
LRESULT CALLBACK MainWindowCallback(
   HWND   window,       //handle to windows
   UINT   message,
   WPARAM wParam,
   LPARAM lParam)
{
    LRESULT result = 0;

    switch (message) {

        case WM_SIZE:{
            /*
                when the window resizes, get the width and height of the window
                pass to the ResizeDIBSection to create a new bitmap
                using the bitmap infomation to UpdateWindow in WM_PAINT
            */
        }break;
        case WM_DESTROY:{
            running = false;
        }break;
        case WM_SYSKEYUP:
        case WM_SYSKEYDOWN:
        case WM_KEYDOWN:
        case WM_KEYUP:{
            uint32 VKCode = wParam;  //distingish key
            bool wasDown = ((lParam & (1 << 30)) != 0); // if bit 30 is not 0, the key was down before (true)
            bool isDown = ((lParam & (1 << 31)) == 0);
            if(wasDown != isDown){


                if(VKCode =='W'){

                }
                else if(VKCode == 'A'){

                }
                else if(VKCode == 'S'){

                }
                else if(VKCode == 'D'){

                }
                else if(VKCode == 'Q'){

                }
                else if(VKCode == 'E'){

                }
                else if(VKCode == 'F'){

                }
                else if(VKCode == VK_UP){

                }
                else if(VKCode == VK_DOWN){

                }
                else if(VKCode == VK_LEFT){

                }
                else if(VKCode == VK_RIGHT){

                }
                else if(VKCode == VK_SPACE){

                }
                else if(VKCode == VK_LBUTTON){

                }
                else if(VKCode == VK_RBUTTON){

                }
                else if(VKCode == VK_ESCAPE){
                    OutputDebugStringA("ESCAPE: ");
                    if(isDown){
                        OutputDebugStringA("isdown");
                    }
                    if(wasDown){
                        OutputDebugStringA("wasdown");
                    }
                    OutputDebugStringA("\n");
                }

            }
            bool AltKeyWasDown = ((lParam & (1 << 29)) != 0); // if it is true

            if((VKCode == VK_F4) && AltKeyWasDown){
                    OutputDebugStringA("test");
                running = false;
            }
        }break;
        case WM_CLOSE:{
            running = false;
        }break;
        case WM_ACTIVATEAPP:{
            OutputDebugStringA("WM_ACTIVATEAPP\n");
        }break;
        case WM_PAINT: {
            PAINTSTRUCT paint;
            HDC DeviceContext = BeginPaint(window, &paint);
            int X = paint.rcPaint.left;
            int Y = paint.rcPaint.top;
            int Width = paint.rcPaint.right - paint.rcPaint.left;
            int Height = paint.rcPaint.bottom - paint.rcPaint.top;
            windows_dimension dimension = getWindowDimension(window);

            UpdateWindow(&backbuffer, DeviceContext, dimension.width, dimension.height, X, Y, Width, Height);
            EndPaint(window, &paint);
		}break;

        default:{
            //OutputDebugStringA("default\n");
            result = DefWindowProc(window, message, wParam, lParam);
        }break;
    }
    return result;
}
int CALLBACK wWinMain(
HINSTANCE instance,
HINSTANCE prevInstance,
LPSTR     commandLine,
int       showCode)
{
    //LoadXInput();
    //register window class
    WNDCLASSEX windowClass = {}; //define all parameters as 0


    //pass the size of the window to create new dib DIBSection
    ResizeDIBSection(&backbuffer, 1280, 720);

    windowClass.style = CS_OWNDC|CS_HREDRAW|CS_VREDRAW; //unsigned integer
    //window needs handle function
    windowClass.lpfnWndProc = MainWindowCallback;
    //window needs instance
    windowClass.hInstance = instance;
    //HICON     hIcon;
	windowClass.lpszClassName = "MyGame";
    if(RegisterClassEx(&windowClass)){
        HWND WindowHandle = CreateWindowExA(
           0,
           windowClass.lpszClassName,
           "Handmade hero",
           WS_OVERLAPPEDWINDOW|WS_VISIBLE,
           CW_USEDEFAULT,
           CW_USEDEFAULT,
           CW_USEDEFAULT,
           CW_USEDEFAULT,
           0,
           0,
           instance,
           0);
       if(WindowHandle){
           HDC DeviceContext = GetDC(WindowHandle);
           //graphics test
           int xOffset = 0;
           int yOffset = 0;
           //sound test
           int samplesPerSecond = 48000;
           int  toneHz= 256;
           int16 toneVolumn = 3000;
           uint32 runningSampleIndex = 0;
           int squareWavePeriod = samplesPerSecond / toneHz;
           int halfSquareWavePeriod = squareWavePeriod / 2;
           int bytePerSample = sizeof(int16)*2;
           int secondaryBufferSize = samplesPerSecond * bytePerSample;
           //direct sound
           initSound(WindowHandle, samplesPerSecond, secondaryBufferSize);

           secondaryBuffer->Play(0, 0, DSBPLAY_LOOPING);

           running = true;

           while(running){
               //put it in the while loop to avoid reference this variable
               MSG Message;

               while(PeekMessageA(&Message, 0, 0, 0, PM_REMOVE)){
                   if(Message.message == WM_QUIT){
                       running = false;
                   }

                   TranslateMessage(&Message);
                   DispatchMessage(&Message);
               }
               for (DWORD controllerIndex = 0; controllerIndex < XUSER_MAX_COUNT; controllerIndex++)
                {
                    //there is a bug of xinput getstate
                    XINPUT_STATE controllState;

                    if(XInputGetState(controllerIndex, &controllState) == ERROR_SUCCESS)
                    {
                        //this controller is plugged in
                        XINPUT_GAMEPAD *pad = &controllState.Gamepad;
                        bool up = (pad->wButtons & XINPUT_GAMEPAD_DPAD_UP);
                        bool down = (pad->wButtons & XINPUT_GAMEPAD_DPAD_DOWN);
                        bool left = (pad->wButtons & XINPUT_GAMEPAD_DPAD_LEFT);
                        bool right = (pad->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT);
                        bool start = (pad->wButtons & XINPUT_GAMEPAD_START);
                        bool back = (pad->wButtons & XINPUT_GAMEPAD_BACK);
                        bool leftShoulder = (pad->wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER);
                        bool rightShoulder = (pad->wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER);
                        bool AButton = (pad->wButtons & XINPUT_GAMEPAD_A);
                        bool BButton = (pad->wButtons & XINPUT_GAMEPAD_B);
                        bool XButton = (pad->wButtons & XINPUT_GAMEPAD_X);
                        bool YButton = (pad->wButtons & XINPUT_GAMEPAD_Y);

                        int16 stickX = pad->sThumbLX;
                        int16 stickY = pad->sThumbLY;

                        if(AButton)
                        {
                            xOffset += stickX >> 12;
                            yOffset += stickY >>12;

                        }
                    }
                    else
                    {
                        // the controller is not available
                    }
                }

                /*
                XINPUT_VIBRATION vibration;
                vibration.wLeftMotorSpeed = 40000;
                vibration.wRightMotorSpeed = 40000;
                XInputSetState(0, &vibration);
                */
               RenderWeirdGradient(&backbuffer, xOffset, yOffset);
               DWORD playCursor;
               DWORD writeCursor;
               if(SUCCEEDED(secondaryBuffer->GetCurrentPosition(&playCursor, &writeCursor)))
               {
                    //directsound output test
                    DWORD byteToLock = runningSampleIndex * bytePerSample % secondaryBufferSize;
                    WORD byteToWrite;
                    if(byteToLock > playCursor)
                    {
                         byteToWrite = secondaryBufferSize - byteToLock;
                         byteToWrite += playCursor;
                    }
                    else
                    {
                         byteToWrite = playCursor - byteToLock;
                    }


                    void *region1;
                    DWORD region1Size;
                    void *region2;
                    DWORD region2Size;

                    if(SUCCEEDED(secondaryBuffer->Lock(
                              byteToLock,
                              byteToWrite,
                             &region1, &region1Size,
                             &region2, &region2Size,
                             0)))
                   {
                       DWORD region1SampleCount =region1Size / bytePerSample;
                       int16 *sampleOut = (int16 *)region1;


                       for(DWORD sampleIndex = 0;
                       sampleIndex < region1SampleCount;
                       sampleIndex++)
                       {

                            int16 sampleValue = (runningSampleIndex / halfSquareWavePeriod % 2) ? toneVolumn : -toneVolumn;
                            *sampleOut++ = sampleValue;
                            *sampleOut++ = sampleValue;
                            ++runningSampleIndex;

                       }
                       DWORD region2SampleCount =region2Size / bytePerSample;
                       sampleOut = (int16 *)region2;
                       for(DWORD sampleIndex = 0; sampleIndex < region2SampleCount; sampleIndex++)
                       {

                            int16 sampleValue = (runningSampleIndex / halfSquareWavePeriod % 2) ? toneVolumn : -toneVolumn;
                            *sampleOut++ = sampleValue;
                            *sampleOut++ = sampleValue;
                            ++runningSampleIndex;
                       }
                       secondaryBuffer->Unlock(region1,region1Size,region2,region2Size);

                   }

               }


               windows_dimension dim = getWindowDimension(WindowHandle);
               UpdateWindow(&backbuffer, DeviceContext, dim.width, dim.height, 0, 0, dim.width, dim.height);
               ReleaseDC(WindowHandle, DeviceContext);
               ++xOffset;

           }
       }
       else{
                 //TODO:logging
           }
     }
     else{
         //TODO:logging
     }

     return 0;
}
