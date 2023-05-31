#include<stdio.h>
#include<windows.h>
#include"include/buffer.h"

#define BUF_SIZE 48*2*30
#define PBUF_SIZE 48*2*27
MMRESULT mmrt;
WAVEOUTCAPS woutdevs[5];
WAVEINCAPS windevs[5];

BYTE recordbuffer[BUF_SIZE]; 
BYTE recordbuffer2[BUF_SIZE];
BYTE playbuffer[PBUF_SIZE]; 
BYTE playbuffer2[PBUF_SIZE];  
WAVEHDR buf_header = {
    .lpData = recordbuffer,
    .dwBufferLength = BUF_SIZE,
    .dwBytesRecorded = 0,
    .dwFlags = 0,
    .dwUser = 0,
    .dwLoops = 1
};
WAVEHDR buf_header2 = {
    .lpData = recordbuffer2,
    .dwBufferLength = BUF_SIZE,
    .dwBytesRecorded = 0,
    .dwFlags = 0,
    .dwUser = 0,
    .dwLoops = 1
};
WAVEHDR pbuf_header = {
    .lpData = playbuffer,
    .dwBufferLength = PBUF_SIZE,
    .dwBytesRecorded = 0,
    .dwFlags = 0,
    .dwUser = 0,
    .dwLoops = 1
};
WAVEHDR pbuf_header2 = {
    .lpData = playbuffer2,
    .dwBufferLength = PBUF_SIZE,
    .dwBytesRecorded = 0,
    .dwFlags = 0,
    .dwUser = 0,
    .dwLoops = 1
};

WAVEFORMATEX recordform = {
    .wFormatTag = WAVE_FORMAT_PCM,
    .nChannels = 1,
    .nSamplesPerSec = 48000,
    .wBitsPerSample = 16,
    .cbSize = 0,
    .nBlockAlign = 1*16/8,
    .nAvgBytesPerSec = 48000*1*16/8
};
HWAVEIN wvin_hnd;
HWAVEOUT wvout_hnd;


int WaveIn_StopSign = 0;

void Checkmmrt(MMRESULT mmrt,const int line){
    if(mmrt == 0){
        //printf("success line %d\n",line);
    }else{
        printf("fail at line %d, error num: %d \n",line,mmrt);
    }
}

void CALLBACK waveInProc(HWAVEIN hWaveIn,UINT uMsg,DWORD dwInstance,DWORD dwParam1,DWORD dwParam2){
    LPWAVEHDR pdh = (LPWAVEHDR)dwParam1;
    BufferQueue *callb_q = (BufferQueue*)dwInstance;
    switch(uMsg){
        case WIM_OPEN:{
            break;
        }
        case WIM_DATA:{
            EnBufferQueue(callb_q,pdh->lpData,pdh->dwBytesRecorded);
            if(WaveIn_StopSign){
                printf("stop record dont add buffer back\n");
                break;
            }else{
                mmrt = waveInAddBuffer(wvin_hnd,pdh,sizeof(WAVEHDR));
                Checkmmrt(mmrt,__LINE__);
            }
            break;
        }
        case WIM_CLOSE:{
            break;
        }
    }
}
int count = 0;
void CALLBACK waveOutProc(HWAVEIN hWaveIn,UINT uMsg,DWORD dwInstance,DWORD dwParam1,DWORD dwParam2){
    LPWAVEHDR pdh = (LPWAVEHDR)dwParam1;
    BufferQueue *callpb_q = (BufferQueue*)dwInstance;
    switch(uMsg){
        case WOM_OPEN:{
            break;
        }
        case WOM_DONE:{
            if(callpb_q->size > PBUF_SIZE){
                DeBufferQueue(callpb_q,pdh->lpData,pdh->dwBufferLength);
                mmrt = waveOutWrite(wvout_hnd,pdh,sizeof(WAVEHDR));
                Checkmmrt(mmrt,__LINE__);
                FILE *f = fopen("record.pcm","ab+");
                fwrite(pdh->lpData,1,pdh->dwBufferLength,f);
                fclose(f);
            }else{
                printf("not enough buffer %d size left \n",callpb_q->size);
            }
            break;
        }
        case WOM_CLOSE:{
            break;
        }
    }
}

int main(int argc, char* argv[]){
    BufferQueue* q = initBufferQueue();
    PrintBufferQ(q);
    printf("rec & playback test start!! \n");
    UINT WaveInCount;
    WaveInCount = waveInGetNumDevs();
    printf("There are %d audio input devices \n",WaveInCount);
    printf("================device list================ \n");
    for(UINT id = 0; id < WaveInCount; id++){
        mmrt = waveInGetDevCaps(id,windevs+id,sizeof(WAVEINCAPS));
        printf("inputdevice %d : %s ; \n",id,windevs[id].szPname);
    }
    printf("================device list================ \n\n\n");

    UINT WaveOutCount;
    WaveOutCount = waveOutGetNumDevs();
    printf("There are %d audio output devices \n",WaveOutCount);
    printf("================device list================ \n");
    for(UINT id = 0; id < WaveOutCount; id++){
        mmrt = waveOutGetDevCaps(id,woutdevs+id,sizeof(WAVEOUTCAPS));
        printf("outdevice %d : %s ; \n",id,woutdevs[id].szPname);
    }
    printf("================device list end================ \n");

    UINT Choose_Outid = 0;
    mmrt = waveOutOpen(&wvout_hnd,Choose_Outid,&recordform,(DWORD)waveOutProc,(DWORD)q,CALLBACK_FUNCTION);
    Checkmmrt(mmrt,__LINE__);
    mmrt = waveOutPrepareHeader(wvout_hnd,&pbuf_header,sizeof(WAVEHDR));
    Checkmmrt(mmrt,__LINE__);
    mmrt = waveOutPrepareHeader(wvout_hnd,&pbuf_header2,sizeof(WAVEHDR));
    Checkmmrt(mmrt,__LINE__);

    UINT Choose_Inid = 0;
    mmrt = waveInOpen(&wvin_hnd,Choose_Inid,&recordform,(DWORD)waveInProc,(DWORD)q,CALLBACK_FUNCTION);
    Checkmmrt(mmrt,__LINE__);
    mmrt = waveInPrepareHeader(wvin_hnd,&buf_header,sizeof(WAVEHDR));
    Checkmmrt(mmrt,__LINE__);
    mmrt = waveInPrepareHeader(wvin_hnd,&buf_header2,sizeof(WAVEHDR));
    Checkmmrt(mmrt,__LINE__);
    mmrt = waveInAddBuffer(wvin_hnd,&buf_header,sizeof(WAVEHDR));
    Checkmmrt(mmrt,__LINE__);
    mmrt = waveInAddBuffer(wvin_hnd,&buf_header2,sizeof(WAVEHDR));
    Checkmmrt(mmrt,__LINE__);

    printf("Start recording from id:%d name:%s \n",Choose_Inid,windevs[Choose_Inid].szPname);
    mmrt = waveInStart(wvin_hnd);
    Checkmmrt(mmrt,__LINE__);
    Sleep(80);

    DeBufferQueue(q,pbuf_header.lpData,pbuf_header.dwBufferLength);
    DeBufferQueue(q,pbuf_header2.lpData,pbuf_header2.dwBufferLength);

    printf("Start playing from id:%d name:%s \n",Choose_Outid,woutdevs[Choose_Outid].szPname);
    mmrt = waveOutWrite(wvout_hnd,&pbuf_header,sizeof(WAVEHDR));
    Checkmmrt(mmrt,__LINE__);
    mmrt = waveOutWrite(wvout_hnd,&pbuf_header2,sizeof(WAVEHDR));
    Checkmmrt(mmrt,__LINE__);

    Sleep(18000);
    WaveIn_StopSign = 1;
    
    printf("Reset recording~~\n");
    mmrt = waveInReset(wvin_hnd);
    Checkmmrt(mmrt,__LINE__);
    
    Sleep(3000);
    printf("Stop recording~~\n");
    mmrt = waveInStop(wvin_hnd);
    Checkmmrt(mmrt,__LINE__);
    Sleep(2000);
    mmrt = waveInUnprepareHeader(wvin_hnd,&buf_header,sizeof(WAVEHDR));
    Checkmmrt(mmrt,__LINE__);
    mmrt = waveInUnprepareHeader(wvin_hnd,&buf_header2,sizeof(WAVEHDR));
    Checkmmrt(mmrt,__LINE__);
    mmrt = waveInClose(wvin_hnd);
    Checkmmrt(mmrt,__LINE__);

    
    Sleep(3000);

    printf("Stop playing\n");

    mmrt = waveOutUnprepareHeader(wvout_hnd,&pbuf_header,sizeof(WAVEHDR));
    Checkmmrt(mmrt,__LINE__);
    mmrt = waveOutUnprepareHeader(wvout_hnd,&pbuf_header2,sizeof(WAVEHDR));
    Checkmmrt(mmrt,__LINE__);
    mmrt = waveOutClose(wvout_hnd);
    Checkmmrt(mmrt,__LINE__);
    printf("end\n");

    

    return 0;
}