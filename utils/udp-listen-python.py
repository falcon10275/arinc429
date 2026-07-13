import socket
import struct

UDP_IP = "127.0.0.1"
UDP_PORT = 9999

# Create and bind the UDP socket
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind((UDP_IP, UDP_PORT))

print(f"Listening for ARINC 429 UDP packets on {UDP_IP}:{UDP_PORT}...")

while True:
    data, addr = sock.recvfrom(1024)
    
    # Ensure we received at least 4 bytes (one 32-bit word)
    if len(data) >= 4:
        # Unpack as a 32-bit unsigned integer (! = Network Order / Big Endian, I = unsigned int)
        arinc_word = struct.unpack("!I", data[:4])[0]
        
        # Extract fields based on the 0-indexed bit-masking in the C program
        parity  = (arinc_word >> 31) & 0x01
        ssm     = (arinc_word >> 29) & 0x03
        payload = (arinc_word >> 10) & 0x7FFFF
        sdi     = (arinc_word >> 8)  & 0x03
        label   = arinc_word & 0xFF
        
        print(f"\nReceived from {addr}: 0x{arinc_word:08X}")
        print(f"  Label:  0x{label:02X}")
        print(f"  SDI:    {sdi}")
        print(f"  Data:   0x{payload:05X}")
        print(f"  SSM:    {ssm}")
        print(f"  Parity: {parity}")