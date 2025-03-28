#include <iostream>
#include <list>

#include <pthread.h>
#include <unistd.h>
#include <string.h>

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>

#include "Config.hpp"

#define BACKLOG 10

struct Client {
    int sockfd;
    pthread_t recv_thread;
    char name[USERNAME_LEN], ip[INET6_ADDRSTRLEN];

    friend std::ostream& operator<<(std::ostream& outstrm, const Client& client) {
        return outstrm << "Name: " << client.name << " | IP: " << client.ip << " | Socket: " << client.sockfd;
    };
};

pthread_rwlock_t clist_mtx = PTHREAD_RWLOCK_INITIALIZER;
std::list<Client> clients;
using ClientItr = std::list<Client>::iterator;

// Displays all clients | You should have exclusive access to `clients` before calling
void query_clients_no_lock(std::ostream& outstrm = std::cout) {
    if (clients.size() < 1) {
        outstrm << "There are no clients connected" << std::endl;
        return;
    };

    outstrm << "CONNECTED CLIENTS" << std::endl;

    for (const auto& client : clients) {
        outstrm << "\t" << client << std::endl;
    };
};


// Displays all clients | Gets a write-lock for `clients`
inline void query_clients(std::ostream& outstrm = std::cout) {
    pthread_rwlock_wrlock(&clist_mtx);
    query_clients_no_lock(outstrm);
    pthread_rwlock_unlock(&clist_mtx);
};

// Relays a message to all other clients
void relay_no_lock(const ClientItr& sender, char* message, int msg_size) {
    // Relay to clients after the sender
    ClientItr current = sender; current++;
    while (current != clients.end()) {
        if (send(current->sockfd, message, msg_size, 0) == -1) {
            std::cerr << "relay() -> Failed to send message to client: sockfd=" << current->sockfd << ", IP=" << current->ip << std::endl;
        };

        current++;
    };

    // Relay to clients before the sender
    current = sender;
    while (current != clients.begin()) {
        current--;

        if (send(current->sockfd, message, msg_size, 0) == -1) {
            std::cerr << "relay() -> Failed to send message to client: sockfd=" << current->sockfd << ", IP=" << current->ip << std::endl;
        };
    };
};

// Relays a message to all other clients
inline void relay(const ClientItr& sender, char* message, int msg_size) {
    pthread_rwlock_rdlock(&clist_mtx); // Get read (shared) lock
    relay_no_lock(sender, message, msg_size);
    pthread_rwlock_unlock(&clist_mtx); // Release lock
};


void disconnect_client(ClientItr client, std::ostream& outstrm = std::cout) {
    std::string discon_name = client->name;
    pthread_t discon_thread = client->recv_thread;

    outstrm << "Disconnecting client: {" << *client << "}" << std::endl;

    // Get an exclusive lock for deleting the client
    pthread_rwlock_wrlock(&clist_mtx);
    close(client->sockfd);
    clients.erase(client);

    // Swap exclusive lock for a shared one so that we can send messages
    pthread_rwlock_unlock(&clist_mtx);
    pthread_rwlock_rdlock(&clist_mtx);

    std::string discon_message = "SERVER\0" + discon_name + " has disconnected";
    int msg_size = discon_message.length();

    // Relay disconnect message to remaining clients
    for (const auto& client : clients) {
        if (send(client.sockfd, discon_message.c_str(), msg_size, 0) == -1) {
            std::cerr << "disconnect_client() -> Failed to send message to client: sockfd=" << client.sockfd << ", IP=" << client.ip << std::endl;
        };
    };

    query_clients_no_lock(outstrm); // We already have a lock, so use the no_lock varient

    pthread_cancel(discon_thread); // Cancel the client recv thread

    pthread_rwlock_unlock(&clist_mtx); // Release lock
};


void* client_loop(void* args) {
    pthread_rwlock_rdlock(&clist_mtx);
    ClientItr client_itr = *((ClientItr*)args);
    Client client = *client_itr;
    pthread_rwlock_unlock(&clist_mtx);

	if (send(client.sockfd, "SERVER\0Enter your username (up to 6 characters)", 48, 0) == -1) {
        std::cerr << "client_loop() -> Failed to request username from client" << std::endl;
        disconnect_client(client_itr);
        return NULL; 
    };

    int bytes_recvd = recv(client.sockfd, client.name, 6, 0);
    if(bytes_recvd < 1) {
        strncpy(client.name, "ERROR\0", USERNAME_LEN);
    };
    client.name[bytes_recvd] = '\0';
    bytes_recvd = -1;

    char total_buffer[MESSAGE_LEN+USERNAME_LEN];
    char* msg_buffer = total_buffer + USERNAME_LEN;

    memset(total_buffer, MESSAGE_LEN, 0);
    strncpy(total_buffer, client.name, USERNAME_LEN);


    pthread_rwlock_rdlock(&clist_mtx);
    strncpy(client_itr->name, client.name, USERNAME_LEN);
    pthread_rwlock_unlock(&clist_mtx);

    do {
        bytes_recvd = recv(client.sockfd, msg_buffer, MESSAGE_LEN, 0);
        if (bytes_recvd < 1) break;

        msg_buffer[bytes_recvd] = '\0';

        std::cout << "(" << total_buffer << "): \"" << msg_buffer << "\"" << std::endl;

        relay(client_itr, total_buffer, bytes_recvd + USERNAME_LEN);

    } while(true);

    disconnect_client(client_itr);
    return nullptr;
};

void connect_client(const Client& client_data, std::ostream& outstrm = std::cout) {
    pthread_rwlock_wrlock(&clist_mtx);

    clients.push_back(client_data);
    ClientItr client = std::prev(clients.end());

    int tresult = pthread_create(&(client->recv_thread), NULL, client_loop, (void*)(&client));

    if (tresult != 0) {
        std::cerr << "connect_client() -> Failed to create thread for client" << std::endl;
        close(client->sockfd);
        clients.erase(client);
        return;
    };
    pthread_detach(client->recv_thread);

    pthread_rwlock_unlock(&clist_mtx);

    query_clients(outstrm);
};

void sigchld_handler(int s)
{
	(void)s; // quiet unused variable warning

	// waitpid() might overwrite errno, so we save and restore it:
	int saved_errno = errno;

	while(waitpid(-1, NULL, WNOHANG) > 0);

	errno = saved_errno;
}


// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(void)
{
	int sockfd, new_fd;  // listen on sock_fd, new connection on new_fd
	struct addrinfo hints, *servinfo, *p;
	struct sockaddr_storage their_addr; // connector's address information
	socklen_t sin_size;
	struct sigaction sa;
	int yes=1;
	char s[INET6_ADDRSTRLEN];
	int rv;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE; // use my IP

	if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and bind to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("server: socket");
			continue;
		}

		if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
				sizeof(int)) == -1) {
			perror("setsockopt");
			exit(1);
		}

		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("server: bind");
			continue;
		}

		break;
	}

	freeaddrinfo(servinfo); // all done with this structure

	if (p == NULL)  {
		fprintf(stderr, "server: failed to bind\n");
		exit(1);
	}

	if (listen(sockfd, BACKLOG) == -1) {
		perror("listen");
		exit(1);
	}

	sa.sa_handler = sigchld_handler; // reap all dead processes
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if (sigaction(SIGCHLD, &sa, NULL) == -1) {
		perror("sigaction");
		exit(1);
	}

	printf("server: waiting for connections...\n");

	while(1) {  // main accept() loop
		// Allocate memory for new client data
		Client new_client;
	
		sin_size = sizeof(their_addr);
		new_client.sockfd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
		if (new_client.sockfd == -1) {
			perror("accept");
			continue;
		}

		inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), new_client.ip, INET6_ADDRSTRLEN);
		printf("server: got connection from %s\n", new_client.ip);
		
		connect_client(new_client);
	}

	return 0;
}