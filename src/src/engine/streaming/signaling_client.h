#ifndef WEBSOCKET_CLIENT_H
#define WEBSOCKET_CLIENT_H

#include <functional>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio.hpp>
#include <string>
#include <thread>
#include <memory>
#include <deque>

class WebSocketClient {
public:
    // Type alias for our callback function on receiving a message.
    using MessageHandler = std::function<void(const std::string &message)>;

    // Singleton accessor.
    // You can optionally pass host, port, and target parameters (default values provided).
    static WebSocketClient& getInstance(const std::string &host = "localhost",
                                        const std::string &port = "8000",
                                        const std::string &target = "/ws/signaling");

    // Deleted copy constructor and assignment operator.
    WebSocketClient(const WebSocketClient&) = delete;
    WebSocketClient& operator=(const WebSocketClient&) = delete;

    // Public API methods.
    void connect();
    void sendMessage(const std::string &message);
    void close();
    void setMessageHandler(MessageHandler handler);

private:
    // Private constructor.
    WebSocketClient(const std::string &host,
                    const std::string &port,
                    const std::string &target);
    ~WebSocketClient();

    // Start an asynchronous read cycle.
    void doRead();

    // ASIO/Beast asynchronous operation handlers.
    void onResolve(const boost::system::error_code &ec,
                   boost::asio::ip::tcp::resolver::results_type results);
    void onConnect(const boost::system::error_code &ec,
                   boost::asio::ip::tcp::resolver::results_type::endpoint_type endpoint);
    void onHandshake(const boost::system::error_code &ec);
    void onWrite(const boost::system::error_code &ec, std::size_t bytes_transferred);
    void onRead(const boost::system::error_code &ec, std::size_t bytes_transferred);
    void doWrite();

    // Connection details.
    std::string host_;
    std::string port_;
    std::string target_;

    // Boost.Asio context and resolver.
    boost::asio::io_context io_context_;
    boost::asio::ip::tcp::resolver resolver_;

    // Boost.Beast WebSocket stream over an asynchronous TCP socket.
    boost::beast::websocket::stream<boost::asio::ip::tcp::socket> ws_;

    // Buffer for incoming messages.
    boost::beast::multi_buffer buffer_;

    // Thread for running the io_context.
    std::thread io_thread_;

    // Callback for received messages.
    MessageHandler messageHandler_;

    std::deque<std::string> writeQueue_;
};

#endif // WEBSOCKET_CLIENT_H
