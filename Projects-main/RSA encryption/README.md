# Assignment 5: Public Key Cryptography

## Description: 
	Keygen: Makes public and private Key
	Encrypt: Takes public key and encrypts the message using the public key and creates the ciphertext
	Decrypt: Decrypts with the private key

## Build:
	make
	make all
	make keygen
	make encrypt
	make decrypt

## Options:
	• -b:specifies the minimum bits needed for the public modulus n.
	• -i:specifies the number of Miller-Rabin iterations for testing primes (default:50).
	• -n pbfile: specifies the publickey file (default:rsa.pub).
	• -d pvfile: specifies the private key file (default:rsa.priv).
	• -s : specifies the random seed for the random state initialization (default: the seconds since the UNIX epoch, given by time(NULL)).
	• -v:enables verbose output.
	• -h:displays program synops is and usage.

## Run:
	./keygen
	./encrypt -i[input_file] -o[output_file]
	./decrypt -i[input_file] -o[output_file]


## Files:
	1. decrypt.c: This contains the implementation and main() function for the decrypt program. 
	2. encrypt.c: This contains the implementation and main() function for the encrypt program. 
	3. keygen.c: This contains the implementation and main() function for the keygen program. 
	4. numtheory.c: This contains the implementations of the number theory functions. 
	5. numtheory.h: This specifies the interface for the number theory functions. 
	6. randstate.c: This contains the implementation of the random state interface for the RSA library and number theory functions. 
	7. randstate.h: This specifies the interface for initializing and clearing the random state. 
	8. rsa.c: This contains the implementation of the RSA library. 
	9. rsa.h: This specifies the interface for the RSA library.
