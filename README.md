
# WebSocket Game Server

[Português](README.pt.md) | English

This project is a basic WebSocket server that allows user registration, maintenance of their scores, and querying scores for a specific game.

## Dependencies

- **WebSocket++**: Library to handle WebSocket connections.
- **nlohmann/json**: Library to handle JSON.
- **uuid**: Library for generating UUIDs.

## Data Structures

- `User`: Represents a user with an ID, game ID, WebSocket handle, username, score, level, and creation timestamp.
- `users`: Stores all the registered users on the server.
- `connection_list`: Stores all active WebSocket connections.

## Helper Functions

- `generateUUID()`: Provides unique ID generation.
- `usernameExists`: Checks if a username already exists for a specific game.
- `compareUsers`: Provides a comparison function to sort users by their score in descending order.
- `getScoresByGameID`: Returns top scores for a given game ID as a JSON array.

## WebSocket Handlers

- `on_message`: Handles received WebSocket messages. The server expects certain commands/payloads from the client:
  - `/register <username> <game_id>`: Registers a user with the provided username and game_id. If successful, the server sends back the user's unique ID.
  - `/get-scores <game_id>`: Returns the top scores for the provided game ID.
  - `/set-score <user_id>`: Increases the score of the user with the provided user_id by one and sends the updated scores to all connected users of the same game.

## Usage Instructions

1. Compile the code using an appropriate C++ compiler with the mentioned libraries.
2. Run the server. It will start listening on port 9900 for WebSocket connections.
3. Connect to the server using a WebSocket client and send commands as mentioned in the handlers section.

## Notes

1. The code assumes that the received payload for registering a user will be in the format `/register <username> <game_id>`.
2. The user's score is incremented with every `/set-score` request. There's no error handling in case an invalid user_id is provided.
3. There's no way to set a user's level, even though there's a `level` field in the `User` struct.
4. Broadcasting score updates to all users of a game every time a single user's score changes might not scale well with a large number of users.
5. Security and validation are minimal. In a production scenario, you'd want more rigorous checks and error handling. You would also consider encrypting WebSocket traffic using TLS (WSS).
6. Storing users in a vector (`users`) works for this small example, but for a real-world application, consider using a database.

## Nexts Movements

1. Replace third-party libraries, such as Websocket, with native socket functionalities for more streamlined integration.
2. Enhance security measures by implementing stricter CORS policies and robust client-side validations.
3. Develop efficient memory management techniques to guard against memory overflows and potential memory injections.

## Contributions

Feel free to contribute to this project by adding more features, fixing bugs, or improving the documentation.
