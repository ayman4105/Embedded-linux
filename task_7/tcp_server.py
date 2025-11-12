import socket

HOST = '127.0.0.1'
PORT = 9001

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.bind((HOST, PORT))
s.listen(1)
print(f"Server listening on {HOST}:{PORT}...")

conn, addr = s.accept()
print(f"Connected by {addr}")

while True:
    data = conn.recv(1024).decode()
    if not data or data == 'exit':
        break
    a, b = map(float, data.split())
    result = a + b
    print(f"Received numbers: {a} and {b}, sending back sum: {result}")
    conn.send(str(result).encode())

conn.close()
s.close()
