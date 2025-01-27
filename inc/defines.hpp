// Limits
#define MAX_CLIENTS 10
#define MAX_CLIENT_NICKNAME_LENGTH 9
#define MAX_CLIENT_USERNAME_LENGTH 32
#define MAX_CLIENT_REALNAME_LENGTH 100
#define MAX_CLIENT_CHANNELS	10

#define SERVER_NAME_MAX_LENGTH 63

#define	TIMEOUT		5000
#define	BUFFER_SIZE 1024

#define USER_MODES "aiwroOs"
#define CHANNEL_MODES "itkol"

#define RED_TEXT(msg) std::cout << "\033[31m" << msg << "\033[0m" << std::endl;

#define PORT 8080 // Port number to bind

// FOR NOW HERE
#define CLIENT_TIMEOUT_SECONDS 600 // Seconds before a client should be sent a PING message to check if it's still alive
#define CLIENT_PING_TIMEOUT_SECONDS 30 // Seconds before a connection is considered dead after no PONG reply is received
#define CLIENT_REGISTRATION_TIMEOUT_SECONDS 10 // Seconds before a client is considered not registered and disconnected
