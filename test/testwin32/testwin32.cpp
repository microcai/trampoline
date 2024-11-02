// WinHttpExample.cpp : Defines the entry point for the console application.
//

#include <windows.h>
#include <winhttp.h>
#include <cstdio>
#include "trampoline.hpp"
#include <iostream>

template <auto fn>
struct ptr_deleter
{
    template <typename T>
    constexpr void operator()(T* arg) const
    {
        fn(arg);
    }
};

template <typename T, auto fn>
using auto_raii = std::unique_ptr<T, ptr_deleter<fn>>;

using WinHttpAutoHandle = auto_raii<void, [](HINTERNET h) {
	WinHttpCloseHandle(h);
}>;

int main(int argc, char* argv[])
{
	// WinHTTP Sessions Overview | https://msdn.microsoft.com/en-us/library/windows/desktop/aa384270(v=vs.85).aspx

	DWORD dwSize = 0;
	DWORD dwDownloaded = 0;
	std::string OutBuffer;
	BOOL  bResults = FALSE;
	HINTERNET  hSession = NULL,
		hConnect = NULL,
		hRequest = NULL;

	// Use WinHttpOpen to obtain a session handle.
	hSession = WinHttpOpen(L"WinHTTP Example/1.0",
		WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
		WINHTTP_NO_PROXY_NAME,
		WINHTTP_NO_PROXY_BYPASS, 0);

	auto status_cb = trampoline::c_function_ptr<WINHTTP_STATUS_CALLBACK>(
		[&](IN HINTERNET hInternet,
			IN DWORD_PTR dwContext,
			IN DWORD dwInternetStatus,
			IN LPVOID lpvStatusInformation,
			IN DWORD dwStatusInformationLength)
		{
			printf("http status change callback %d\n", dwInternetStatus);
		});

	WinHttpSetStatusCallback(hSession, status_cb, WINHTTP_CALLBACK_FLAG_ALL_NOTIFICATIONS, NULL);

	auto auto_close_session = WinHttpAutoHandle(hSession);

	// Specify an HTTP server.
	if (hSession)
		hConnect = WinHttpConnect(hSession, L"microcai.org",
			INTERNET_DEFAULT_HTTPS_PORT, 0);
	auto auto_close_hConnect = WinHttpAutoHandle(hConnect);



	// Create an HTTP request handle.
	if (hConnect)
		hRequest = WinHttpOpenRequest(hConnect, L"GET", L"/2024/10/31/c-closure-without-void-userdata.html",
			NULL, WINHTTP_NO_REFERER,
			WINHTTP_DEFAULT_ACCEPT_TYPES,
			WINHTTP_FLAG_SECURE);
	auto auto_close_hRequest = WinHttpAutoHandle(hRequest);


	// Send a request.
	if (hRequest)
		bResults = WinHttpSendRequest(hRequest,
			WINHTTP_NO_ADDITIONAL_HEADERS, 0,
			WINHTTP_NO_REQUEST_DATA, 0,
			0, 0);


	// End the request.
	if (bResults)
		bResults = WinHttpReceiveResponse(hRequest, NULL);

	// Keep checking for data until there is nothing left.
	if (bResults)
	{
		do
		{
			// Check for available data.
			dwSize = 0;
			if (!WinHttpQueryDataAvailable(hRequest, &dwSize))
				printf("Error %u in WinHttpQueryDataAvailable.\n",
					GetLastError());

			// Allocate space for the buffer.

			OutBuffer.resize(dwSize);

			if (!WinHttpReadData(hRequest, &OutBuffer[0],
				dwSize, &dwDownloaded))
				printf("Error %u in WinHttpReadData.\n", GetLastError());
			else
			{
				OutBuffer.resize(dwDownloaded);
				std::cout << OutBuffer;
				std::cout.flush();
			}

		} while (dwSize > 0);
	}


	// Report any errors.
	if (!bResults)
		printf("Error %d has occurred.\n", GetLastError());

	return 0;
}
