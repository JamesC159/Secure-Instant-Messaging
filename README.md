# Secure-Instant-Messaging
This project is an implementation of an internet instant messaging system that satisfies certain security requirements, namely authentication, confidentiality, and integrity.

Group Members: James Combs, Joel Seida

Platform: Linux (Debian)
Required Libraries: Crypto++
To Build: Run make in root directory of project
To Run: In three seperate terminals:
	Run ./server
	Run ./client
	Run ./client

	For the two clients enter a valid username and password
		and then request to connect to the other from
		one of the clients

	After the session connects, both clients can talk

	To exit, one of the clients must type EXIT in their chat
		window

	
	Note: 
	You may have to run `make clean ; make` if it doesn't build correctly. You MUST start the server first,
	Then run both clients. You must make sure that you log in with both clients before you try to talk to one
	or the other. You may need to make sure you `sudo chmod -R 777` the directory to run or make any changes. When
	you are talking across the socket with another client, backspace does not work. When the program exits, you may
	see a socket error. This error is related to a bad file descriptor which we have no idea why. This has something
	to do with the CryptoPP API.

	
	List of username and passwords to log in.

	john123		stat8&imapale
	james99		Hood744}mede
	joel100		QMs82?hunter
	billy77777	hoax63)Fuqua
	hankhill91	verso3*bowie
	abcde112	dew32*checks
