//
// Created by 吴凡 on 2017/3/16.
//


// asynchronous

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <iostream>

using boost::asio::ip::tcp;

std::string make_daytime_string() {
    using namespace std;
    time_t daytime = time(nullptr);
    return ctime(&daytime);
}

class tcp_connection : public boost::enable_shared_from_this<tcp_connection> {
public:
    using pointer = boost::shared_ptr<tcp_connection>;

    static pointer create(boost::asio::io_service& io_service) {
        return pointer(new tcp_connection(io_service));
    }

    tcp::socket& get_socket() {
        return socket_;
    }

    void start() {
        std::string message = make_daytime_string();
        boost::asio::async_write(socket_, boost::asio::buffer(message),
                                 boost::bind(&tcp_connection::handle_write,
                                             shared_from_this(),
                                             boost::asio::placeholders::error,
                                             boost::asio::placeholders::bytes_transferred));
    }
private:
    tcp_connection(boost::asio::io_service& io_service): socket_{io_service} {}
    void handle_write(const boost::system::error_code& /*error*/, size_t /*bytes_transferred*/) {}
    tcp::socket socket_;
};

class tcp_server {
public:
    tcp_server(boost::asio::io_service& io_service): acceptor_{io_service, tcp::endpoint(tcp::v4(), 13003)}{
        start_accept();
    }

private:
    void start_accept() {
        tcp_connection::pointer new_connection = tcp_connection::create(acceptor_.get_io_service());
        acceptor_.async_accept(new_connection->get_socket(),
                               boost::bind(&tcp_server::handle_accept, this,
                                           new_connection, boost::asio::placeholders::error));
    }

    void handle_accept(tcp_connection::pointer connection, const boost::system::error_code& error) {
        if(!error) {
            connection->start();
        }
        start_accept();
    }

    tcp::acceptor acceptor_;

};

int main() {
    try {
        boost::asio::io_service io_service;
        tcp_server server(io_service);
        io_service.run();
    } catch (std::exception &e) {
        std::cerr << e.what() << "\n";
    }
    return 0;
}

