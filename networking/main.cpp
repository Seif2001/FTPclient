#include <iostream>
#include <asio.hpp>
#include <asio/ts/buffer.hpp>
#include <asio/ts/internet.hpp>
#include <fstream>

using namespace std;
struct Message {
    string header;
    string payload;
    string additionalContent;
    

    // Serialize the struct to a string
    string serialize() const {
        return header + ',' + payload + ',' + additionalContent;
    }

    static Message deserialize(const string& data) {
        Message message;
        size_t pos = data.find(',');
        if (pos != string::npos) {
            message.header = data.substr(0, pos);

            size_t nextPos = data.find(',', pos + 1);
            if (nextPos != string::npos) {
                message.payload = data.substr(pos + 1, nextPos - pos - 1);

                // Corrected the starting position for additionalContent
                pos = nextPos + 1;
                message.additionalContent = data.substr(pos);
            }
        }
        return message;
    }
};



vector<char> vBuffer(1 * 1024);


void handleFileTransfer(asio::ip::tcp::socket& socket, ofstream& outputFile) {
    std::vector<char> receivedData;
    std::error_code ec;

    while (true) {
        if (!ec) {
            size_t length = socket.read_some(asio::buffer(vBuffer.data(), vBuffer.size()), ec);

            if (length > 0) {
                cout << endl << "Received data from server: ";
                for (int i = 0; i < length; i++) {
                    cout << vBuffer[i];
                    receivedData.push_back(vBuffer[i]);
                }

                for (char c : receivedData) {
                    outputFile << c;
                }

                receivedData.clear();
            }
            else {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                break;
            }
        }
        else {
            break;
        }
    }
}


int main() {
    try {
        asio::error_code ec;
        asio::io_context context;

        asio::io_context::work idleWork(context);

        thread th = thread([&]() { context.run(); });

        asio::ip::tcp::endpoint endpoint(asio::ip::make_address("127.0.0.1", ec), 8888);

        asio::ip::tcp::socket socket(context);
        socket.connect(endpoint, ec);
        

        if (!ec) {
            cout << "Connected to " << endpoint.address() << " Port 8888" << endl;
            
            std::string uOd;
            while (uOd != "-1") {
                do {
                    cout << "Enter u for upload or d for download OR -1 to quit: ";
                    getline(cin, uOd);
                } while (uOd != "d" && uOd != "u" && uOd !="-1");
                if (uOd == "-1") {
                    break;
                }
                std::string add;

                std::string message;
                cout << "Enter the file directory: ";
                getline(cin, message);

                if (uOd == "u") {
                    cout << "Enter what you want to upload into " << message << endl;
                    getline(cin, add);
                }

                Message m = { uOd, message, add };

                asio::write(socket, asio::buffer(m.serialize() + '\n'), ec);
                cout << m.serialize()<<" has been sent successfully."<<endl;
                if (m.header == "d") {
                    ofstream outputFile(m.payload, ios::binary);
                    if (!outputFile.is_open()) {
                        cerr << "Error opening output file." << endl;
                        return 1;
                    }
                    handleFileTransfer(socket, outputFile);


                    outputFile.close();
                }
                
            }
            context.stop();
            if (th.joinable()) th.join();
            //this_thread::sleep_for(20000ms);
        }
        else {
            cout << "Connection failed " << ec.message() << endl;
        }
    }
    catch (const std::exception& e) {
        cerr << "Exception: " << e.what() << endl;
    }

    return 0;
}
