import socket

HOST = '127.0.0.1'
PORT = 9001

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect((HOST, PORT))
print(f"Connected to server {HOST}:{PORT}")

while True:
    a = input("Enter the first number (or 'exit' to quit): ")
    if a.lower() == 'exit':
        s.send(b'exit')
        break
    b = input("Enter the second number: ")
    s.send(f"{a} {b}".encode())
    result = s.recv(1024).decode()
    print(f"Server replied: The sum is: {result}")

s.close()
