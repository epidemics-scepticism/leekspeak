# leekspeak
convert an onion address into a 5 word phrase, and back

### How it works
Each .onion address is a base32 encoding of the first 10 bytes of a SHA1 hash of the public key of the hidden service.

By decoding the base32 we get 10 bytes of binary data, we break this up into 5 blocks of 2 bytes (uint16_t) and map those to a wordlist containing 65536 words.

This create a unique 5 word listing that can be memorized like a diceware/correct-horse-battery-staple password.

By taking back in the 5 word passphrase and mapping it against the same wordlist, we can reconstruct the original 10byte value, reencode and it reconstruct the .onion address.

### Building

    user@subgraph:~/src/leekspeak$ make
    gcc -Wall -Wextra -pedantic -std=gnu99 -O2 -s -o leekspeak leekspeak.c onion.c

### Usage

    user@subgraph:~/src/leekspeak$ ./leekspeak encode 2b5dj4wasoaww3k6.onion
    chokidars defrosts glads reprisals smirky 
    user@subgraph:~/src/leekspeak$ ./leekspeak decode chokidars defrosts glads reprisals smirky
    2b5dj4wasoaww3k6.onion

A python version also exists and is interoperable (if you're on Tails or another system where you don't have access to a compiler)
[`pyleek`](https://github.com/epidemics-scepticism/pyleek)
