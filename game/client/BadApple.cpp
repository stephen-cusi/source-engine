#include "cbase.h"
#include "filesystem.h"
#include "tier0/memdbgon.h"

class AppleThread : public CThread
{
public:
    bool bIsRunning;
    
    int Run()
    {
        FileHandle_t file;
        file = g_pFullFileSystem->Open("scripts/BadApple.txt", "r");
        if (file != FILESYSTEM_INVALID_HANDLE)
        {
            int fileSize = g_pFullFileSystem->Size(file);
            char *buf = new char[fileSize + 1];
            g_pFullFileSystem->Read(buf, fileSize, file);
            const char *frame = strtok(buf, "newframe");
            bIsRunning = true;
            while (frame && bIsRunning)
            {
                Msg("%s\n",frame); //I used monospaced font for prining, otherwise the output'll be broken (because spaces symbol are less space than "#" symbol)
                Sleep(33);
                frame = strtok(NULL,"newframe");
            }
            
            delete [] buf; 
            g_pFullFileSystem->Close(file);
        }
        return 0;
    }
    
};

static AppleThread g_AppleThread;

CON_COMMAND(badapple,"badapple")
{    
engine->ClientCmd("play BadApple"); //I didn't care about the beauty of the audio output solution :/
if(!g_AppleThread.IsAlive())
        g_AppleThread.Start();
}

CON_COMMAND(badapple_stop,"badapple_stop")
{
    engine->ClientCmd("stopsound"); //It's bad decision and needs sv_cheats 1, but who cares?
    if(g_AppleThread.IsAlive())
    {
        g_AppleThread.bIsRunning = false;
        g_AppleThread.Stop();
        printf("Stopped thread!\n");
    }
}
