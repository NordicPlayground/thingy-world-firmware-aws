#!/usr/bin/env python3

import argparse
from intelhex import IntelHex
import sys

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description="Set PDN credentials in HEX file with placeholders")
    parser.add_argument("source_hex")
    parser.add_argument("destination_hex")
    parser.add_argument("username") 
    parser.add_argument("password") 
    parser.add_argument("--username-placeholder", default="5a1d5c1e-2ce9-424b-8b1d-92999c33fa759ff4dfb1-71ff-4016-8684-00d5931e6b1d")
    parser.add_argument("--password-placeholder", default="a9e977c2-9a86-424e-9cc2-a3345f0bff71ce443d28-c88b-4096-93a2-5e24cb31e792")

    args = parser.parse_args()

    if len(args.username) > len(args.username_placeholder):
        print("Error: username is longer than available space in HEX", file=sys.stderr)
        sys.exit(1)
    
    if len(args.password) > len(args.password_placeholder):
        print("Error: password is longer than available space in HEX", file=sys.stderr)
        sys.exit(1)

    hex = IntelHex(args.source_hex)

    username_address = hex.find(args.username_placeholder.encode("ASCII"))

    if username_address == -1:
        print("Error: username placeholder not found", file=sys.stderr)
        sys.exit(1)

    password_address = hex.find(args.password_placeholder.encode("ASCII"))

    if password_address == -1:
        print("Error: password placeholder not found", file=sys.stderr)
        sys.exit(1)

    hex[username_address:username_address+len(args.username_placeholder)].dump()
    hex[username_address:username_address+len(args.username_placeholder)] = \
        list(args.username.encode("ASCII")) + [0]*(len(args.username_placeholder)-len(args.username))
    hex[username_address:username_address+len(args.username_placeholder)].dump()
    
    hex[password_address:password_address+len(args.password_placeholder)] = \
        list(args.password.encode("ASCII")) + [0]*(len(args.password_placeholder)-len(args.password))
    
    hex.write_hex_file(args.destination_hex)
    
