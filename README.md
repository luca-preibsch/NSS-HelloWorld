# Getting Started

## Creating the Certificates:
Since this is a NSS "Hello World" program, we will use the tools provided by NSS.

#### 1. Create a new NSS database
```shell
mkdir pki           # the new db will be located in the directory pki
certutil -N -d pki  # create a new db in the directory pki
```

```shell
Enter new password:

> <password for the NSS db, can be ommitted>
```


#### 2. Create a self-signed Root CA certificate
```shell
certutil -S -d pki -n RootCA -s "CN=RootCA" -t "CT,," -x -v 120 -2
```

- `n <nickname>`: Specify the nickname, the certificate will later be referred to as
- `-x`: Create a self-signed certificate
- `-v <valid month>`: Set the number of month the certificate will be valid, in our case 1 year.
- `-2`: Add a basic constraint extension to a certificate that is being created or added to a database.
This will cause the prompt to ask if this is meant to be a CA certificate.

```
> certutil -S -d pki -n RootCA -s "CN=RootCA" -t "CT,," -x -v 120 -2

Enter Password or Pin for "NSS Certificate DB":

> <password set earlier>

Is this a CA certificate [y/N]?

> y

Enter the path length constraint, enter to skip [<0 for unlimited path]:

> -1    # unlimited path length

Is this a critical extension [y/N]?

> N
```

#### 3. Extract the CA certificate from the database for later use

```shell
certutil -L -d pki -n RootCA -a -o rootca.crt
```

- `n <nickname>`: Nickname of the certificate to be extracted
- `-a`: ASCII format
- `-o <output path>`: specify the output file name

#### 4. Create the server certificate

```shell
certutil -S -d pki -c "RootCA" -n "server" -s "CN=helloworld.example.com" -t "u,u,u" -m 1001 -1 -6
```

- `n <nickname>`: Specify the nickname, the certificate will later be referred to as
- `-c <issuer name>`: Set the issuer nickname, which will be the certificate in the database signing the new server certificate.
- `-s <subject>`: The subject should contain a common name with the server's url.
- `-1`: Create a key usage extension. This will cause a prompt that asks which values should be set.
- `-6`: Create an extended key usage extension. This will cause a prompt that asks which values should be set.

```shell
Enter Password or Pin for "NSS Certificate DB":

> password set earlier

		0 - Digital Signature
		1 - Non-repudiation
		2 - Key encipherment
		3 - Data encipherment
		4 - Key agreement
		5 - Cert signing key
		6 - CRL signing key
		Other to finish
 > 0  # Digital Signature
 
		0 - Digital Signature
		1 - Non-repudiation
		2 - Key encipherment
		3 - Data encipherment
		4 - Key agreement
		5 - Cert signing key
		6 - CRL signing key
		Other to finish
 > 2  # Key encipherment
 
		0 - Digital Signature
		1 - Non-repudiation
		2 - Key encipherment
		3 - Data encipherment
		4 - Key agreement
		5 - Cert signing key
		6 - CRL signing key
		Other to finish
 > 9  # Other to finish
 
Is this a critical extension [y/N]?
> y
		0 - Server Auth
		1 - Client Auth
		2 - Code Signing
		3 - Email Protection
		4 - Timestamp
		5 - OCSP Responder
		6 - Step-up
		7 - Microsoft Trust List Signing
		Other to finish
 > 0  # Server Auth
 
		0 - Server Auth
		1 - Client Auth
		2 - Code Signing
		3 - Email Protection
		4 - Timestamp
		5 - OCSP Responder
		6 - Step-up
		7 - Microsoft Trust List Signing
		Other to finish
 > 1  # Client Auth
 
		0 - Server Auth
		1 - Client Auth
		2 - Code Signing
		3 - Email Protection
		4 - Timestamp
		5 - OCSP Responder
		6 - Step-up
		7 - Microsoft Trust List Signing
		Other to finish
 > 9  # Other to finish
 
Is this a critical extension [y/N]?
> n
```

#### Further useful tools:

- List all certificates in the database:

```shell
certutil -L -d pki
```

- Print further information about a specific certificate
```shell
certutil -L -d pki -n <nickname>
```

- Remove a certificate from the database:

```shell
certutil -F -d pki -n <nickname>
```
