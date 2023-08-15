#include <functional>
#include <iostream>
#include <string>
#include <vector>
#include <set>
#include <algorithm>
#include <ctime>

// Third-Party Libraries
#include <uuid/uuid.h>
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

std::string generateUUID() {
    uuid_t uuid;
    uuid_generate_random(uuid);
    char s[37];
    uuid_unparse(uuid, s);
    return std::string(s);
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

        if (payload.length() < 16) {
            response["status"] = "error";
            response["message"] = "Malformed request!";
            s->send(hdl, response.dump(), msg->get_opcode());
            return;
        }

        std::string username = payload.substr(10, 5); // Assuming the username follows after "/register "

        if (username.length() != 5) {
            response["status"] = "error";
            response["message"] = "Username should have exactly 5 characters!";
            s->send(hdl, response.dump(), msg->get_opcode());
            return;
        }

        // Check if username is unique within the game_id (assuming a game_id is also part of the payload)
        std::string game_id = payload.substr(16); // Adjust accordingly based on your payload format
        if (usernameExists(game_id, username)) {
            response["status"] = "error";
            response["message"] = "Username already exists for this game!";
            s->send(hdl, response.dump(), msg->get_opcode());
            return;
        }

        User user;
        user.id = generateUUID();
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

        std::string game_id = payload.substr(12);  // Isso pressupõe que depois de "/get-scores" exista um espaço seguido pelo game_id.
        
        s->send(hdl, getScoresByGameID(game_id).dump(), msg->get_opcode());

    } else if (payload.substr(0, 10) == "/set-score") {
        std::string id = payload.substr(11); 
        std::string game_id;

        for (User &user : users) {
            if (user.id == id) {
                game_id = user.game_id;
                user.score++;
                break;
            }
        }

        std::vector<User> conn_users;
        std::copy_if(users.begin(), users.end(), std::back_inserter(conn_users), [&game_id](const User& u) {
            return u.game_id == game_id;
        });
        
        for (auto& user : conn_users) {
            s->send(user.hdl, getScoresByGameID(game_id).dump(), msg->get_opcode());
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
