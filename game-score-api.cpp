#include <functional>
#include <iostream>
#include <string>
#include <vector>
#include <set>
#include <algorithm>
#include <ctime>
#include <random>
#include <sstream>
#include <iomanip>

// Third-Party Libraries
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#include <nlohmann/json.hpp>

#define PORT 9900

typedef websocketpp::server<websocketpp::config::asio> server;

struct User {
    std::string id;
    std::string game_id;
    websocketpp::connection_hdl hdl;
    std::string username;
    int score;
    int level;
    time_t created_at;
};

std::vector<User> users;

std::set<websocketpp::connection_hdl, std::owner_less<websocketpp::connection_hdl>> connection_list;


// RFC4122
std::string generateUUIDv4() {
    std::random_device rd;
    std::mt19937 rng(rd());
    std::uniform_int_distribution<std::uint32_t> uni(0, std::numeric_limits<std::uint32_t>::max());

    std::uint32_t randomData[4];
    for (int i = 0; i < 4; ++i) {
        randomData[i] = uni(rng);
    }

    randomData[1] = (randomData[1] & 0xFFFF0FFF) | 0x00004000; // set version to 0100, the version of the UUID is stored in bits 12 to 15 of the third group of 16 bits of the UUID
    randomData[2] = (randomData[2] & 0x3FFFFFFF) | 0x80000000; // set variant to 10xx, Combining the two operations, we are ensuring that the first two bits of randomData[2] are 10.

    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    ss << std::setw(8) << randomData[0] << '-';
    ss << std::setw(4) << ((randomData[1] & 0xFFFF0000) >> 16) << '-';
    ss << std::setw(4) << (randomData[1] & 0x0000FFFF) << '-';
    ss << std::setw(4) << ((randomData[2] & 0xFFFF0000) >> 16) << '-';
    ss << std::setw(4) << (randomData[2] & 0x0000FFFF);
    ss << std::setw(8) << randomData[3];

    return ss.str();
}

// New function to check if a username within a game_id exists
bool usernameExists(const std::string &game_id, const std::string &username) {
    for (const auto &user : users) {
        if (user.game_id == game_id && user.username == username) {
            return true;
        }
    }
    return false;
}

bool isValidUsername(const std::string &username) {
    // Check length
    if (username.size() != 5) {
        return false;
    }

    // Check if all characters are alphanumeric
    for (char c : username) {
        if (!std::isalnum(c)) {
            return false;
        }
    }
    return true;
}

bool isValidGameID(const std::string &game_id) {
    // Check length (for this example, I'm assuming a valid game_id has a length of at least 1)
    if (game_id.empty()) {
        return false;
    }

    // Check if all characters are alphanumeric (you can adjust this as per your requirements)
    for (char c : game_id) {
        if (!std::isalnum(c)) {
            return false;
        }
    }
    return true;
}

bool compareUsers(const User &a, const User &b) {
    return a.score > b.score;
}

nlohmann::json getScoresByGameID(const std::string &game_id){
    // Filtra os usuários pelo game_id
    std::vector<User> filtered_users;
    std::copy_if(users.begin(), users.end(), std::back_inserter(filtered_users), [&game_id](const User& u) {
        return u.game_id == game_id;
    });

    // Ordena os usuários filtrados pelo score
    std::sort(filtered_users.begin(), filtered_users.end(), compareUsers);
    
    nlohmann::json scores = nlohmann::json::array();
    int limit = std::min(10, (int)filtered_users.size());
    for (int i = 0; i < limit; ++i) {
        scores.push_back({{"username", filtered_users[i].username}, {"score", filtered_users[i].score}});
    }

    return scores;
}

void on_message(server* s, websocketpp::connection_hdl hdl, server::message_ptr msg) {
    
    nlohmann::json response;

    std::string payload = msg->get_payload();

    if (payload.substr(0, 9) == "/register") { 
        response["payload"] = "register";

        if (payload.length() < 16) {
            response["status"] = "error";
            response["message"] = "Malformed request!";
            s->send(hdl, response.dump(), msg->get_opcode());
            return;
        }

        std::string username = payload.substr(10, 5); // Assuming the username follows after "/register "

        if (!isValidUsername(username)) {
            response["status"] = "error";
            response["message"] = "Invalid username! Username should be 5 alphanumeric characters.";
            s->send(hdl, response.dump(), msg->get_opcode());
            return;
        }

        // Check if username is unique within the game_id (assuming a game_id is also part of the payload)
        std::string game_id = payload.substr(16); // Adjust accordingly based on your payload format

        if (!isValidGameID(game_id)) {
            response["status"] = "error";
            response["message"] = "Invalid game_id! Game ID should be alphanumeric.";
            s->send(hdl, response.dump(), msg->get_opcode());
            return;
        }

        if (usernameExists(game_id, username)) {
            response["status"] = "error";
            response["message"] = "Username already exists for this game!";
            s->send(hdl, response.dump(), msg->get_opcode());
            return;
        }

        User user;
        user.id = generateUUIDv4();
        user.game_id = game_id; // Assign the game_id
        user.hdl = hdl; // Salva o connection_hdl no struct User
        user.username = username;  // In a real scenario, extract from payload
        user.score = 0;
        user.level = 0;
        user.created_at = time(0);
        users.push_back(user);
        
        response["status"] = "success";
        response["user_id"] = user.id;

        s->send(hdl, response.dump(), msg->get_opcode());

    } else if (payload.substr(0, 11) == "/get-scores") {
        response["payload"] = "get-scores";

        std::string game_id = payload.substr(12);  // Isso pressupõe que depois de "/get-scores" exista um espaço seguido pelo game_id.
        
        response["data"] = getScoresByGameID(game_id);

        s->send(hdl, response.dump(), msg->get_opcode());

    } else if (payload.substr(0, 10) == "/set-score") {
        response["payload"] = "set-score";

        std::string id = payload.substr(11); 
        std::string game_id;

        for (User &user : users) {
            if (user.id == id) {
                game_id = user.game_id;
                user.score++;
                break;
            }
        }

        response["data"] = getScoresByGameID(game_id);

        std::vector<User> conn_users;
        std::copy_if(users.begin(), users.end(), std::back_inserter(conn_users), [&game_id](const User& u) {
            return u.game_id == game_id;
        });
        
        for (auto& user : conn_users) {
            s->send(user.hdl, response.dump(), msg->get_opcode());
        }
    }

}

int main() {
    server print_server;
    print_server.init_asio();
    
    print_server.set_message_handler(bind(&on_message, &print_server, std::placeholders::_1, std::placeholders::_2));
    
    print_server.set_open_handler([&](websocketpp::connection_hdl hdl) {
        connection_list.insert(hdl);
        std::cout << "Client connected with connection_hdl: " << hdl.lock().get() << std::endl;
    });

    print_server.set_close_handler([&](websocketpp::connection_hdl hdl) {
        connection_list.erase(hdl);
        std::cout << "Client disconnected with connection_hdl: " << hdl.lock().get() << std::endl;
    });
    
    print_server.listen(PORT);
    print_server.start_accept();
    print_server.run();
}
