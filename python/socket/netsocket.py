#!/usr/bin/env python3
import socket
import threading
import time

# 全局变量，用于控制服务端和客户端的运行状态
udp_server_running = False
tcp_server_running = False
udp_client_running = False
tcp_client_running = False


def handle_data(data: bytes, addr: tuple) -> bytes:
    try:
        decoded = data.decode('utf-8').strip().lower()
        
        # 根据内容返回不同响应
        if decoded == "ping":
            return b"Pong!"
        elif decoded == "time":
            import time
            return time.ctime().encode()
        else:
            return b"Unknown command"
            
    except Exception as e:
        return b"Error: Invalid data"

# UDP Server
def udp_server(port):
    global udp_server_running
    udp_server_running = True
    server_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    server_socket.bind(("0.0.0.0", port))
    print(f"UDP Server started on port {port}")

    while udp_server_running:
        try:
            data, addr = server_socket.recvfrom(1024)
            print(f"Received from {addr}: {data.decode()}")

            # 使用处理函数生成响应
            response = handle_data(data, addr)

            server_socket.sendto(response, addr)
        except Exception as e:
            print(f"UDP Server error: {e}")
            break

    server_socket.close()
    udp_server_running = False
    print("UDP Server stopped")

# TCP Server
def tcp_server(port):
    global tcp_server_running
    tcp_server_running = True
    server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server_socket.bind(("0.0.0.0", port))
    server_socket.listen(5)
    print(f"TCP Server started on port {port}")
    
    client_socket, addr = server_socket.accept()
    print(f"Connected by {addr}")

    while tcp_server_running:
        try:
            data = client_socket.recv(1024)
            if not data:
                print(f"Client {addr} disconnected gracefully")
                break
            print(f"Received from {addr}: {data.decode()}")

            # 使用处理函数生成响应
            response = handle_data(data, addr)

            client_socket.send(response)
        except ConnectionResetError:
            print(f"Client {addr} forcibly disconnected")
            break
        except Exception as e:
            print(f"TCP Server error: {e}")
            break
        
    client_socket.close()
    server_socket.close()
    tcp_server_running = False
    print("TCP Server stopped")

def udp_client_receive_thread(sock):
    while udp_client_running:
        try:
            data, addr = sock.recvfrom(1024)
            # 添加时间戳避免消息覆盖
            timestamp = time.strftime("%H:%M:%S", time.localtime())
            print(f"\n[{timestamp}] Received: {data.decode()}")
            print("Enter message to send (or 'quit' to stop): ", end='', flush=True)
        except (socket.timeout, BlockingIOError):
            time.sleep(0.01)
        except OSError:
            break

# UDP Client
def udp_client(server_ip, server_port):
    global udp_client_running
    udp_client_running = True
    client_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    client_socket.settimeout(0.1)
    print(f"UDP Client started, sending to {server_ip}:{server_port}")

    # 启动接收线程
    recv_thread = threading.Thread(target=udp_client_receive_thread, args=(client_socket,))
    recv_thread.daemon = True
    recv_thread.start()

    while udp_client_running:
        message = input("Enter message to send (or 'quit' to stop): ")
        if message.lower() == "quit":
            break
        client_socket.sendto(message.encode(), (server_ip, server_port))

    client_socket.close()
    udp_client_running = False
    print("UDP Client stopped")

def tcp_client_receive_thread(sock):
    while tcp_client_running:
        try:
            data = sock.recv(1024)
            # 添加时间戳避免消息覆盖
            timestamp = time.strftime("%H:%M:%S", time.localtime())
            print(f"\n[{timestamp}] Received: {data.decode()}")
            print("Enter message to send (or 'quit' to stop): ", end='', flush=True)
        except (socket.timeout, BlockingIOError):
            time.sleep(0.01)
        except OSError:
            break

# TCP Client
def tcp_client(server_ip, server_port):
    global tcp_client_running
    tcp_client_running = True
    client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    client_socket.connect((server_ip, server_port))
    client_socket.settimeout(0.1)
    print(f"TCP Client started, connected to {server_ip}:{server_port}")

    # 启动接收线程
    recv_thread = threading.Thread(target=tcp_client_receive_thread, args=(client_socket,))
    recv_thread.daemon = True
    recv_thread.start()

    while tcp_client_running:
        message = input("Enter message to send (or 'quit' to stop): ")
        if message.lower() == "quit":
            break
        client_socket.send(message.encode())

    client_socket.close()
    tcp_client_running = False
    print("TCP Client stopped")

# 主菜单
def main_menu():
    print("\n--- Main Menu ---")
    print("1. Start UDP Server")
    print("2. Start TCP Server")
    print("3. Start UDP Client")
    print("4. Start TCP Client")
    print("5. Exit")
    choice = int(input("Enter your choice: "))
    if choice not in [1,2,3,4,5]:
        print("Invalid choice, please try again")
        return 0
    
    return choice

# 主程序
if __name__ == "__main__":
    udp_server_thread = None
    tcp_server_thread = None
    udp_client_thread = None
    tcp_client_thread = None

    choice = main_menu()

    while True:

        if choice == 1:
            if not udp_server_running:
                port = int(input("Enter UDP Server port: "))
                udp_server_thread = threading.Thread(target=udp_server, args=(port,), daemon=True)
                udp_server_thread.start()
            else:
                print("UDP Server is already running")

        elif choice == 2:
            if not tcp_server_running:
                port = int(input("Enter TCP Server port: "))
                tcp_server_thread = threading.Thread(target=tcp_server, args=(port,), daemon=True)
                tcp_server_thread.start()
            else:
                print("TCP Server is already running")

        elif choice == 3:
            if not udp_client_running:
                server_ip = input("Enter server IP: ")
                server_port = int(input("Enter server port: "))
                udp_client_thread = threading.Thread(target=udp_client, args=(server_ip, server_port), daemon=True)
                udp_client_thread.start()
            else:
                print("UDP Client is already running")

        elif choice == 4:
            if not tcp_client_running:
                server_ip = input("Enter server IP: ")
                server_port = int(input("Enter server port: "))
                tcp_client_thread = threading.Thread(target=tcp_client, args=(server_ip, server_port), daemon=True)
                tcp_client_thread.start()
            else:
                print("TCP Client is already running")

        elif choice == 5:
            print("Exiting...")
            break

        if udp_server_running or tcp_server_running or udp_client_running or tcp_client_running:
            time.sleep(1)
            choice = 0
        else:
            choice = main_menu()


