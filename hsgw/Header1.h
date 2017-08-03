#pragma once 
class CSerialPortProcessor
{
public:
	CSerialPortProcessor(void);
	~CSerialPortProcessor(void);

	// 指定ポートをオープンし受信開始 
	void Start(LPCSTR comPortName, DCB* portConfig);
	// COM ポートを閉じ処理終了 
	void End();

	// 受信済みのデータを buffer に格納し buffer に格納したデータのバイト数を返す 
	DWORD GetReceivedData(LPVOID buffer);

	// buffer に格納されたデータを length バイト送信する 
	DWORD SendData(LPVOID buffer, int toWriteBytes);

private:
	static DWORD WINAPI ThreadFunc(LPVOID lpParametr);
	DWORD WINAPI ReceiveData();

	HANDLE myHComPort;
	HANDLE myHThread;    // スレッド用ハンドル 
	DWORD  myThreadId;   // スレッド ID 

	HANDLE myMutex;      // myReceivedData,myReceivedDataLength を排他するための Mutex 

	BYTE* myReceivedData;  // 受信用内部データ バッファ 
	DWORD myReceivedDataLength;  // myReceivedData に格納されている受信データのバイト数 
};
