#! /usr/bin/env python
 
import socket
import sys
import time
from paises import *

HOST = 'localhost'
PORT = 5555
CLIENTES = 1
#CLIENTES = 22

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
	def __init__(self, nombre, posicion):
		self.nombre   = nombre
		self.posicion = posicion
		sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
		sock.connect((HOST, PORT))
		
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
			print ("rebote")
				
		return self.posicion == (0,-1)


	def salir(self):
		
		sali = self.posicion == (0,-1)
		while not sali:
			sali = self.avanzarUnPaso()
							
		print(self.name, "salio")
		self.sock.close()
 
clientes = []
for i in range(CLIENTES):
	 c = Cliente(paises[i], (5,5))
	 clientes.append(c)
	 
#for cliente in clientes:
#	cliente.salir()

i = 0
while len(clientes) > 0:
	if clientes[i].avanzarUnPaso():
		clientes[i].esperarMascara()
		clientes.pop(i)
	else:
		i += 1
		
	if i >= len(clientes):
		i = 0
		
		
sys.exit(0)



