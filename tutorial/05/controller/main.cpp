#include <cpp-pcp-client/connector/connector.hpp>  // Connector
#include <cpp-pcp-client/connector/errors.hpp>     // connection_config_error

#include  <leatherman/json_container/json_container.hpp>  // JsonContainer

#include <string>
#include <iostream>
#include <memory>  // unique_ptr

namespace Tutorial {

const std::string BROKER_URL { "wss://127.0.0.1:8142/pcp/" };

const std::string AGENT_CLIENT_TYPE { "tutorial_agent" };
const std::string CONTROLLER_CLIENT_TYPE { "tutorial_controller" };

const std::string CA   { "../../resources/controller_certs/ca.pem" };
const std::string CERT { "../../resources/controller_certs/crt.pem" };
const std::string KEY  { "../../resources/controller_certs/key.pem" };

const std::string REQUEST_SCHEMA_NAME { "tutorial_request_schema" };
const int MSG_TIMEOUT_S { 10 };

int main(int argc, char *argv[]) {
    std::unique_ptr<PCPClient::Connector> connector_ptr;

    // Connector constructor

    try {
        connector_ptr.reset(new PCPClient::Connector { BROKER_URL,
                                                       CONTROLLER_CLIENT_TYPE,
                                                       CA,
                                                       CERT,
                                                       KEY });
        std::cout << "Configured the connector\n";
    } catch (PCPClient::connection_config_error& e) {
        std::cout << "Failed to configure the PCP Connector: "
                  << e.what() << "\n";
        return 1;
    }

    // Connector::connect()

    int num_connect_attempts { 4 };

    try {
        connector_ptr->connect(num_connect_attempts);
    } catch (PCPClient::connection_config_error& e) {
        std::cout << "Failed to configure WebSocket: " << e.what() << "\n";
        return 2;
    } catch (PCPClient::connection_fatal_error& e) {
        std::cout << "Failed to connect to " << BROKER_URL << " after "
                  << num_connect_attempts << " attempts: " << e.what() << "\n";
        return 2;
    }

    // Connector::isConnected()

    if (connector_ptr->isConnected()) {
        std::cout << "Successfully connected to " << BROKER_URL << "\n";
    } else {
        std::cout << "The connection has dropped; we can't send anything\n";
        return 2;
    }

    // Connector::send() - specify message fields

    leatherman::json_container::JsonContainer track {
        "{\"artist\" : \"Captain Beefheart\"}" };
    leatherman::json_container::JsonContainer data_entries {};
    data_entries.set<leatherman::json_container::JsonContainer>("request", track);
    data_entries.set<std::string>("details", "please send some good music");

    std::vector<std::string> endpoints { "pcp://*/" + AGENT_CLIENT_TYPE };

    try {
        connector_ptr->send(endpoints,
                            REQUEST_SCHEMA_NAME,
                            MSG_TIMEOUT_S,
                            data_entries);
        std::cout << "Request message sent\n";
    } catch (PCPClient::connection_processing_error& e) {
        std::cout << "Failed to send the request message: " << e.what() << "\n";
        return 2;
    }

    return 0;
}

}  // namespace Tutorial

int main(int argc, char** argv) {
    return Tutorial::main(argc, argv);
}
