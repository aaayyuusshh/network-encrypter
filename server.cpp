/* SERVER SIDE CODE */

//Socket programming imports & more!
#include<stdio.h>
#include<string.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<unistd.h>	

#include <stdlib.h>
#include <sys/types.h>
#include <netinet/in.h>


//reminder: non vowels: TCP, vowels: UDP

/* GLOBAL VARIABLES */

//constants
int ENCODING_TYPE_FlAG;

int listeningSocket;
int serverSocket;
int udpSocket;
struct sockaddr_in udpServerAddr, udpClientAddr;

//simple encryption: devoweling
//paramerts: clientMessage[] = letter to devowel that the client sent
void simpleEncrypt(char clientMessage[]){ /* SIMPLE ENCRYPTION ALGORITHM */
    printf("-- SIMPLE ENCRYPTION --\n");
    int length= strlen(clientMessage);

    char vowels[length];                    
    char nonVowels[length];

    for(int i=0; i < length; i++){
        
        if(clientMessage[i] == 'a' || clientMessage[i] == 'e' || clientMessage[i] == 'i' || clientMessage[i] == 'o' || clientMessage[i] == 'u'
            || clientMessage[i] == 'A' || clientMessage[i] == 'E' || clientMessage[i] == 'I' || clientMessage[i] == 'O' || clientMessage[i] == 'U'){

            vowels[i] = clientMessage[i];
            nonVowels[i]= ' ';
        }

        else{
            nonVowels[i] = clientMessage[i];
            vowels[i] = ' ';
        }
    }

    vowels[length] = '\0';
    nonVowels[length]= '\0';

    printf("Non Vowels:\'%s\'\n", nonVowels);
    printf("Vowels:    \'%s\'\n", vowels);

    //recieving dummy message through UDP for client address purposes
    char buffer[1000];
    bzero(buffer, 1000);
    int len= sizeof(udpClientAddr);
    recvfrom(udpSocket, buffer, 1000, 0, (struct sockaddr *)&udpClientAddr, (socklen_t*)&len);

    //send non-vowels to client through TCP
    send(serverSocket, nonVowels, strlen(nonVowels), 0);

    usleep(20);
    
    //send vowels to client through UDP
    //send(serverSocket, vowels, strlen(vowels), 0); //TCP way
    sendto(udpSocket, vowels, strlen(vowels), 0, (const struct sockaddr *)&udpClientAddr, len);

}

//simple decryption: envoweling
void simpleDecrypt(char nonVowels[], char vowels[]){

    int length= strlen(nonVowels);

    char decrypted[length];

    for(int j=0; j< length; j++){
        if(vowels[j] == ' '){
            decrypted[j] = nonVowels[j];
        }

        else if(vowels[j] != ' '){
            decrypted[j] = vowels[j];
        }
    }
    decrypted[length] ='\0';
    printf("Decrypted: \'%s\'\n", decrypted);
    printf("Length: %lu\n",strlen(decrypted));

    //send decrypted message to server
     send(serverSocket, decrypted, strlen(decrypted), 0);

}

//set up tcp socket
void setupTCP(){

    //address initialization
    struct sockaddr_in address;
    memset(&address,0, sizeof(address));
    address.sin_family= AF_INET;
    address.sin_port= htons(8000);
    address.sin_addr.s_addr= INADDR_ANY;

     //listening socket creation
   
    listeningSocket= socket(AF_INET, SOCK_STREAM, 0);
    if(listeningSocket == -1){
        perror("Listening socket creation failed!");
    }
    printf("Listening socket created.\n");

    //binding address w/ listening socket
    int bindStatus;
    bindStatus= bind(listeningSocket, (struct sockaddr *)&address, sizeof(address));
    if(bindStatus == -1){
        perror("Binding failed!");
    }
    printf("Binding successful.\n");

    //server listening for clients
    int listenStatus;
    listenStatus = listen(listeningSocket, 5);
    if(listenStatus == -1){
        perror("Listening failed!");
    }

    printf("Waiting(Listening) for clients....\n");

    //accept an incoming client connection
    serverSocket = accept(listeningSocket, NULL, NULL);
    if(serverSocket == -1){
        perror("accept() call failed!");
    }
    printf("--- CONNECTION ACCEPTED ---\n");
    
}

//set up udp socket (NOTE: UDP Is connectionless!)
void setupUDP(){

    //creating udp socket file descriptor
    if((udpSocket = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
        perror("UPD socket creation failed!\n");
        exit(EXIT_FAILURE);
    }
    printf("UDP socket created.\n");

    //memset to 0s
    memset(&udpServerAddr, 0, sizeof(udpServerAddr));
    memset(&udpClientAddr, 0, sizeof(udpClientAddr));

    //address initialization for the UDP server
    udpServerAddr.sin_family= AF_INET;  
    udpServerAddr.sin_addr.s_addr = INADDR_ANY;
    udpServerAddr.sin_port= htons(8000);

    //bind the UDP socket with the UDP server address
    if(bind(udpSocket, (const struct sockaddr *)&udpServerAddr, sizeof(udpServerAddr)) < 0){
        printf("UDP binding failed!");
        exit(EXIT_FAILURE);

    }
}

int main(){

    printf("Welcome to our vowelizer server (: \n");

    setupTCP();
    setupUDP();

    /* SEND // RECIEVE */

    int recieveStatus;
    char recieveMsg[1000]="";
    //recieving clients option: 1, 2 or 3
    while((recieveStatus =  recv(serverSocket, recieveMsg, 1000, 0)) > 0){
        printf("Clients option: %s\n", recieveMsg);

        int option = recieveMsg[0] - '0';

        //1= encrypt (devowel)
        if(option == 1){

            char toDevowel[1000]="";
            recv(serverSocket, toDevowel, 1000, 0);
            printf("Client's message: \'%s\'\n", toDevowel);

            // send(serverSocket, "vowels", strlen("vowels"), 0);
            // send(serverSocket, "non-vowels", strlen("non-vowels"), 0);

            simpleEncrypt(toDevowel);
        }

        //2= decrypt (envowel)
        else if(option ==2){

            char nonVowels[1000]="";
            char vowels[1000]="";
          
            //recieve nonvowels from client
            recv(serverSocket, nonVowels, 1000, 0);
            printf("Non Vowels from client: \'%s\'\n", nonVowels);

            usleep(20);

            //recieve vowels from client (through UPD)
            recv(serverSocket, vowels, 1000, 0);
            printf("Vowels from client: \'%s\'\n", vowels);

            simpleDecrypt(nonVowels, vowels);
        }

        //3= quit

    }

    //closing sockets when done
    close(serverSocket);
    close(listeningSocket);

    return 0;
}