// RemoteControl.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <shellapi.h>

namespace
{
	const int HELLO_PORT = 50013;
	const char* HELLO_PORT_STR = "50013";
}
void asioTcpServer()
{
	try
	{
		boost::asio::io_service aios;
		boost::asio::ip::tcp::acceptor acceptor(aios,
			boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), HELLO_PORT));

		// just once
		{
			boost::asio::ip::tcp::socket socket(aios);
			acceptor.accept(socket);

			std::string message("Hello from RadarClient");
			boost::asio::write(socket, boost::asio::buffer(message));
		}
	}
	catch (std::exception& e)
	{
		std::cerr << "Exception: " << e.what() << std::endl;
	}
}

int main()
{
	asioTcpServer();
	ShellExecute(NULL, TEXT("open"), TEXT("C:\\share2\\Debug\\RadarClient.exe"), NULL, NULL, SW_SHOWDEFAULT);
    return 0;
}

