#include <arpa/inet.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>

/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400

// Header Sizes
#define HEADER_NAME_MAX_SIZE 512
#define HEADER_VALUE_MAX_SIZE 1024
#define MAX_HEADERS 32
#define HTTP_REQUEST_MAX_SIZE 8192

typedef struct {
	char name[HEADER_NAME_MAX_SIZE];
	char value[HEADER_VALUE_MAX_SIZE];
} http_header;


/* You won't lose style points for including this long line in your code */
static const char *user_agent_hdr = "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/10.0.3\r\n";


/* Input: request, a string
 * Output: 1 if request is a complete HTTP request, 0 otherwise
 * */
int is_complete_request(const char *request) {

	bool isValid = false;

	if (memcmp(request, "GET", strlen("GET")) == 0) {
        isValid = true;
    } else if (memcmp(request, "POST", strlen("POST")) == 0) {
        isValid = true;
    }

	if((memcmp(request + (strlen(request) - strlen("\r\n\r\n")), "\r\n\r\n", strlen("\r\n\r\n")) == 0) && isValid) {
		return 1;
	}
	else {
		return 0;
	}
}

/* Parse an HTTP request, and copy each parsed value into the
 * corresponding array as a NULL-terminated string.
 * Input: request - string containing the original request;
 *        should not be modifed.
 * Input: method, hostname, port, uri - arrays to which the
 *        corresponding parts parsed from the request should be
 *        copied, as strings.  The uri is the "file" part of the requested URL.
 *        If no port is specified, the port array should be populated with a
 *        string specifying the default HTTP port.
 * Input: headers - an array of http_headers, each of which should be
 *        populated with the corresponding name/value of a header.
 * Output: return the number of headers in the request.
 * */
int parse_request(const char *request, char *method,
		char *hostname, char *port, char *uri, http_header *headers) {

	char request_copy[strlen(request)];

	strcpy(request_copy, request);

	char EOLdelimiter[] = "\r\n";
	char spaceDelimiter[] = " ";
	char backslashDelimiter[] = "/";
	char doubleBackslashDelimiter[] = "//";
	char portDelimiter[] = ":";
	

	char *request_line = strtok(request_copy, EOLdelimiter);  // get the first line of the request

	char *req_ptr;
	strcpy(method,strtok_r(request_line, spaceDelimiter ,&req_ptr));  // Obtains the GET from the 1st request line
	char url[128];
	strcpy(url,strtok_r(NULL, spaceDelimiter, &req_ptr));  // obtains the URL - req_ptr helps the tokenizer continue from the last call

	char url_cpy[128];

	bool hasPort = false;
	int num_cols = 0;

	int i;
	for(i = 0; i < (strlen(url) - 1); ++i) {
		if(url[i] == ':') {
			++num_cols;
		}
	}
	char *url_ptr;
	strtok_r(url, doubleBackslashDelimiter, &url_ptr);  // move the ptr past the “http://”

	if (num_cols > 1) {
		hasPort = true;
	}

	// If there is a port
	if(hasPort) {
		url_ptr++;
		strcpy(hostname,strtok_r(NULL, portDelimiter ,&url_ptr));
		strcpy(port, strtok_r(NULL, backslashDelimiter, &url_ptr));
	}
	// Else add the default http port
	else {
		strcpy(hostname,strtok_r(NULL, backslashDelimiter ,&url_ptr));
		strcpy(port, "80");
	}



    // Make sure there is a uri
    memcpy(uri, "/", 1);
    if(url_ptr != NULL) {
	    strcpy(uri + 1,strtok_r(NULL, EOLdelimiter ,&url_ptr));
    }

	char *req_line_ptr;
	int numHeaders = 0;

	while((request_line = strtok(NULL, EOLdelimiter))) { // get subsequent lines until it returns NULL

		strcpy(headers[numHeaders].name, strtok_r(request_line, portDelimiter, &req_line_ptr));
		req_line_ptr++;
		strcpy(headers[numHeaders].value, strtok_r(NULL, EOLdelimiter, &req_line_ptr));

		numHeaders++;
	} 
	
	return numHeaders;
}

/* Iterate through an array of headers, and return the value
 * (as a char *) corresponding to the name passed.  If there is no
 * header with the name passed, return NULL.
 * Input: name - the name of the header whose value is being sought.
 * Input: headers - the array of http_headers to be searched.
 * Input: num_headers - the number of headers in the headers array.
 * */
char *get_header_value(const char *name, http_header *headers, int num_headers) {
	int i;
	for(i = 0; i < num_headers; ++i) {
		if(strcmp(name, headers[i].name) == 0) {
			return headers[i].value;
		}
	}
	
	return NULL;
}

int listenForClient(int argc, char *argv[], struct sockaddr_storage* peer_addr, socklen_t* peer_addr_len, bool* firstIteration) {

    // Socket Variables
    struct addrinfo hints;
	struct addrinfo *result;
	int portindex;
	int sfd;
	int s;
    int af;

     if (!(argc == 2 || (argc == 3 &&
			(strcmp(argv[1], "-4") == 0 || strcmp(argv[1], "-6") == 0)))) {
		fprintf(stderr, "Usage: %s [ -4 | -6 ] port\n", argv[0]);
		exit(EXIT_FAILURE);
	}
	if (argc == 2) {
		portindex = 1;
	} else {
		portindex = 2;
	}
	if (argc == 2 || strcmp(argv[1], "-4") == 0) {
		af = AF_INET;
	} else {
		af = AF_INET6;
	}

	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = af;           /* Choose IPv4 or IPv6 */
	hints.ai_socktype = SOCK_STREAM; /* Datagram socket */
	hints.ai_flags = AI_PASSIVE;    /* Wildcard IP address - i.e., listen
					   on _all_ IPv4 or IPv6 addresses */
	hints.ai_protocol = 0;          /* Any protocol */
	hints.ai_canonname = NULL;
	hints.ai_addr = NULL;
	hints.ai_next = NULL;

	if ((s = getaddrinfo(NULL, argv[portindex], &hints, &result)) < 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
		exit(EXIT_FAILURE);
	}

    if ((sfd = socket(result->ai_family, result->ai_socktype,
			result->ai_protocol)) < 0) {
		perror("Error creating socket");
		exit(EXIT_FAILURE);
	}

	if (bind(sfd, result->ai_addr, result->ai_addrlen) < 0) {
		perror("Could not bind");
		exit(EXIT_FAILURE);
	}

	freeaddrinfo(result);           /* No longer needed */


	printf("Waiting for data on port %s...\n", argv[portindex]);
	/* Read datagrams and echo them back to sender */

	if(listen(sfd, 100) < 0) {
		perror("Could not listen");
		exit(EXIT_FAILURE);	
	}

    return sfd;
 }

ssize_t readAndParse_client(int sfd, char* request_buff, struct sockaddr_storage* peer_addr, socklen_t* peer_addr_len, char* method, char* hostname, char* port, char* uri, http_header *headers, int* numHeaders) {
    ssize_t nread = 0;
    ssize_t totalRead = 0;
    *numHeaders = 0;

    int accept_fd;
	accept_fd = accept(sfd, (struct sockaddr *) peer_addr, peer_addr_len);
	if (accept_fd < 0) {
		perror("Could not accept");
	}

    // getnameinfo((struct sockaddr *) &peer_addr, peer_addr_len, client_hostname, MAXLINE, 
	// 			client_port, MAXLINE, 0);

    for (;;) {
        // Get Request from client
        if((nread = recv(accept_fd, &request_buff[totalRead], HTTP_REQUEST_MAX_SIZE, 0)) < 0) {
            perror("recve error: ");
            break;
        };
        // printf("Read %zd bytes\n", nread);
        totalRead += nread;
        fflush(stdout);
        if(strstr(request_buff, "\r\n\r\n")) {
        printf("request_buff from client: \n%s", request_buff);
            break;
        }
    }
    request_buff[totalRead] = '\0';

    if (is_complete_request(request_buff)) {
        *numHeaders = parse_request(request_buff, method, hostname, port, uri, headers);

        // printf("request method: %s\n", method);
        // printf("request hostname: %s\n", hostname);
        // printf("request port: %s\n", port);
        // printf("request host: %s\n", get_header_value("Host", headers, numHeaders));
        printf("request uri: %s\n", uri);
        // printf("\n");
    }
    else {
		printf("request is incomplete\n");
		printf("\n");
	}
    return accept_fd;
}

int connectAndSendRequest_server(char* hostname, char* port, char* method, char* uri, http_header* headers, int numHeaders) {

    #define BUF_SIZE 500

    // Vars for communicating with Server
	struct addrinfo hints;
	struct addrinfo *result, *rp;
	int sfd, s, j;
	size_t len;
	ssize_t nread;
	char buf[BUF_SIZE];

	/* Obtain address(es) matching host/port */

	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
	hints.ai_socktype = SOCK_STREAM; /* Datagram socket */
	hints.ai_flags = 0;
	hints.ai_protocol = 0;          /* Any protocol */

	s = getaddrinfo(hostname, port, &hints, &result);
	if (s != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
		exit(EXIT_FAILURE);
	}

	/* getaddrinfo() returns a list of address structures.
	   Try each address until we successfully connect(2).
	   If socket(2) (or connect(2)) fails, we (close the socket
	   and) try the next address. */

	for (rp = result; rp != NULL; rp = rp->ai_next) {
		sfd = socket(rp->ai_family, rp->ai_socktype,
				rp->ai_protocol);
		if (sfd == -1)
			continue;

		if (connect(sfd, rp->ai_addr, rp->ai_addrlen) != -1)
			break;                  /* Success */

		close(sfd);
	}

	if (rp == NULL) {               /* No address succeeded */
		fprintf(stderr, "Could not connect\n");
		exit(EXIT_FAILURE);
	}

	freeaddrinfo(result);           /* No longer needed */

	// Send Request to server
    char request_buff[HTTP_REQUEST_MAX_SIZE];
	sprintf(request_buff, "%s %s HTTP/1.0\r\n", method, uri);
	sprintf(request_buff, "%sHost: %s\r\n", request_buff, hostname);
	sprintf(request_buff, "%sUser-Agent: %s", request_buff, user_agent_hdr);
	sprintf(request_buff, "%sConnection: close\r\n", request_buff);
	sprintf(request_buff, "%sProxy-Connection: close\r\n", request_buff);

    int header_index;
    for(header_index = 0; header_index < numHeaders; ++header_index) {
        if((strcmp(headers[header_index].name, "Host") == 0) ||
        ((strcmp(headers[header_index].name, "User-Agent") == 0)) ||
        ((strcmp(headers[header_index].name, "Connection") == 0)) ||
        ((strcmp(headers[header_index].name, "Proxy-Connection") == 0))) {
            //skip these
        } else {
            sprintf(request_buff, "%s%s: %s\r\n", request_buff, headers[header_index].name, headers[header_index].value);
        }
    }
    size_t headerLength = sprintf(request_buff, "%s\r\n", request_buff);
    // printf("request_buff to server:\n%s", request_buff);

	size_t wbytes = 0;
	size_t total_wbytes = 0;
	while(total_wbytes < headerLength) {
		wbytes = write(sfd, request_buff, headerLength);
		total_wbytes += wbytes;
	}
	// printf("Sent %d bytes to the server\n", (int)total_wbytes);

    return sfd;
}

void readAndSendToClient(int sfd, int client_fd) {
    char read_buffer[MAX_OBJECT_SIZE];
	int totalRead = 0;
    // Read Response from server
	for(;;) {
		int bytesRead =  read(sfd, &read_buffer[totalRead], sizeof(read_buffer));
		if(bytesRead == 0) {
			break;
		}
       		totalRead += bytesRead;
		// printf("Recieved %d bytes from the server\n", bytesRead);	
	}

    // Send Response to Client
	send(client_fd, read_buffer, totalRead, 0);

}

int main(int argc, char *argv[])
{
    // Socket Variables
	struct sockaddr_storage peer_addr;
    int s;
	socklen_t peer_addr_len;
	char request_buff[HTTP_REQUEST_MAX_SIZE];
	char host[NI_MAXHOST], service[NI_MAXSERV];
    bool firstIteration = true;

    // Http Parser Variables
    char method[8];
	char hostname[60];
	char port[6];
	char uri[60];
    int numHeaders;
	http_header headers[30];

        //Listen for incoming connections
        int sfd = listenForClient(argc, argv, &peer_addr, &peer_addr_len, &firstIteration);


    for (;;) {
        //Accept connection and Read and parse request from client
        int client_fd = readAndParse_client(sfd, request_buff, &peer_addr, &peer_addr_len, method, hostname, port, uri, headers, &numHeaders);

        //Connect to web server and request
        int server_fd = connectAndSendRequest_server(hostname, port, method, uri, headers, numHeaders);

        //Read response and send to client
        readAndSendToClient(server_fd, client_fd);

        close(client_fd);
    }
	
    



	

    //     sleep(2);

    //     // send response to client
	// 	if (nread == -1)
	// 		continue;               /* Ignore failed request_buff */

	// 	s = getnameinfo((struct sockaddr *) &peer_addr,
	// 			peer_addr_len, host, NI_MAXHOST,
	// 			service, NI_MAXSERV, NI_NUMERICSERV);

	// 	if (s == 0)
	// 	       printf("Received %zd bytes from %s:%s\n",
	// 			       nread, host, service);
	// 	else
	// 	       fprintf(stderr, "getnameinfo: %s\n", gai_strerror(s));

	// 	if (sendto(accept_fd, request_buff, nread, 0,
	// 				(struct sockaddr *) &peer_addr,
	// 				peer_addr_len) != nread)
	// 		perror("Error sending response");
	// }


    // printf("%s", user_agent_hdr);
    // return 0;
}
