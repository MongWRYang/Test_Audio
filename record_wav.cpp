#include<iostream>
#include<windows.h>
#include<mmsystem.h>

using namespace std;

#define bufsize 44100*2*1 //samplerate*bytespersample(16bit=2byte)*seconds

//MMRESULT 是mmsystem相關函數回傳值的變數
MMRESULT mmreturn;
//WAVEINCAPS 是儲存錄音input設備的結構體 (這邊用陣列儲存多個~)
WAVEINCAPS WaveInCapsArr[5];
//WAVEFORMATEX 定義音訊資料的格式的結構體
WAVEFORMATEX WaveFormatex;
//HWAVEIN 是一個被開啟的錄音input設備的句柄(handle)
//句柄是Windows作業系統用來標識被應用程式所建立或使用的物件的整數。其本質相當於帶有參照計數的智慧型指標
HWAVEIN hWaveIn_handle;

//用來選擇錄音裝置的ID
UINT WaveInDeviceID = 0;

//WAVEHDR是用來識別 音訊資料緩衝區(buffer)的 標頭header結構體 
//這個結構體會儲存緩衝區的一些資訊 裡面其中一個結構體的成員是一個指向buffer的指標(lpData)
WAVEHDR wave_header;
//設置buffer
BYTE sound_buffer[bufsize];


FILE* f;
int STOPSIGN = 0;

//設計callback函數
void CALLBACK waveInProc(HWAVEIN hWaveIn,UINT uMsg,DWORD dwInstance,DWORD dwParam1,DWORD dwParam2){
        cout << "callback called=======" << endl; 
        LPWAVEHDR phd = (LPWAVEHDR)dwParam1;
        cout << "dwParam1 address=" << dwParam1 << endl;
        switch(uMsg){
                case WIM_OPEN:{
                        cout << "WIM_OPEN called" << endl; 
                        break;
                }
                case WIM_DATA:{
                        cout << "buffer address=" << phd << endl;
                        cout << "recorded:"<< phd->dwBytesRecorded << endl;
                        /*
                        if(STOPSIGN == 1){
                                cout << "STOPSIGN inside!!" << endl;
                                return;
                        }
                        */
                        if(phd->dwBytesRecorded < phd->dwBufferLength){
                                cout << "not full return" << endl;
                                return;
                        }
                        FILE *f = fopen("file2.pcm","ab+");
                        fwrite(phd->lpData,1,phd->dwBytesRecorded,f);
                        fclose(f);
                        
                        mmreturn = waveInAddBuffer(hWaveIn_handle,phd,sizeof(WAVEHDR));
                        if(mmreturn != MMSYSERR_NOERROR){
                                cout << "waveInAddBuffer Error! Error num: " << mmreturn << endl;
                        }else{
                                 cout << "waveInAddBuffer Success! in callback" << endl;
                        }
                        break;
                }
                case WIM_CLOSE:{
                        cout << "WIM_CLOSE called" << endl; 
                        break;
                }
                default:{
                        break;
                }
        }
        cout << "callback end=======" << endl; 
}

int main(int argc, char* argv[]){
    
    cout << "this is record wavfile program" << endl;
    //獲取電腦上能用的錄音input裝置
    UINT WaveinCount;
    WaveinCount = waveInGetNumDevs();
    cout << "there are " << WaveinCount << " record devices on system" << endl;
    cout << "==========================" << endl;

    for (UINT id_in = 0; id_in < WaveinCount; id_in++){
        mmreturn = waveInGetDevCaps(id_in, &WaveInCapsArr[id_in], sizeof(WAVEINCAPS));
        if(mmreturn != MMSYSERR_NOERROR){
            cout << "waveInGetDevCaps Error! Error num: " << mmreturn << endl;
        }else{
            //cout << "waveInGetDevCaps Success!" << endl;
        }
        cout << "Record Device ID "<< id_in << " :   "<< WaveInCapsArr[id_in].szPname << endl;
    }

    cout << "==========================" << endl;

    //給創立的WaveFormatex結構體賦值 以定義我們要的錄音音訊格式
    WaveFormatex.wFormatTag = WAVE_FORMAT_PCM;
    WaveFormatex.nChannels = 1;
    WaveFormatex.nSamplesPerSec = 44100;
    WaveFormatex.wBitsPerSample = 16;
    WaveFormatex.cbSize = 0;
    WaveFormatex.nBlockAlign = 1*16/8;
    WaveFormatex.nAvgBytesPerSec = 44100*1*16/8;

    //啟動錄音設備的函數 參數: 給該設備用的handle 選定的設備ID WaveFormatex結構體 回傳函數 回傳參數 flag
    cout << "Selected Device Id: " << WaveInDeviceID << "  - " << WaveInCapsArr[WaveInDeviceID].szPname << endl;
    mmreturn = waveInOpen(&hWaveIn_handle, WaveInDeviceID, &WaveFormatex, (DWORD)waveInProc, (DWORD)NULL, CALLBACK_FUNCTION);
    if(mmreturn != MMSYSERR_NOERROR){
            cout << "waveInOpen Error! Error num: " << mmreturn << endl;
    }else{
            cout << "waveInOpen Success!" << endl;
    }
    
    //給header賦值
    wave_header.lpData = (LPSTR)sound_buffer;
    wave_header.dwBufferLength = bufsize;
    wave_header.dwBytesRecorded = 0;
    wave_header.dwFlags = 0;
    wave_header.dwUser = 0;
    wave_header.dwLoops = 1;
    
    //給錄音設備準備緩衝區 參數: 要添加緩衝的設備handle 緩衝區標頭的結構體地址 大小

    
    mmreturn = waveInPrepareHeader(hWaveIn_handle,&wave_header,sizeof(WAVEHDR));
    if(mmreturn != MMSYSERR_NOERROR){
            cout << "waveInPrepareHeader Error! Error num: " << mmreturn << endl;
    }else{
            cout << "waveInPrepareHeader Success!" << endl;
    }

    cout << "wave_header address=" << &wave_header << " prepared" << endl;


    //給錄音設備準備緩衝區 參數: 要添加緩衝的設備handle 緩衝區標頭的結構體地址 大小
    mmreturn = waveInAddBuffer(hWaveIn_handle,&wave_header,sizeof(WAVEHDR));
    if(mmreturn != MMSYSERR_NOERROR){
            cout << "waveInAddBuffer Error! Error num: " << mmreturn << endl;
    }else{
            cout << "waveInAddBuffer Success!" << endl;
    }

    //開始錄音
    mmreturn = waveInStart(hWaveIn_handle);
    if(mmreturn != MMSYSERR_NOERROR){
            cout << "waveInStart Error! Error num: " << mmreturn << endl;
    }else{
            cout << "waveInStart Success!" << endl;
    }

    Sleep(3300);
   
    //停止錄音
    cout << "Calling waveInStop" << endl;
    mmreturn = waveInStop(hWaveIn_handle);
    if(mmreturn != MMSYSERR_NOERROR){
            cout << "waveInStop Error! Error num: " << mmreturn << endl;
    }else{
            cout << "waveInStop Success!" << endl;
    }

    Sleep(1000);

    mmreturn = waveInUnprepareHeader(hWaveIn_handle, &wave_header, sizeof(WAVEHDR));
    if(mmreturn != MMSYSERR_NOERROR){
            cout << "waveInUnprepareHeader Error! Error num: " << mmreturn << endl;
            cout << "WAVERR_STILLPLAYING is " << WAVERR_STILLPLAYING << endl;
    }else{
            cout << "waveInUnprepareHeader Success!" << endl;
    }

    Sleep(500);

    mmreturn = waveInClose(hWaveIn_handle);
    if(mmreturn != MMSYSERR_NOERROR){
            cout << "waveInClose Error! Error num: " << mmreturn << endl;
    }else{
            cout << "waveInClose Success!" << endl;
    }


/*
    MMCKINFO wav_file_header1;
    MMCKINFO wav_file_header2;
    HMMIO wav_file;
    char wavfilename[20] = "wavfile.wav";
    wav_file = mmioOpenA((LPSTR)wavfilename, NULL, MMIO_CREATE|MMIO_WRITE|MMIO_EXCLUSIVE|MMIO_ALLOCBUF);

    if(wav_file == NULL){
        cout << "mmioOpen Fail!" << endl;
    }

    ZeroMemory(&wav_file_header1, sizeof(MMCKINFO));
    wav_file_header1.fccType=mmioFOURCC('W', 'A', 'V', 'E');

    mmreturn = mmioCreateChunk(wav_file ,&wav_file_header1, MMIO_CREATERIFF);
    if(mmreturn != MMSYSERR_NOERROR){
            cout << "mmioCreateChunk for wav_file_header1 Error! Error num: " << mmreturn << endl;
    }else{
            cout << "mmioCreateChunk for wav_file_header1 Success!" << endl;
    }

    

    ZeroMemory(&wav_file_header2, sizeof(MMCKINFO));
	wav_file_header2.ckid=mmioFOURCC('f', 'm', 't', ' ');
    wav_file_header2.cksize=sizeof(WAVEFORMATEX)-sizeof(WaveFormatex.cbSize);

    mmreturn = mmioCreateChunk(wav_file ,&wav_file_header2, 0);
    if(mmreturn != MMSYSERR_NOERROR){
            cout << "mmioCreateChunk for wav_file_header2 Error! Error num: " << mmreturn << endl;
    }else{
            cout << "mmioCreateChunk for wav_file_header2 Success!" << endl;
    }
    
    long mmioWrite_return; 
    mmioWrite_return = mmioWrite(wav_file ,(char*)&WaveFormatex,wav_file_header2.cksize);
    if(mmioWrite_return == -1){
            cout << "mmioWrite Error! Error num: " << mmioWrite_return << endl;
    }else{
            cout << "mmioWrite Success!  Write " << mmioWrite_return << " Bytes"<< endl;
    }

    mmreturn = mmioAscend(wav_file,&wav_file_header2,0);
    if(mmreturn != MMSYSERR_NOERROR){
            cout << "mmioAscend for wav_file_header2 Error! Error num: " << mmreturn << endl;
    }else{
            cout << "mmioAscend for wav_file_header2 Success!" << endl;
    }

    wav_file_header2.ckid=mmioFOURCC('d', 'a', 't', 'a');

    mmreturn = mmioCreateChunk(wav_file ,&wav_file_header2,0);
    if(mmreturn != MMSYSERR_NOERROR){
            cout << "mmioCreateChunk for wav_file_header2 Error! Error num: " << mmreturn << endl;
    }else{
            cout << "mmioCreateChunk for wav_file_header2 Success!" << endl;
    }  

    cout << "Wave_header " << wave_header.dwBytesRecorded << " Bytes recorded" << endl;

    mmioWrite_return = mmioWrite(wav_file,(char*)wave_header.lpData,bufsize);
    if(mmioWrite_return == -1){
            cout << "mmioWrite Error! Error num: " << mmioWrite_return << endl;
    }else{
            cout << "mmioWrite Success!  Write " << mmioWrite_return << " Bytes"<< endl;
    }

    mmreturn = mmioAscend(wav_file,&wav_file_header2,0); 
    if(mmreturn != MMSYSERR_NOERROR){
            cout << "mmioAscend for wav_file_header2 Error! Error num: " << mmreturn << endl;
    }else{
            cout << "mmioAscend for wav_file_header2 Success!" << endl;
    }


    mmreturn = mmioAscend(wav_file,&wav_file_header1,0);
    if(mmreturn != MMSYSERR_NOERROR){
            cout << "mmioAscend for wav_file_header1 Error! Error num: " << mmreturn << endl;
    }else{
            cout << "mmioAscend for wav_file_header1 Success!" << endl;
    }

    
    mmreturn = mmioClose(wav_file,0);
    if(mmreturn==MMIOERR_CANNOTWRITE){
		cout << "mmioClose Fail" << endl;
	}else{
        cout << "mmioClose Success eric" << endl;
    }
*/

    return 0;
}