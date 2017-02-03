// test_boost_asio_tcp_client.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

using boost::asio::ip::tcp;

class client
{
public:
	client(boost::asio::io_service& io_service,
		const std::string& server, const std::string& port)
		: resolver_(io_service),
		socket_(io_service)
	{
		// Start an asynchronous resolve to translate the server and service names
		// into a list of endpoints.
		this->server = server;
		this->port = port;

		tcp::resolver::query query(server, port);
		resolver_.async_resolve(query,
			boost::bind(&client::handle_resolve, this,
				boost::asio::placeholders::error,
				boost::asio::placeholders::iterator));
	}
	void re_init() {
		disconnect = false;
		bytes_received = 0;
		tcp::resolver::query query(server, port);
		resolver_.async_resolve(query,
			boost::bind(&client::handle_resolve, this,
				boost::asio::placeholders::error,
				boost::asio::placeholders::iterator));
	}
private:
	void handle_resolve(const boost::system::error_code& err,
		tcp::resolver::iterator endpoint_iterator)
	{
		if (!err)
		{
			std::cout << "handle_resolve...\n";
			// Attempt a connection to each endpoint in the list until we
			// successfully establish a connection.
			boost::asio::async_connect(socket_, endpoint_iterator,
				boost::bind(&client::handle_connect, this,
					boost::asio::placeholders::error));
		}
		else
		{
			std::cout << "Error: " << err.message() << "\n";
		}
	}

	void handle_connect(const boost::system::error_code& err)
	{
		if (!err)
		{
			std::cout << "handle_connect...\n";
			// The connection was successful. Send the request.
			boost::asio::async_write(socket_, request_,
				boost::bind(&client::handle_write_request, this,
					boost::asio::placeholders::error));
		}
		else
		{
			std::cout << "Error: " << err.message() << "\n";
		}
	}

	void handle_write_request(const boost::system::error_code& err)
	{
		if (!err)
		{
			std::cout << "handle_write_request:\n";
			// Read the response status line. The response_ streambuf will
			// automatically grow to accommodate the entire line. The growth may be
			// limited by passing a maximum size to the streambuf constructor.
			/*boost::asio::async_read_until(socket_, response_, "\r\n",
				boost::bind(&client::handle_read_status_line, this,
					boost::asio::placeholders::error));*/

			boost::asio::async_read(socket_, response_,
				boost::asio::transfer_at_least(1),
				boost::bind(&client::handle_read_response, this,
					boost::asio::placeholders::error));
		}
		else
		{
			std::cout << "Error: " << err.message() << "\n";
		}
	}

	void handle_read_response(const boost::system::error_code& err)
	{
		if (!err)
		{
			//std::cout << "handle_read_response:\n";
			// Write all of the data that has been read so far.
			bytes_received += response_.size();
			
			// Continue reading remaining data until EOF.
			if (!disconnect) 
			{
				boost::asio::async_read(socket_, response_,
					boost::asio::transfer_at_least(1),
					boost::bind(&client::handle_read_response, this,
						boost::asio::placeholders::error));
			}
			else {
				std::cout << bytes_received << " bytes received" << std::endl;
				socket_.close();
			}
		}
		else if (err != boost::asio::error::eof)
		{
			std::cout << "Error: " << err << "\n";
		}
	}

	tcp::resolver resolver_;
	tcp::socket socket_;
	boost::asio::streambuf request_;
	boost::asio::streambuf response_;
	std::string server;
	std::string port;

	long bytes_received{ 0 };

	bool disconnect{ false };

	public:
	long iteration{ 0 };

	void input_loop()
	{
		char c{ 0 };
		while (c != 'e') {
			if (c == 'd') {
				disconnect = true;
			}
			iteration++;
			std::cin >> c;
		}
	}
};


int main(int argc, char* argv[])
{
	try
	{
		boost::asio::io_service io_service;
		client c(io_service, "localhost", "10001");		
		
		boost::thread *t;

		boost::shared_ptr<boost::asio::io_service::work> work(new boost::asio::io_service::work(io_service));
		t = new boost::thread(boost::bind(&boost::asio::io_service::run, &io_service));
		t->detach();

		c.input_loop();

		io_service.stop();
		delete t;

		io_service.reset();
		c.re_init();

		boost::shared_ptr<boost::asio::io_service::work> work2(new boost::asio::io_service::work(io_service));
		t = new boost::thread(boost::bind(&boost::asio::io_service::run, &io_service));
		t->detach();

		c.input_loop();

		io_service.stop();
		delete t;
		//io_service.stop();
	}
	catch (std::exception& e)
	{
		std::cout << "Exception: " << e.what() << "\n";
	}

	return 0;
}

