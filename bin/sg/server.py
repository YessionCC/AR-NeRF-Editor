import numpy as np
import time
import socket

class Server():
  """TCP IP communication server
  If automatic_port == True, will iterate over port until find a free one
  """
  def __init__(self, ip, port, automatic_port=True):
    max_connections_attempts = 5

    # Start and connect to client
    self.s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    if automatic_port:
      connected = False
      while (not connected) and (max_connections_attempts > 0):
        try:
          self.s.bind((ip, port))
          connected = True
        except:
          print("[Server]: Port", port, "already in use. Binding to port:", port+1)
          port += 1
          max_connections_attempts -= 1
      if not connected:
        print("[Server]: Error binding to adress!")
    else:
      self.s.bind((ip, port))

    self.s.listen(True)
    print("[Server]: Waiting for connection...")
    self.conn, addr = self.s.accept()
    print("[Server]: Connected")

  def __del__(self):
    self.s.close()

  def send(self, message):
    message_size = (len(message))
    self.conn.sendall(message_size.to_bytes(8, 'little'))  # Send length of msg (in known size, 16)
    self.conn.sendall(message)  # Send message

  def receive(self, decode=True):
    len_buf = self.conn.recv(8)
    if len_buf == None:
      return None
    length = int.from_bytes(len_buf, 'little')
    buf = b''
    while length:
      newbuf = self.conn.recv(length)
      if newbuf == None:
        print('Error: incomplete msg')
        break
      buf += newbuf
      length -= len(newbuf)
    return buf

  def clear_buffer(self):
    try:
      while self.conn.recv(1024): pass
    except:
      pass
    return