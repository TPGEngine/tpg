#include "signaling_client.h"
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <iostream>
#include <memory>
#include <string>

// The singleton accessor
WebSocketClient& WebSocketClient::getInstance(const std::string &host,
                                                const std::string &port,
                                                const std::string &target) {
    // Meyers singleton: initialized on first call.
    static WebSocketClient instance(host, port, target);
    return instance;
}

// Private constructor.
WebSocketClient::WebSocketClient(const std::string &host,
                                 const std::string &port,
                                 const std::string &target)
    : host_(host),
      port_(port),
      target_(target),
      resolver_(io_context_),
      ws_(io_context_) {}

// Destructor.
WebSocketClient::~WebSocketClient() {
    close();
    if (io_thread_.joinable())
        io_thread_.join();
}

// Connect to the remote WebSocket server.
void WebSocketClient::connect() {
    // Start an asynchronous resolve.
    resolver_.async_resolve(
        host_, port_,
        [this](const boost::system::error_code &ec,
               boost::asio::ip::tcp::resolver::results_type results) {
            onResolve(ec, results);
        });

    // Run the io_context on a separate thread.
    io_thread_ = std::thread([this]() { io_context_.run(); });
}

// onResolve: Called when the host name is resolved.
void WebSocketClient::onResolve(const boost::system::error_code &ec,
                                boost::asio::ip::tcp::resolver::results_type results) {
    if (ec) {
        std::cerr << "Resolve error: " << ec.message() << "\n";
        return;
    }

    boost::asio::async_connect(
        ws_.next_layer(), results,
        [this](const boost::system::error_code &ec,
               const boost::asio::ip::tcp::endpoint &endpoint) {
            onConnect(ec, endpoint);
        });
    
}

// onConnect: Called when a connection is established.
void WebSocketClient::onConnect(
    const boost::system::error_code &ec,
    boost::asio::ip::tcp::resolver::results_type::endpoint_type /*endpoint*/) {
    if (ec) {
        std::cerr << "Connect error: " << ec.message() << "\n";
        return;
    }

    // Perform the WebSocket handshake.
    ws_.async_handshake(host_, target_,
                        [this](const boost::system::error_code &ec) { onHandshake(ec); });
}

// onHandshake: Called when the WebSocket handshake has been completed.
void WebSocketClient::onHandshake(const boost::system::error_code &ec) {
    if (ec) {
        std::cerr << "Handshake error: " << ec.message() << "\n";
        return;
    }
    std::cout << "WebSocket connection established to " << host_ << "\n";

    // Start reading incoming messages.
    doRead();
}

// doRead: Initiate an asynchronous read operation.
void WebSocketClient::doRead() {
    ws_.async_read(
        buffer_,
        [this](const boost::system::error_code &ec, std::size_t bytes_transferred) {
            onRead(ec, bytes_transferred);
        });
}

// onRead: Called when a message is received.
void WebSocketClient::onRead(const boost::system::error_code &ec,
                             std::size_t bytes_transferred) {
    if (ec) {
        std::cerr << "Read error: " << ec.message() << "\n";
        return;
    }

    // Convert the buffer to a string.
    std::string message = boost::beast::buffers_to_string(buffer_.data());
    // Clear the buffer.
    buffer_.consume(bytes_transferred);

    // If a message handler is set, invoke it.
    if (messageHandler_) {
        messageHandler_(message);
    }

    // Continue reading for the next message.
    doRead();
}

// sendMessage: Asynchronously send a message over the WebSocket.
void WebSocketClient::sendMessage(const std::string &message) {
    ws_.async_write(
        boost::asio::buffer(message),
        [this](const boost::system::error_code &ec, std::size_t bytes_transferred) {
            onWrite(ec, bytes_transferred);
        });
}

// onWrite: Called when a write operation completes.
void WebSocketClient::onWrite(const boost::system::error_code &ec,
                              std::size_t /*bytes_transferred*/) {
    if (ec) {
        std::cerr << "Write error: " << ec.message() << "\n";
    }
    // Optionally add further handling here.
}

// close: Gracefully close the WebSocket connection.
void WebSocketClient::close() {
    boost::system::error_code ec;
    ws_.close(boost::beast::websocket::close_code::normal, ec);
    if (ec) {
        std::cerr << "Close error: " << ec.message() << "\n";
    }
    io_context_.stop();
}

// setMessageHandler: Set a callback that is called when a message is received.
void WebSocketClient::setMessageHandler(MessageHandler handler) {
    messageHandler_ = handler;
}
