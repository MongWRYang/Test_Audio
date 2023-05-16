#include<iostream>
#include<windows.h>
#include<mmsystem.h>

using namespace std;

#define bufsize 44100*2*5 //samplerate*bytespersample(16bit=2byte)*seconds

MMRESULT mmreturn;
WAVEINCAPS WaveInCapsArr[5];
WAVEFORMATEX WaveFormatex;
HWAVEIN hWaveIn_handle;

UINT WaveInDeviceID = 0;

WAVEHDR wave_header;
BYTE sound_buffer[bufsize];

int main(int argc, char* argv[]){
    
    cout << "this is record wavfile program" << endl;

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

    WaveFormatex.wFormatTag = WAVE_FORMAT_PCM;
    WaveFormatex.nChannels = 1;
    WaveFormatex.nSamplesPerSec = 44100;
    WaveFormatex.wBitsPerSample = 16;
    WaveFormatex.cbSize = 0;
    WaveFormatex.nBlockAlign = 1*16/8;
    WaveFormatex.nAvgBytesPerSec = 44100*1*16/8;

    cout << "Selected Device Id: " << WaveInDeviceID << "  - " << WaveInCapsArr[WaveInDeviceID].szPname << endl;
    mmreturn = waveInOpen(&hWaveIn_handle, WaveInDeviceID, &WaveFormatex, (DWORD)NULL, (DWORD)NULL, (DWORD)NULL);
    if(mmreturn != MMSYSERR_NOERROR){
            cout << "waveInOpen Error! Error num: " << mmreturn << endl;
    }else{
            cout << "waveInOpen Success!" << endl;
    }

    wave_header.lpData = (LPSTR)sound_buffer;
    wave_header.dwBufferLength = bufsize;
    wave_header.dwBytesRecorded = 0;
    wave_header.dwFlags = 0;
    wave_header.dwUser = 0;
    wave_header.dwLoops = 0;

    mmreturn = waveInPrepareHeader(hWaveIn_handle,&wave_header,sizeof(WAVEHDR));
    if(mmreturn != MMSYSERR_NOERROR){
            cout << "waveInPrepareHeader Error! Error num: " << mmreturn << endl;
    }else{
            cout << "waveInPrepareHeader Success!" << endl;
    }

    mmreturn = waveInAddBuffer(hWaveIn_handle,&wave_header,sizeof(WAVEHDR));
    if(mmreturn != MMSYSERR_NOERROR){
            cout << "waveInAddBuffer Error! Error num: " << mmreturn << endl;
    }else{
            cout << "waveInAddBuffer Success!" << endl;
    }

    mmreturn = waveInStart(hWaveIn_handle);
    if(mmreturn != MMSYSERR_NOERROR){
            cout << "waveInStart Error! Error num: " << mmreturn << endl;
    }else{
            cout << "waveInStart Success!" << endl;
    }

    Sleep(5000);

    mmreturn = waveInStop(hWaveIn_handle);
    if(mmreturn != MMSYSERR_NOERROR){
            cout << "waveInStop Error! Error num: " << mmreturn << endl;
    }else{
            cout << "waveInStop Success!" << endl;
    }

    mmreturn = waveInUnprepareHeader(hWaveIn_handle, &wave_header, sizeof(WAVEHDR));
    if(mmreturn != MMSYSERR_NOERROR){
            cout << "waveInUnprepareHeader Error! Error num: " << mmreturn << endl;
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

    MMCKINFO wav_file_header1;
    MMCKINFO wav_file_header2;
    HMMIO wav_file;
    char wavfilename[20] = "wavfile.wav";
    wav_file = mmioOpen((LPSTR)wavfilename, NULL, MMIO_CREATE|MMIO_WRITE|MMIO_EXCLUSIVE|MMIO_ALLOCBUF);

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


    return 0;
}