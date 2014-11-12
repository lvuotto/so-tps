#!/usr/bin/env python

import os
import socket
import sys
import time
import random


class TCPFramer:
  def __init__(self, socket):
    self.sock = socket
    self.sock.settimeout(None)
    self.buf = ""

  def send(self, message):
    self.sock.sendall(message + '|\n')

  def receive(self):
    index = self.buf.find('|')
    if index == -1:    # not a complete message, look for more
      self.buf = self.buf + self.sock.recv(1024)

    (res, remaining) = (self.buf.split('|', 1) + [ "" ])[0:2]
    self.buf = remaining

    return res.strip()


class Cliente:
  def __init__(self, host, port, nombre, posicion):
    self.nombre   = nombre
    self.posicion = posicion
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.connect((host, port))

    self.framer = TCPFramer(sock)

    string = "NOMBRE: %s FILA: %d COLUMNA: %d" % (self.nombre, self.posicion[0], self.posicion[1])
    print(string)

    self.framer.send(string)
    #time.sleep(0.5)

  def esperarMascara(self):
    response = self.framer.receive()
    print("Respuesta: <"+ response+ ">")


  def avanzarUnPaso(self):

    if self.posicion[0] == 0:
      direc = "IZQUIERDA"
      next = (0, -1)
    else:
      direc = "ARRIBA"
      next = (-1, 0)

    self.framer.send(direc)
    response = self.framer.receive()
    time.sleep(0.5)
    print("Respuesta: <"+ response+ ">")
    if response == "OK":
      self.posicion = (self.posicion[0] + next[0], self.posicion[1] + next[1])
    else:
      print("rebote")

    return self.posicion == (0,-1)


  def salir(self):

    sali = self.posicion == (0,-1)
    while not sali:
      sali = self.avanzarUnPaso()

    print(self.name, "salio")
    self.sock.close()

#~ clientes = []
#~ for i in range(CLIENTES):
#~    c = Cliente(paises[i], (5,5))
#~    clientes.append(c)
#~ 
#~ #for cliente in clientes:
#~ #  cliente.salir()
#~ 
#~ i = 0
#~ while len(clientes) > 0:
#~   if clientes[i].avanzarUnPaso():
#~     clientes[i].esperarMascara()
#~     clientes.pop(i)
#~   else:
#~     i += 1
#~ 
#~   if i >= len(clientes):
#~     i = 0


def main():
  host, port = sys.argv[1].split(':', 2)
  port = int(port)
  cant_clientes = int(sys.argv[2])
  pids = []
  for i in range(cant_clientes):
    cpid = os.fork()
    if cpid == 0:
      random.seed(i)
      x = random.randint(0, 9)
      y = random.randint(0, 9)
      c = Cliente(host, port, str(i), (x, y))
      moverme(c)
      break
    else:
      pids.append(cpid)
  for pid in pids:
    try:
      os.wait()
    except OSError:
      continue



def moverme(c):
  while True:
    if c.avanzarUnPaso():
      c.esperarMascara()
      break


if __name__ == "__main__":
  main()
